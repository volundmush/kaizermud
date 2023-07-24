#include "kaizermud/base.h"
#include "kaizermud/game.h"
#include "kaizermud/Components.h"
#include "kaizermud/utils.h"

namespace kaizer {
    entt::registry registry;
    std::unordered_map<ObjectID, entt::entity> entities;

}