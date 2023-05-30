#include "kaizermud/base.h"
#include "kaizermud/game.h"
#include "kaizermud/Components.h"
#include "kaizermud/Api.h"
#include "kaizermud/utils.h"

namespace kaizer {
    entt::registry registry;
    std::unordered_map<ObjectID, entt::entity> entities;

    OpResult<entt::entity> createEntity(std::optional<ObjectID> id) {

        ObjectID newID;

        if(id.has_value()) {
            if(entities.count(id.value())) {
                return {entt::null, "Object ID already in use"};
            }
            newID = id.value();
        } else {
            newID = getNextAvailableID();
        }

        auto ent = registry.create();
        auto &info = registry.get_or_emplace<components::ObjectInfo>(ent);
        info.id = newID;
        entities[newID] = ent;
        atCreate(ent);
        return {ent, std::nullopt};
    }
}