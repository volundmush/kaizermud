#include "kaizermud/Api.h"

namespace kaizer::base {

    OpResult<> roomAtPreMove(entt::entity ent, const CallParameters& param);
    OpResult<> exitAtPreMove(entt::entity ent, const CallParameters& param);
    OpResult<> characterAtPreMove(entt::entity ent, const CallParameters& param);

    void registerBaseAPI();

}