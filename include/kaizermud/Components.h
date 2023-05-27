#pragma once
#include "kaizermud/base.h"
#include "kaizermud/Aspects.h"
#include "kaizermud/Equip.h"
#include "kaizermud/Session.h"

namespace kaizermud::components {

    struct ObjectInfo {
        ObjectID id;
        std::pair<std::string, std::string> types;
        [[nodiscard]] std::string getMainType() const { return types.first; }
        [[nodiscard]] std::string getSubType() const { return types.second; }
    };

    struct Aspects {
        std::unordered_map<std::string, std::shared_ptr<game::Aspect>> data;
    };

    struct Equipment {
        std::unordered_map<std::string, std::shared_ptr<game::EquipSlot>> data;
    };

    struct Strings {
        std::unordered_map<std::string, std::string> data;
    };

    struct Integers {
        std::unordered_map<std::string, int64_t> data;
    };

    struct Doubles {
        std::unordered_map<std::string, double> data;
    };

    struct Stats {
        std::unordered_map<std::string, double> data;
    };

    struct Relations {
        std::unordered_map<std::string, entt::entity> data;
    };

    struct ReverseRelations {
        std::unordered_map<std::string, std::vector<entt::entity>> data;
    };

    struct Session {
        // 0 is no session, 1 is main puppet, 2 is sub puppet
        uint8_t sessionMode{0};
        std::shared_ptr<game::Session> data;
    };

}