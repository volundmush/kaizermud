#include "kaizermud/Types.h"

namespace kaizer {

    void Type::onLoad(entt::entity ent) const {

    }

    void Type::onAdd(entt::entity ent) const {

    }

    void Type::onRemove(entt::entity ent) const {

    }

    int Type::getSortPriority() const {
        return 0;
    }

    std::vector<std::string> Type::getAspectSlots() const {
        return {};
    }

    std::vector<std::string> Type::getQuirkSlots() const {
        return {};
    }

    std::vector<std::string> Type::getStatSlots() const {
        return {};
    }

    std::vector<std::string> Type::getEquipSlots() const {
        return {};
    }


    std::unordered_map<std::string, Type*> typeRegistry;

}