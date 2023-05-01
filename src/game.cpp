#include "kaizermud/game.h"
#include <chrono>
#include <boost/asio/steady_timer.hpp>

namespace kaizermud::game {
    using namespace std::chrono_literals;

    lua_CompileOptions compile_options;

    namespace state {
        std::vector<std::optional<Object>> objects;
        std::set<int64_t> free_ids;
        std::set<uint64_t> pending_connections, disconnected_connections;
        std::unordered_map<uint64_t, std::shared_ptr<kaizermud::net::ClientConnection>> connections;
        lua_State* L = luaL_newstate();
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

    LuaCode::LuaCode(const std::string &code) {
        this->code = code;
        if(code.empty()) return;
        compile();
    }

    void LuaCode::compile() {
        if(bytecode) return;
        bytecode = luau_compile(code.c_str(), code.size(), &compile_options, &length);
    }

    Task::Task() {
        L = luaL_newstate();
        auto callbacks = lua_callbacks(L);
        callbacks->userdata = this;
        callbacks->interrupt = &Task::interrupt_check;
    }

    void Task::interrupt_check(lua_State* L, int gc) {
        if(gc >= 0) {
            // We don't support GC interrupts since they cannot survive Lua exceptions
            return;
        }

        auto callbacks = lua_callbacks(L);
        auto task = static_cast<Task*>(callbacks->userdata);

        // Do some kind of check here. How much time has passed, maybe?
        // if we've exceeded the time limit...
        if(false) {
            task->state = TS_INTERRUPTED;
            lua_yield(L, 0);
        }
    }

}

kaizermud::game::LuaCode::~LuaCode() {
    if (bytecode) free(bytecode);
}

kaizermud::game::Task::~Task() {
    lua_close(L);
}