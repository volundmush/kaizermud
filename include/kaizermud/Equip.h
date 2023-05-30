#pragma once
#include "kaizermud/base.h"
#include <optional>

#include "SQLiteCpp/SQLiteCpp.h"


namespace kaizer {

    class EquipSlot {
    public:
        virtual ~EquipSlot() = default;
        std::string objType, slot, slotType;
        int sortOrder{0};
        virtual void equip(entt::entity ent);
        [[nodiscard]] virtual bool isAvailable(entt::entity ent);
        [[nodiscard]] virtual OpResult<> canEquip(entt::entity ent);
    };


    extern std::unordered_map<std::string, std::unordered_map<std::string, EquipSlot*>> equipRegistry;
    OpResult<> registerEquip(EquipSlot* entry);



}