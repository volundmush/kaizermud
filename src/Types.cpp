#include "kaizermud/Types.h"

namespace kaizer {

    void Type::onLoad(entt::entity ent) const {

    }

    void Type::onAdd(entt::entity ent) const {

    }

    void Type::onRemove(entt::entity ent) const {

    }

    std::vector<std::string> Type::getStatSlots() const {
        return {};
    }

    std::vector<std::string> Type::getEquipSlots() const {
        return {};
    }

    std::vector<std::shared_ptr<Type>> typeRegistry;

    void registerType(const std::shared_ptr<Type>& entry) {
        while(typeRegistry.size() < entry->getID() + 1) {
            typeRegistry.emplace_back(nullptr);
        }
        typeRegistry[entry->getID()] = entry;
    }

}