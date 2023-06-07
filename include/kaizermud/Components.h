#pragma once
#include "kaizermud/base.h"
#include "kaizermud/Aspects.h"
#include "kaizermud/Quirks.h"
#include "kaizermud/Equip.h"
#include "kaizermud/Session.h"
#include "kaizermud/Types.h"
#include "kaizermud/Commands.h"
#include "nlohmann/json.hpp"
#include "kaizermud/CallParameters.h"

namespace kaizer::components {

    struct ObjectInfo {
        ObjectID id{-1};
        std::unordered_map<std::string, Type*> types{};
        std::vector<Type*> sortedTypes{};
        void doSort();
    };

    struct Account {
        Account() = default;
        explicit Account(const nlohmann::json &j);
        std::string username{}, password{}, email{};
        std::chrono::system_clock::time_point created{};
        std::chrono::system_clock::time_point lastLogin{};
        std::chrono::system_clock::time_point lastLogout{};
        std::chrono::system_clock::time_point lastPasswordChanged{};
        std::string disabledReason{};
        double totalPlayTime{0}, totalLoginTime{0};
        std::set<ObjectID> characters{};
        std::unordered_map<uint64_t, std::shared_ptr<ClientConnection>> connections{};
        std::unordered_map<ObjectID, std::shared_ptr<Session>> sessions{};
        // This is account permission level, not character level.
        int16_t level{0};
        OpResult<> setPassword(std::string_view newPassword);
        OpResult<> checkPassword(std::string_view check);
        nlohmann::json serialize();
        void deserialize(const nlohmann::json& j);
    };

    struct Aspects {
        std::unordered_map<std::string, Aspect*> data{};
    };

    struct Quirks {
        std::unordered_map<std::string, std::unordered_map<std::string, Quirk*>> data{};
    };

    struct Equipment {
        std::unordered_map<std::string, entt::entity> data{};
    };

    struct Strings {
        std::unordered_map<std::string, std::string_view> data{};
    };

    struct Integers {
        std::unordered_map<std::string, int64_t> data{};
    };

    struct Doubles {
        std::unordered_map<std::string, double> data{};
    };

    struct Stats {
        std::unordered_map<std::string, double> data{};
    };

    struct Relations {
        std::unordered_map<std::string, entt::entity> data{};
    };

    struct ReverseRelations {
        std::unordered_map<std::string, std::vector<entt::entity>> data{};
    };

    struct SavedLocations {
        std::unordered_map<std::string, ObjectID> data{};
    };

    struct SessionHolder {
        // 0 is no session, 1 is main puppet, 2 is sub puppet
        uint8_t sessionMode{0};
        std::shared_ptr<Session> data;
    };

    struct CommandCache {
        std::unordered_map<std::string, Command*> commands{};
        std::vector<std::pair<std::string, Command*>> sortedCommands{};
        void sortCommands();
    };

    // Systems and game state components below...
    struct PendingCommand {
        std::string input;
    };

    struct PendingMove {
        CallParameters params;
        entt::entity reportTo{entt::null};
    };

    struct PendingLook {
        entt::entity target{entt::null};
    };

}