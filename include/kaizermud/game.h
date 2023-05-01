#pragma once
#include "kaizermud/base.h"
#include <set>
#include <optional>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <boost/asio/experimental/channel.hpp>
#include "kaizermud/thermite.h"
#include "kaizermud/net.h"
#include "lualib.h"
#include "luacode.h"
#include "LuaBridge/LuaBridge.h"

namespace kaizermud::game {

    extern lua_CompileOptions compile_options;

    struct LuaCode {
        explicit LuaCode(const std::string &code);
        ~LuaCode();
        std::string code;
        char *bytecode;
        size_t length;

        void compile();
    };

    class Property {
        LuaCode lua;
    };


    class Function {
        LuaCode lua;
    };

    enum SupervisorLevel : uint8_t {
        SL_NONE = 0,
        SL_PLAYER = 1,
        SL_BUILDER = 2,
        SL_ADMIN = 3,
        SL_OWNER = 4,
        SL_GOD = 5,
    };

    class Command {
    public:
        uint8_t priority;
        bool external;
        uint8_t sLevel;

        LuaCode lua;

    };

    class ObjectReference;

    class Object {
    public:
        Object(int64_t id, uint64_t timestamp)
                : id(id), timestamp(timestamp) {}
        int64_t id{-1};
        uint64_t timestamp{0};
        std::string name;

        uint8_t sLevel{0};

        std::map<std::string, Property> properties;
        std::map<std::string, Function> functions;
        std::map<std::string, Command> commands;

        // The concept of ownership is at a code-level, determining who has authority over
        // this object, and how they relate for matters of quota and permissions control.
        // It is NOT relevant to an in-universe concept of ownership, like a player owning
        // a sword they looted.
        int64_t owner{-1};
        std::set<int64_t> belongings{};

        // Parent is used for determing a thing's code properties and game logic.
        int64_t parent{-1};
        std::set<int64_t> children{};

        // Domain forms a security boundary. Objects belonging to a domain can be
        // modified by those who have power over the domain.
        int64_t domain{-1};
        std::set<int64_t> dominion{};

        ObjectReference makeReference() const;

        std::unordered_map<uint64_t, std::weak_ptr<kaizermud::net::ClientConnection>> connections;

    };



    void fill_free_ids();
    std::optional<int64_t> pop_free_id();
    int64_t get_next_available_id();

    class ObjectReference {
    public:
        ObjectReference() = default;
        ObjectReference(int64_t object_id, uint64_t timestamp)
                : object_id_(object_id), timestamp_(timestamp) {}

        std::optional<std::reference_wrapper<Object>> getObject() const;

    private:
        int64_t object_id_;
        uint64_t timestamp_;
    };

    enum TaskState : uint8_t {
        TS_NEW = 0, // Task is newly created.
        TS_READY = 1, // Not sure if I'll really use this.
        TS_RUNNING = 2, // Task is running, duh.
        TS_WAITING = 3, // Task has voluntarily yielded for some length of time.
        TS_INTERRUPTED = 4, // Task was interrupted by the runtime for taking too long.
        TS_FINISHED = 5, // Task is completed and should be purged.
        TS_ERROR = 6, // Task has errored out and should be purged.
    };

    class Task {
    public:
        Task();
        ~Task();
        uint64_t task_id;
        uint64_t timestamp;

        // Tasks have a state.
        TaskState state{0};

        // tasks should probably have some kind of Type? I'm not
        // sure yet...

        // The enactor is the object which created the task, or for which it was
        // created by other systems. It is used to determine permissions and other
        // variables.
        ObjectReference enactor;

        // If the task has had its permissions changed to a different object,
        // then it's stored here.
        std::optional<ObjectReference> permissions;

        // all tasks contain their own Lua state used to run the task.
        // Luau allows for VM memory restrictions and other shenanigans,
        // so we'll eat the overhead and see what happens.
        lua_State *L;

        static void interrupt_check(lua_State* L, int gc);

    };

    boost::asio::awaitable<void> process_connections();
    boost::asio::awaitable<void> process_tasks();
    boost::asio::awaitable<void> load();
    boost::asio::awaitable<void> heartbeat();
    boost::asio::awaitable<void> run();


    namespace state {
        extern std::vector<std::optional<Object>> objects;
        extern std::set<int64_t> free_ids; // this will store like 100 vector slots that are currently unused to reduce scans.
        extern std::set<uint64_t> pending_connections, disconnected_connections;
        extern std::unordered_map<uint64_t, std::shared_ptr<kaizermud::net::ClientConnection>> connections;
        extern lua_State* L;
    }

}