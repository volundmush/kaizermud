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

    void EquipSlot::equip(const std::shared_ptr<Object>& object) {
        item = object;
        object->atEquipped(handler->obj, slot);
    }

    void EquipSlot::setProperty(const std::string &name, std::string_view value) {
        properties[name] = utils::intern(std::string(value));
    }

    std::string_view EquipSlot::getProperty(std::string_view name) const {
        auto it = properties.find(std::string(name));
        if(it == properties.end()) {
            return "";
        }
        return it->second;
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

    OpResult EquipSlot::canEquip(const std::shared_ptr<Object>& object) const {
        if(item) {
            return {false, "You are already wearing something in that slot."};
        }
        return {true, std::nullopt};
    }

    // EquipEntry
    std::unordered_map<std::string, std::unordered_map<std::string, EquipEntry>> equipRegistry;

    OpResult registerEquip(EquipEntry entry) {
        if(entry.objType.empty()) {
            return {false, "EquipEntry objType cannot be empty"};
        }
        if(entry.slot.empty()) {
            return {false, "EquipEntry slot cannot be empty"};
        }
        if(entry.slotType.empty()) {
            return {false, "EquipEntry slotType cannot be empty"};
        }
        if(entry.ctor == nullptr) {
            return {false, "EquipEntry ctor cannot be null"};
        }
        auto &reg = equipRegistry[entry.objType];
        if(reg.find(entry.slot) != reg.end()) {
            return {false, "EquipEntry already registered"};
        }
        reg[entry.slot] = entry;
        return {true, std::nullopt};
    }

    // EquipHandler
    EquipHandler::EquipHandler(const std::shared_ptr<Object>& obj) : obj(obj) {

    }

    void EquipHandler::load() {
        if(loaded) return;
        loaded = true;

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
            for(const auto &[name, val]: entry.properties) {
                slotObj->setProperty(name, val);
            }
        }
    }

    void EquipHandler::saveToDB(const std::shared_ptr<SQLite::Database> &db) {
        SQLite::Statement q1(*db, "INSERT INTO objectEquip (objectId, slot, itemId) VALUES (?, ?, ?);");
        auto refID = obj->getId();

        for(const auto &[slot, entry] : slots) {
            if(entry->item) {
                q1.bind(1, refID);
                q1.bind(2, std::string(entry->getSlot()));
                q1.bind(3, entry->item->getId());
                q1.exec();
            }
        }

    }

    void EquipHandler::loadFromDB(const std::shared_ptr<SQLite::Database> &db) {
        SQLite::Statement q1(*db, "SELECT slot, itemId FROM objectEquip WHERE objectId = ?;");
        q1.bind(1, obj->getId());
        while(q1.executeStep()) {
            auto slot = q1.getColumn(0).getString();
            auto itemId = q1.getColumn(1).getInt64();
            auto it = slots.find(slot);
            if(it != slots.end()) {
                auto found = objects.find(itemId);
                if(found != objects.end()) {
                    it->second->equip(found->second);
                }
            }
        }
    }

}