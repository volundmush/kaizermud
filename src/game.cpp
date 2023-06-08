#include "kaizermud/game.h"
#include <chrono>
#include <boost/asio/steady_timer.hpp>
#include "kaizermud/Systems.h"
#include "kaizermud/Commands.h"
#include "kaizermud/startup.h"
#include "spdlog/spdlog.h"
#include "kaizermud/Database.h"

namespace kaizer {
    using namespace std::chrono_literals;

    lua_CompileOptions compile_options{};

    namespace state {
        ObjectID lastInsertID{0};
        std::set<uint64_t> pending_connections, disconnected_connections;
        std::unordered_map<uint64_t, std::shared_ptr<ClientConnection>> connections;
        std::unordered_map<ObjectID, std::shared_ptr<Session>> sessions;
    }


    ObjectID getNextAvailableID() {
        while(entities.count(state::lastInsertID)) {
            state::lastInsertID++;
        }
        return state::lastInsertID;
    }

    boost::asio::awaitable<void> heartbeat(double deltaTime) {

        for(auto &sys : sortedSystems) {
            if(co_await sys->shouldRun(deltaTime))
                co_await sys->run(deltaTime);
        }
        co_return;
    }

    boost::asio::awaitable<void> load() {
        sortSystems();
        expandCommands();
        loadLatestSave();

        co_return;
    }

    boost::asio::awaitable<void> run() {
        co_await load();
        auto hb = std::chrono::milliseconds(100);
        auto previousTime = boost::asio::steady_timer::clock_type::now();
        boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor, hb);

        while(true) {
            auto timeStart = boost::asio::steady_timer::clock_type::now();
            co_await timer.async_wait(boost::asio::use_awaitable);
            auto timeEnd = boost::asio::steady_timer::clock_type::now();

            auto deltaTime = timeEnd - timeStart;
            double deltaTimeInSeconds = std::chrono::duration<double>(deltaTime).count();

            try {
                co_await heartbeat(deltaTimeInSeconds);
            } catch(std::exception& e) {
                std::cerr << "Exception in heartbeat: " << e.what() << std::endl;
            }

            auto timeAfterHeartbeat = boost::asio::steady_timer::clock_type::now();
            auto elapsed = timeAfterHeartbeat - timeStart;
            auto nextWait = hb - elapsed;

            // If heartbeat takes more than 100ms, default to a very short wait
            if(nextWait.count() < 0) {
                nextWait = std::chrono::milliseconds(1);
            }

            timer.expires_from_now(nextWait);
        }

        // todo: figure out a shutdown / restart routine.
        co_return;
    }

    void broadcast(std::string_view text) {
        boost::json::object j;
        j["kind"] = "broadcast";
        if(text.ends_with("\n")) j["data"] = text;
        else {
            j["data"] = std::string(text) + "\n";
        }
        spdlog::info("Broadcasting: {}", text);
        linkManager->linkChan.try_send(boost::system::error_code{}, j);
    }

    void LuaCode::compile() {
        if(bytecode) free(bytecode);
        state = State::UNCOMPILED;
        bytecode = luau_compile(code.c_str(), code.size(), &compile_options, &length);

        if (bytecode) {
            int line_number;
            if (sscanf(bytecode, ":%d: ", &line_number) == 1) {
                // Error message detected
                state = State::ERROR;
            } else {
                // No error message found, assumed to be valid bytecode
                state = State::COMPILED;
            }

        } else {
            // Error: could not allocate memory for the compiled code
            // TODO: Handle this error... if it is even possible.
        }
    }

    Task::Task() {
        L = luaL_newstate();
        luaL_openlibs(L);
        //luaL_sandbox(L);

    }

    void Task::engageSafety() {
        auto callbacks = lua_callbacks(L);
        callbacks->userdata = this;
        callbacks->interrupt = &Task::interrupt_check;
    }

    void Task::disengageSafety() {
        auto callbacks = lua_callbacks(L);
        callbacks->userdata = nullptr;
        callbacks->interrupt = nullptr;
    }

    void Task::interrupt_check(lua_State* L, int gc) {
        if(gc >= 0) {
            // We don't support GC interrupts since they cannot survive Lua exceptions
            return;
        }

        auto callbacks = lua_callbacks(L);
        auto task = static_cast<Task*>(callbacks->userdata);

        // Check how much time has passed since the task started running
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - task->start_time).count();

        // If the task has exceeded the time limit, interrupt it and force it to yield
        if (elapsed_time >= 5) { // Replace 'task_time_limit' with your desired time limit in milliseconds
            task->state = State::INTERRUPTED;
            lua_yield(L, 0);
        }
    }

    void Task::run() {
        if (state != State::READY && state != State::INTERRUPTED && state != State::WAITING) {
            // Task is not in a state that allows running
            return;
        }

        engageSafety();
        // Resume the coroutine
        state = State::RUNNING;
        start_time = std::chrono::steady_clock::now();
        lua_State *NL = lua_tothread(L, -1); // Get the coroutine from the main Lua state's stack
        int status = lua_resume(NL, nullptr, 0);
        disengageSafety();

        if (status == LUA_YIELD) {
            // Coroutine has yielded
            state = State::WAITING;
        } else if (status == LUA_OK) {
            // Coroutine has finished
            state = State::FINISHED;
        } else {
            // Coroutine has encountered an error
            state = State::ERROR;
            err = lua_tostring(NL, -1);
            lua_pop(NL, 1); // Remove the error message from the stack
            // TODO: Handle the error message (e.g., log it, display it to the user, etc.)
        }
    }

    bool Task::load(const std::string &name, const LuaCode &code) {
        if (code.state != LuaCode::State::COMPILED) {
            state = State::ERROR;
            err = "LuaCode " + name + " is not compiled.";
            // Return false if the provided LuaCode is not compiled or has an error
            return false;
        }

        // Load the bytecode into the task's Lua state
        if (luau_load(L, name.c_str(), code.bytecode, code.length, 0) != LUA_OK) {
            // Handle the error and return false
            err = lua_tostring(L, -1);
            lua_pop(L, 1); // Remove the error message from the stack
            state = State::ERROR;
            return false;
        }

        lua_pcall(L, 0, 0, 0);

        // Check for the 'run' function
        lua_getglobal(L, "run");

        if (!lua_isfunction(L, -1)) {
            // 'run' function not found
            lua_pop(L, 1); // Remove the non-function from the stack
            state = State::ERROR;
            err = "No 'run' function found in the loaded LuaCode.";
            return false;
        }

        // Create the coroutine
        lua_State* NL = lua_newthread(L);
        lua_pushvalue(L, -2); // Duplicate the function on top of the stack
        lua_xmove(L, NL, 1); // Move the function to the new coroutine's stack

        // Coroutine is ready to be executed
        state = State::READY;
        return true;
    }

}

kaizer::Task::~Task() {
    lua_close(L);
}