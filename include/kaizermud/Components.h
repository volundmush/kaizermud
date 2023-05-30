#pragma once
#include "kaizermud/base.h"
#include "kaizermud/Aspects.h"
#include "kaizermud/Quirks.h"
#include "kaizermud/Equip.h"
#include "kaizermud/Session.h"
#include "kaizermud/Types.h"

namespace kaizer::components {

    struct ObjectInfo {
        ObjectID id{-1};
        std::unordered_map<std::string, Type*> types{};
        std::vector<Type*> sortedTypes{};
        void doSort();
    };

    struct Account {
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
        OpResult<> setPassword(std::string_view newPassword);
        OpResult<> checkPassword(std::string_view check);
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

    struct SessionHolder {
        // 0 is no session, 1 is main puppet, 2 is sub puppet
        uint8_t sessionMode{0};
        std::shared_ptr<Session> data;
    };

}