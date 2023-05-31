#include "kaizermud/Equip.h"
#include "kaizermud/utils.h"

namespace kaizer {

    // EquipSlot
    void EquipSlot::equip(entt::entity ent) {

    }

    bool EquipSlot::isAvailable(entt::entity ent) {
        return true;
    }

    OpResult<> EquipSlot::canEquip(entt::entity ent) {
        if(true) {
            return {false, "You are already wearing something in that slot."};
        }
        return {true, std::nullopt};
    }

    // EquipEntry
    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<EquipSlot>>> equipRegistry;

    OpResult<> registerEquip(std::shared_ptr<EquipSlot> entry) {
        if(entry->objType.empty()) {
            return {false, "EquipEntry objType cannot be empty"};
        }
        if(entry->slot.empty()) {
            return {false, "EquipEntry slot cannot be empty"};
        }
        if(entry->slotType.empty()) {
            return {false, "EquipEntry slotType cannot be empty"};
        }

        auto &reg = equipRegistry[std::string(entry->objType)];
        if(reg.find(std::string(entry->slot)) != reg.end()) {
            return {false, "EquipEntry already registered"};
        }
        reg[std::string(entry->slot)] = entry;
        return {true, std::nullopt};
    }

    // EquipHandler

}