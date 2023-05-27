#include "kaizermud/Aspects.h"
#include "kaizermud/utils.h"
#include "kaizermud/Components.h"

namespace kaizermud::game {

    // Aspect

    void Aspect::onRemove(entt::entity ent) {
    }

    void Aspect::onAdd(entt::entity ent) {
    }

    void Aspect::onLoad(entt::entity ent) {
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Aspect>>>> aspectRegistry;

    OpResult<> registerAspect(const std::shared_ptr<Aspect> entry) {
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

    OpResult<> AspectSlot::setAspect(entt::entity ent, const std::string& saveKey, bool isLoading) {
        auto &objinfo = registry.get<components::ObjectInfo>(ent);
        auto objreg = aspectRegistry.find(std::string(objinfo.getMainType()));
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

        auto &aspects = registry.get_or_emplace<components::Aspects>(ent);

        auto &aspect = aspects.data[saveKey];

        if (aspect != nullptr) {
            aspect->onRemove(ent);
        }

        aspect = keyreg->second;
        if(isLoading) {
            aspect->onLoad(ent);
        } else {
            aspect->onAdd(ent);
        }

        return {true, std::nullopt};
    }

    OpResult<> AspectSlot::atPostLoad(entt::entity ent) {
        return {true, std::nullopt};
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<AspectSlot>>> aspectSlotRegistry;

    std::vector<std::pair<std::pair<std::string_view, std::string_view>, std::unordered_map<std::string, std::shared_ptr<AspectSlot>>>> aspectSlotsCache;

    OpResult<> registerAspectSlot(std::shared_ptr<AspectSlot> entry) {
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

    std::unordered_map<std::string, std::shared_ptr<AspectSlot>>& getAspectSlots(const std::pair<std::string_view, std::string_view>& objType) {
        auto found = std::find_if(aspectSlotsCache.begin(), aspectSlotsCache.end(), [objType](auto &x) {return x.first == objType;});
        if(found != aspectSlotsCache.end()) {
            return found->second;
        }
        // We didn't find it, so we'll have to make one...
        auto &newreg = aspectSlotsCache.emplace_back(objType, std::unordered_map<std::string, std::shared_ptr<AspectSlot>>()).second;

        for(const auto& type : {objType.first, objType.second}) {
            auto objreg = aspectSlotRegistry.find(std::string(type));
            if(objreg == aspectSlotRegistry.end()) {
                continue;
            }
            auto slotreg = objreg->second;
            for(const auto& [name, entry] : slotreg) {
                newreg[name] = entry;
            }
        }
        return newreg;

    }
}