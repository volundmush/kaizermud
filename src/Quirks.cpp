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

    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, Quirk*>>> quirkRegistry;

    OpResult<> registerQuirk(Quirk* entry) {
        if(entry->objType.empty()) {
            return {false, "Object type cannot be empty"};
        }
        if(entry->slotType.empty()) {
            return {false, "Slot type cannot be empty"};
        }
        if(entry->key.empty()) {
            return {false, "Save key cannot be empty"};
        }

        auto &objreg = quirkRegistry[entry->objType];
        auto &slotreg = objreg[entry->slotType];
        slotreg[entry->key] = entry;
        return {true, std::nullopt};
    }

    std::unordered_map<std::string, std::vector<std::pair<std::pair<std::string_view, std::string_view>, std::unordered_map<std::string, Quirk*>>>> quirksCache;


    const std::unordered_map<std::string, Quirk*>& getQuirks(std::string_view slot, const std::pair<std::string_view, std::string_view>& objType) {
        // Yank the slotreg out of the cache whether it exists or not...
        auto &slotreg = quirksCache[std::string(slot)];

        auto found = std::find_if(slotreg.begin(), slotreg.end(), [objType](auto &x) {return x.first == objType;});
        if(found != slotreg.end()) {
            return found->second;
        }
        // We didn't find it, so we'll have to make one...
        auto &newreg = slotreg.emplace_back(objType, std::unordered_map<std::string, Quirk*>()).second;

        for(const auto& type : {objType.first, objType.second}) {
            auto objreg = quirkRegistry.find(std::string(type));
            if(objreg == quirkRegistry.end()) {
                continue;
            }
            auto sreg = objreg->second;
            auto quirkreg = sreg.find(std::string(slot));
            if(quirkreg == sreg.end()) {
                continue;
            }
            auto quirks = quirkreg->second;
            for(const auto& [name, entry] : quirks) {
                newreg[name] = entry;
            }
        }
        return newreg;

    }
}