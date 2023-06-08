#pragma once
#include "kaizermud/base.h"
#include "kaizermud/Quirks.h"
#include "kaizermud/Equip.h"
#include "kaizermud/Session.h"
#include "kaizermud/Types.h"
#include "kaizermud/Commands.h"
#include "nlohmann/json.hpp"
#include "kaizermud/CallParameters.h"
#include <bitset>

namespace kaizer::components {

    struct ObjectInfo {
        ObjectID id{-1};
        std::bitset<32> typeFlags{};
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

    struct Quirks {
        std::unordered_map<std::string, std::unordered_map<std::string, Quirk*>> data{};
    };

    struct Equipment {
        std::unordered_map<std::string, entt::entity> data{};
    };

    struct Name {
        std::string_view data{};
    };

    struct LookDescription {
        std::string_view data{};
    };

    struct RoomDescription {
        std::string_view data{};
    };

    struct ShortDescription {
        std::string_view data{};
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

    struct Location {
        entt::entity data{entt::null};
    };

    struct Contents {
        std::vector<entt::entity> data{};
    };

    struct Exit {
        entt::entity location{entt::null};
        entt::entity destination{entt::null};
    };

    struct Exits {
        std::unordered_map<std::string, entt::entity> data{};
    };

    struct Entrances {
        std::unordered_map<std::string, entt::entity> data{};
    };

    struct SavedLocations {
        std::unordered_map<std::string, ObjectID> data{};
    };

    struct SessionHolder {
        // 0 is no session, 1 is main puppet, 2 is sub puppet
        uint8_t sessionMode{0};
        std::shared_ptr<Session> data;
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