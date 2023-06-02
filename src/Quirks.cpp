#include "kaizermud/Quirks.h"
#include "kaizermud/utils.h"
#include "kaizermud/Components.h"

namespace kaizer {

    // Quirk

    void Quirk::onRemove(entt::entity ent) {

    }

    void Quirk::onAdd(entt::entity ent) {

    }

    void Quirk::onLoad(entt::entity ent) {

    }

    OpResult<> Quirk::canSet(entt::entity ent) {
        return {true, std::nullopt};
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Quirk>>> quirkRegistry;

    OpResult<> registerQuirk(const std::shared_ptr<Quirk>& entry) {
        if(entry->getSlot().empty()) {
            return {false, "Slot type cannot be empty"};
        }
        if(entry->getKey().empty()) {
            return {false, "Save key cannot be empty"};
        }

        auto &slot = quirkRegistry[std::string(entry->getSlot())];
        slot[std::string(entry->getKey())] = entry;
        return {true, std::nullopt};
    }

}