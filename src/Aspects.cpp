#include "kaizermud/Aspects.h"
#include "kaizermud/utils.h"
#include "kaizermud/Components.h"

namespace kaizer {

    // Aspect

    void Aspect::onRemove(entt::entity ent) {

    }

    void Aspect::onAdd(entt::entity ent) {

    }

    void Aspect::onLoad(entt::entity ent) {
    }

    OpResult<> Aspect::canSet(entt::entity ent) {
        return {true, std::nullopt};
    }

    std::unordered_map<std::string, std::unordered_map<std::string, Aspect*>> aspectRegistry;

    OpResult<> registerAspect(Aspect* entry) {
        if(entry->getSlot().empty()) {
            return {false, "Slot type cannot be empty"};
        }
        if(entry->getKey().empty()) {
            return {false, "Save key cannot be empty"};
        }

        auto &slotreg = aspectRegistry[std::string(entry->getSlot())];
        slotreg[std::string(entry->getKey())] = entry;
        return {true, std::nullopt};
    }

    std::unordered_map<std::string, Aspect*> aspectSlotDefaults;


}