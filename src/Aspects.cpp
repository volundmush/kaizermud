#include "kaizermud/Aspects.h"
#include "kaizermud/Object.h"
#include "kaizermud/utils.h"

namespace kaizermud::game {

    // Aspect

    void Aspect::onRemove(AspectHandler *handler) {
    }

    void Aspect::onAdd(AspectHandler *handler) {
    }

    void Aspect::onLoad(AspectHandler *handler) {
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Aspect>>>> aspectRegistry;

    OpResult registerAspect(const std::shared_ptr<Aspect> entry) {
        if(entry->objType.empty()) {
            return {false, "Object type cannot be empty"};
        }
        if(entry->slotType.empty()) {
            return {false, "Slot type cannot be empty"};
        }
        if(entry->saveKey.empty()) {
            return {false, "Save key cannot be empty"};
        }

        auto &objreg = aspectRegistry[entry->objType];
        auto &slotreg = objreg[entry->slotType];
        slotreg[entry->saveKey] = entry;
        return {true, std::nullopt};
    }

    // AspectSlot

    OpResult AspectSlot::setAspect(AspectHandler *handler, const std::string& saveKey, bool isLoading) {
        auto objreg = aspectRegistry.find(std::string(handler->obj->getMainType()));
        if(objreg == aspectRegistry.end()) {
            return {false, "No such object type"};
        }

        auto slotreg = objreg->second.find(std::string(slotType));
        if(slotreg == objreg->second.end()) {
            return {false, "No such slot type"};
        }

        auto keyreg = slotreg->second.find(saveKey);

        if(keyreg == slotreg->second.end()) {
            return {false, "No such aspect"};
        }

        auto &aspect = handler->aspects[saveKey];

        if (aspect != nullptr) {
            aspect->onRemove(handler);
        }

        aspect = keyreg->second;
        if(isLoading) {
            aspect->onLoad(handler);
        } else {
            aspect->onAdd(handler);
        }

        return {true, std::nullopt};
    }

    OpResult AspectSlot::atPostLoad(AspectHandler *handler) {
        return {true, std::nullopt};
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<AspectSlot>>> aspectSlotRegistry;

    OpResult registerAspectSlot(std::shared_ptr<AspectSlot> entry) {
        if(entry->objType.empty()) {
            return {false, "Object type cannot be empty"};
        }
        if(entry->slotType.empty()) {
            return {false, "Slot type cannot be empty"};
        }
        auto &objreg = aspectSlotRegistry[entry->objType];
        objreg[entry->slotType] = entry;
        return {true, std::nullopt};
    }

    // AspectHandler
    AspectHandler::AspectHandler(const std::shared_ptr<Object>& obj) : obj(obj) {
    }

    void AspectHandler::load() {
        if(loaded) {
            return;
        }
        loaded = true;

        std::unordered_map<std::string, std::shared_ptr<AspectSlot>> aspectSlotsToLoad;

        for(const auto& type : obj->getTypes()) {
            auto objreg = aspectSlotRegistry.find(std::string(type));
            if(objreg == aspectSlotRegistry.end()) {
                continue;
            }
            auto slotreg = objreg->second;
            for(const auto& [name, entry] : slotreg) {
                aspectSlotsToLoad[name] = entry;
            }
        }

        for(const auto &[name, entry] : aspectSlotsToLoad) {

            slots[name] = entry;
        }

    }

    void AspectHandler::saveToDB(const std::shared_ptr<SQLite::Database> &db) {
        SQLite::Statement q1(*db, "INSERT INTO objectAspect (objectId, aspectType, aspectKey) VALUES (?, ?, ?);");

        auto objid = obj->getId();
        for(const auto& [name, aspect] : aspects) {
            if(aspect == nullptr) {
                continue;
            }
            q1.bind(1, objid);
            q1.bind(2, aspect->slotType);
            q1.bind(3, aspect->saveKey);
            q1.exec();
            q1.reset();
        }

    }

    void AspectHandler::loadFromDB(const std::shared_ptr<SQLite::Database> &db) {
        SQLite::Statement q1(*db, "SELECT aspectType, aspectKey FROM objectAspect WHERE objectId = ?;");
        q1.bind(1, obj->getId());
        while(q1.executeStep()) {
            auto slotType = q1.getColumn(0).getString();
            auto aspectKey = q1.getColumn(1).getString();
            auto slot = slots.find(slotType);
            if(slot == slots.end()) {
                continue;
            }
            slot->second->setAspect(this, aspectKey, true);
        }
    }
}