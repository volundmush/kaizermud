#pragma once

#include <optional>
#include "kaizermud/base.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "kaizermud/CallParameters.h"

namespace kaizermud::game {

    class Aspect {
    public:
        std::string objType, slotType, saveKey, name;
        virtual void onRemove(entt::entity ent);
        virtual void onAdd(entt::entity ent);
        virtual void onLoad(entt::entity ent);
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Aspect>>>> aspectRegistry;

    OpResult<> registerAspect(std::shared_ptr<Aspect> entry);

    class AspectSlot : public CallParameters {
    public:
        std::string objType, slotType;
        [[nodiscard]] virtual OpResult<> setAspect(entt::entity ent, const std::string& saveKey, bool isLoading = false);
        [[nodiscard]] virtual OpResult<> atPostLoad(entt::entity ent);
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<AspectSlot>>> aspectSlotRegistry;

    OpResult<> registerAspectSlot(std::shared_ptr<AspectSlot> entry);

    extern std::vector<std::pair<std::pair<std::string, std::string>, std::unordered_map<std::string, std::shared_ptr<AspectSlot>>>> aspectSlotsCache;

    std::unordered_map<std::string, std::shared_ptr<AspectSlot>>& getAspectSlots(const std::pair<std::string, std::string>& objType);


}