#pragma once
#include "kaizermud/base.h"
#include <set>
#include <optional>
#include <string>
#include <map>

namespace kaizermud::db {

    class Property {

    };


    class Function {

    };

    enum SupervisorLevel {
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

    class Object {
    public:
        Object() = default;
        int64_t id{-1};
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

    };

}