#include "kaizermud/Stats.h"

namespace kaizermud::game {

    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Stat>>> statRegistry{};

    double Stat::get(entt::entity ent) {
        return getBase(ent);
    }

    OpResult<> registerStat(std::shared_ptr<Stat> entry) {
        if(entry->objType.empty()) {
            return {false, "StatEntry object_type cannot be empty"};
        }
        if(entry->name.empty()) {
            return {false, "StatEntry name cannot be empty"};
        }

        auto &reg = statRegistry[entry->objType];

        if(reg.find(entry->saveKey) != reg.end()) {
            return {false, "StatEntry already registered"};
        }

        reg[entry->saveKey] = entry;
        return {true, std::nullopt};
    }

}