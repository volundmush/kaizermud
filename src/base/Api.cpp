#include "kaizermud/base/Api.h"

namespace kaizer::base {

    OpResult<> roomAtPreMove(entt::entity ent, const CallParameters& param) {
        return {false, "Rooms cannot be moved!"};
    }

    OpResult<> exitAtPreMove(entt::entity ent, const CallParameters& param) {
        return {false, "Exits cannot be moved!"};
    }

    OpResult<> characterAtPreMove(entt::entity ent, const CallParameters& param) {
        auto dest = param.getEntity("destination");
        if(getType(dest, "character")) {
            return {false, "A character cannot be moved into another character!"};
        }
        return {true, std::nullopt};
    }

    void registerBaseAPI() {
        atPreMove.setOverride("room", roomAtPreMove);
        atPreMove.setOverride("exit", exitAtPreMove);
        atPreMove.setOverride("character", characterAtPreMove);
    }
}