#include "kaizermud/Aspects.h"
#include "kaizermud/Object.h"
#include "kaizermud/utils.h"

namespace kaizermud::game {

    // Aspect
    Aspect::Aspect(AspectSlot *slot) : slot(slot) {
    }

    Object* Aspect::obj() {
        return slot->obj();
    }

    std::string_view Aspect::getSlotType() {
        return slot->slotType;
    }

    void Aspect::onRemove() {
    }

    void Aspect::onAdd() {
    }

    void Aspect::onLoad() {
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, AspectEntry>>> aspectRegistry;

    OpResult registerAspect(AspectEntry entry) {
        if(entry.objType.empty()) {
            return {false, "Object type cannot be empty"};
        }
        if(entry.slotType.empty()) {
            return {false, "Slot type cannot be empty"};
        }
        if(entry.saveKey.empty()) {
            return {false, "Save key cannot be empty"};
        }
        if(entry.ctor == nullptr) {
            return {false, "Constructor cannot be null"};
        }
        auto &objreg = aspectRegistry[entry.objType];
        auto &slotreg = objreg[entry.slotType];
        slotreg[entry.saveKey] = entry;
        return {true, std::nullopt};
    }

    // AspectSlot
    AspectSlot::AspectSlot(AspectHandler *handler, const std::string& slotType) : handler(handler) {
        this->slotType = utils::intern(slotType);
    }

    Object* AspectSlot::obj() {
        return handler->obj;
    }

    OpResult AspectSlot::setAspect(const std::string& saveKey, bool isLoading) {
        auto objreg = aspectRegistry.find(std::string(obj()->getType()));
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

        if (aspect != nullptr) {
            aspect->onRemove();
        }

        aspect = keyreg->second.ctor(this);
        if(isLoading) {
            aspect->onLoad();
        } else {
            aspect->onAdd();
        }

        return {true, std::nullopt};
    }

    std::unordered_map<std::string, std::unordered_map<std::string, AspectSlotEntry>> aspectSlotRegistry;

    OpResult registerAspectSlot(AspectSlotEntry entry) {
        if(entry.objType.empty()) {
            return {false, "Object type cannot be empty"};
        }
        if(entry.slotType.empty()) {
            return {false, "Slot type cannot be empty"};
        }
        if(entry.ctor == nullptr) {
            return {false, "Constructor cannot be null"};
        }
        auto &objreg = aspectSlotRegistry[entry.objType];
        objreg[entry.slotType] = entry;
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

        std::unordered_map<std::string, AspectSlotEntry> aspectSlotsToLoad;

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

            slots[name] = entry.ctor(this);
        }

    }
}