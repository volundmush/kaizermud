#pragma once

#include "kaizermud/base.h"

namespace kaizer {

    class Quirk {
    public:
        std::string objType, slotType, key;
        virtual void onRemove(entt::entity ent);
        virtual void onAdd(entt::entity ent);
        virtual void onLoad(entt::entity ent);
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Quirk>>>> quirkRegistry;

    extern std::unordered_map<std::string, std::vector<std::pair<std::pair<std::string_view, std::string_view>, std::unordered_map<std::string, std::shared_ptr<Quirk>>>>> quirksCache;

    const std::unordered_map<std::string, std::shared_ptr<Quirk>>& getQuirks(std::string_view slot, const std::pair<std::string_view, std::string_view>& objType);

    OpResult<> registerQuirk(std::shared_ptr<Quirk> entry);


}