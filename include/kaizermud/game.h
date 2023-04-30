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

namespace kaizermud::game {

    class Property {

    };


    class Function {

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

    };

    namespace state {
        extern std::vector<std::optional<Object>> objects;
        extern std::set<int64_t> free_ids; // this will store like 100 vector slots that are currently unused to reduce scans.
        extern std::set<uint64_t> pending_connections, disconnected_connections;
        extern std::unordered_map<uint64_t, std::shared_ptr<kaizermud::net::ClientConnection>> connections;
    }

    void fill_free_ids();
    std::optional<int64_t> pop_free_id();
    int64_t get_next_available_id();

    class ObjectReference {
    public:
        ObjectReference(int64_t object_id, uint64_t timestamp)
                : object_id_(object_id), timestamp_(timestamp) {}

        std::optional<std::reference_wrapper<Object>> getObject() const;

    private:
        int64_t object_id_;
        uint64_t timestamp_;
    };


    boost::asio::awaitable<void> process_connections();
    boost::asio::awaitable<void> process_tasks();
    boost::asio::awaitable<void> load();
    boost::asio::awaitable<void> heartbeat();
    boost::asio::awaitable<void> run();

}