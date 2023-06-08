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
        auto &oinfo = registry.get<components::ObjectInfo>(ent);
        if(oinfo.typeFlags.test(2)) {
            return {false, "A character cannot be moved into another character!"};
        }
        return {true, std::nullopt};
    }

    void registerBaseAPI() {
        atPreMove.setOverride(3, roomAtPreMove);
        atPreMove.setOverride(4, exitAtPreMove);
        atPreMove.setOverride(2, characterAtPreMove);
    }
}