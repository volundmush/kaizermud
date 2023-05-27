#pragma once
#include "kaizermud/base.h"
#include <set>
#include <optional>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <boost/asio/experimental/channel.hpp>
#include "kaizermud/thermite.h"
#include "kaizermud/ClientConnection.h"
#include "lualib.h"
#include "luacode.h"
#include "LuaBridge/LuaBridge.h"
#include "kaizermud/Lua.h"

namespace kaizermud::game {

    class Task {
    public:
        Task();
        ~Task();
        uint64_t task_id;
        uint64_t timestamp;

        enum class State : uint8_t {
            NEW = 0, // Task is newly created.
            READY = 1, // Not sure if I'll really use this.
            RUNNING = 2, // Task is running, duh.
            WAITING = 3, // Task has voluntarily yielded for some length of time.
            INTERRUPTED = 4, // Task was interrupted by the runtime for taking too long.
            FINISHED = 5, // Task is completed and should be purged.
            ERROR = 6, // Task has errored out and should be purged.
        };

        bool load(const std::string &name, const LuaCode &code);

        // Tasks have a state.
        State state{State::NEW};

        // time tracking stuff here...
        std::chrono::steady_clock::time_point start_time;

        // all tasks contain their own Lua state used to run the task.
        // Luau allows for VM memory restrictions and other shenanigans,
        // so we'll eat the overhead and see what happens.
        lua_State *L;

        std::string err{};

        static void interrupt_check(lua_State* L, int gc);
        void engageSafety();
        void disengageSafety();
        void run();

    };

    ObjectID getNextAvailableID();
    boost::asio::awaitable<void> process_connections(const boost::asio::steady_timer::duration& deltaTime);
    boost::asio::awaitable<void> process_tasks(const boost::asio::steady_timer::duration& deltaTime);
    boost::asio::awaitable<void> load();
    boost::asio::awaitable<void> heartbeat(const boost::asio::steady_timer::duration& deltaTime);
    boost::asio::awaitable<void> run();


    namespace state {
        extern std::set<int64_t> free_ids; // this will store like 100 vector slots that are currently unused to reduce scans.
        extern std::set<uint64_t> pending_connections, disconnected_connections;
        extern std::unordered_map<uint64_t, std::shared_ptr<kaizermud::net::ClientConnection>> connections;
    }

}