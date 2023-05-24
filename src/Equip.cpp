#include "kaizermud/Equip.h"
#include "kaizermud/Object.h"
#include "kaizermud/utils.h"

namespace kaizermud::game {

    // EquipSlot
    EquipSlot::EquipSlot(kaizermud::game::EquipHandler *handler, const std::string &slot, const std::string &slotType) {
        this->handler = handler;
        this->slot = utils::intern(slot);
        this->slotType = utils::intern(slotType);
    }

    void EquipSlot::setWearVerb(const std::string &verb) {
        wearVerb = utils::intern(verb);
    }

    std::string_view EquipSlot::getWearVerb() const {
        return wearVerb.value_or("wear");
    }

    void EquipSlot::setWearDisplay(const std::string &display) {
        wearDisplay = utils::intern(display);
    }

    std::string_view EquipSlot::getWearDisplay() const {
        return wearDisplay.value_or("You wear $item.");
    }

    void EquipSlot::setRemoveVerb(const std::string &verb) {
        removeVerb = utils::intern(verb);
    }

    std::string_view EquipSlot::getRemoveVerb() const {
        return removeVerb.value_or("remove");
    }

    void EquipSlot::setRemoveDisplay(const std::string &display) {
        removeDisplay = utils::intern(display);
    }

    std::string_view EquipSlot::getRemoveDisplay() const {
        return removeDisplay.value_or("You remove $item.");
    }

    void EquipSlot::setListDisplay(const std::string &display) {
        listDisplay = utils::intern(display);
    }

    std::string_view EquipSlot::getListDisplay() const {
        return listDisplay.value_or("You are wearing $item.");
    }

    std::string_view EquipSlot::getSlot() const {
        return slot;
    }

    std::string_view EquipSlot::getSlotType() const {
        return slotType;
    }

    void EquipSlot::setSortOrder(int order) {
        sortOrder = order;
    }

    int EquipSlot::getSortOrder() const {
        return sortOrder;
    }

    bool EquipSlot::isAvailable() const {
        return true;
    }

    OpResult EquipSlot::canEquip(Object *object) const {
        if(item.has_value()) {
            return {false, "You are already wearing something in that slot."};
        }
        return {true, std::nullopt};
    }

    // EquipEntry
    std::unordered_map<std::string, std::unordered_map<std::string, EquipEntry>> equipRegistry;

    void registerEquip(EquipEntry entry) {
        if(entry.objType.empty()) {
            throw std::runtime_error("EquipEntry objType cannot be empty");
        }
        if(entry.slot.empty()) {
            throw std::runtime_error("EquipEntry slot cannot be empty");
        }
        if(entry.slotType.empty()) {
            throw std::runtime_error("EquipEntry slotType cannot be empty");
        }
        if(entry.ctor == nullptr) {
            throw std::runtime_error("EquipEntry ctor cannot be null");
        }
        auto &reg = equipRegistry[entry.objType];
        if(reg.find(entry.slot) != reg.end()) {
            throw std::runtime_error("EquipEntry already registered");
        }
        reg[entry.slot] = entry;
    }

    // EquipHandler
    EquipHandler::EquipHandler(Object *object) {
        this->obj = object;
    }

    void EquipHandler::load() {
        std::unordered_map<std::string, EquipEntry> toLoad;

        for(const auto &objType : obj->getTypes()) {
            auto it = equipRegistry.find(std::string(objType));
            if(it != equipRegistry.end()) {
                for(const auto &entry : it->second) {
                    toLoad[entry.first] = entry.second;
                }
            }
        }

        for(const auto &[slot, entry] : toLoad) {
            slots[slot] = entry.ctor(this, slot, entry.slotType);
            auto &slotObj = slots[slot];
            slotObj->setSortOrder(entry.sortOrder);
            if(entry.wearVerb.has_value()) {
                slotObj->setWearVerb(*entry.wearVerb);
            }
            if(entry.wearDisplay.has_value()) {
                slotObj->setWearDisplay(*entry.wearDisplay);
            }
            if(entry.removeVerb.has_value()) {
                slotObj->setRemoveVerb(*entry.removeVerb);
            }
            if(entry.removeDisplay.has_value()) {
                slotObj->setRemoveDisplay(*entry.removeDisplay);
            }
            if(entry.listDisplay.has_value()) {
                slotObj->setListDisplay(*entry.listDisplay);
            }
        }
    }

}