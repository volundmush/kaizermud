#include "kaizermud/game.h"
#include <chrono>
#include <boost/asio/steady_timer.hpp>

namespace kaizermud::game {
    using namespace std::chrono_literals;

    lua_CompileOptions compile_options{};

    namespace state {
        std::vector<std::optional<Object>> objects;
        std::set<int64_t> free_ids;
        std::set<uint64_t> pending_connections, disconnected_connections;
        std::unordered_map<uint64_t, std::shared_ptr<kaizermud::net::ClientConnection>> connections;
    }

    void fill_free_ids() {
        // Scan the objects vector to find free IDs
        size_t count = 0;
        size_t index = 0;
        for (const auto& obj : state::objects) {
            if (count >= 50) {
                break;
            }
            if (!obj.has_value()) {
                state::free_ids.insert(static_cast<int64_t>(index));
                count++;
            }
            index++;
        }
    }

    std::optional<int64_t> pop_free_id() {
        if (!state::free_ids.empty()) {
            // Get the iterator pointing to the first free ID in the set
            auto it = state::free_ids.begin();
            int64_t id = *it;
            state::free_ids.erase(it);
            return id;
        }
        return {};
    }

    int64_t get_next_available_id() {
        auto maybe_id = pop_free_id();
        if(maybe_id.has_value()) {
            return maybe_id.value();
        }
        fill_free_ids();

        maybe_id = pop_free_id();
        if(maybe_id.has_value()) {
            return maybe_id.value();
        }

        // No free IDs found, use the next ID in the sequence
        auto id = static_cast<int64_t>(state::objects.size());
        state::objects.emplace_back(); // Add an empty optional to the vector
        return id;
    }


    ObjectReference Object::makeReference() const {
        return {id, timestamp};
    }

    std::optional<std::reference_wrapper<Object>> ObjectReference::getObject() const {
        // Check if the object_id_ is within the bounds of the vector
        if (object_id_ < 0 || object_id_ >= state::objects.size()) {
            return std::nullopt;
        }

        // Check if the object at the given index exists and the timestamp matches
        auto& obj_opt = state::objects[object_id_];
        if (obj_opt.has_value() && obj_opt->timestamp == timestamp_) {
            return std::ref(obj_opt.value());
        }

        // Object not found or timestamp mismatch
        return std::nullopt;
    }

    boost::asio::awaitable<void> process_connections() {
        // First, handle any disconnected connections.
        for (const auto &id : state::disconnected_connections) {
            auto it = state::connections.find(id);
            if (it != state::connections.end()) {
                auto& conn = it->second;
                conn->onNetworkDisconnected();
                state::connections.erase(it);
            }
        }
        state::disconnected_connections.clear();

        // Second, welcome any new connections!
        for(const auto& id : state::pending_connections) {
            auto it = state::connections.find(id);
            if (it != state::connections.end()) {
                auto& conn = it->second;
                // Need a proper welcoming later....
                conn->onWelcome();
            }
        }
        state::pending_connections.clear();

        // Next, we must handle the heartbeat routine for each connection.
        for(auto& [id, conn] : state::connections) {
            conn->onHeartbeat();
        }

        co_return;
    }

    boost::asio::awaitable<void> process_tasks() {
        co_return;
    }

    boost::asio::awaitable<void> heartbeat() {
        co_await process_connections();
        co_await process_tasks();

        co_return;
    }

    boost::asio::awaitable<void> load() {
        co_return;
    }

    boost::asio::awaitable<void> run() {
        co_await load();

        boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor, 100ms);

        while(true) {
            co_await timer.async_wait(boost::asio::use_awaitable);
            co_await heartbeat();
        }

        co_return;

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

kaizermud::game::Task::~Task() {
    lua_close(L);
}