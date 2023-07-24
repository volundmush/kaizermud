#include <kaizermud/CallParameters.h>
#include "kaizermud/base/AdminCommands.h"
#include "kaizermud/Search.h"
#include "fmt/format.h"
#include "kaizermud/Types.h"
#include "kaizermud/Components.h"
#include "kaizermud/Database.h"

namespace kaizer::base {
    void AdmTeleport::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {
        // lsargs is who or what's being teleported and rsargs is the destination...
        auto lsargs = input["lsargs"];
        auto rsargs = input["rsargs"];
        auto &objinfo = registry.get<::kaizer::components::ObjectInfo>(ent);
        if (lsargs.empty() || rsargs.empty()) {
            objinfo.type->sendText(objinfo.id, "Usage: @teleport <target>=<destination>");
            return;
        }

        auto loc = objinfo.type->getRelation(objinfo.id, static_cast<int>(RelationKind::Location));

        auto lsearch = Search(objinfo.id).useID(true).useSelf(true).useAll(false).useHere(true);
        if(loc != -1) {
            lsearch.in(loc);
        }
        auto target = lsearch.find(lsargs);
        if (target.empty()) {
            objinfo.type->sendText(objinfo.id, "No target found.");
            return;
        }

        auto rsearch = Search(objinfo.id).useID(true).useSelf(true).useAll(false).useHere(true);
        if(loc != -1) {
            rsearch.in(loc);
        }
        auto destination = rsearch.find(rsargs);
        if (destination.empty()) {
            objinfo.type->sendText(objinfo.id, "No destination found.");
            return;
        }

        CallParameters params;
        params.setEntity("destination", destination[0]);
        params.setEntity("mover", target[0]);
        params.setString("moveType", "system");

        auto tent = getEntity(target[0]);
        auto &tinfo = registry.get<::kaizer::components::ObjectInfo>(tent);
        auto &pending = registry.get_or_emplace<::kaizer::components::PendingMove>(tent);
        pending.params = params;
        pending.reportTo = objinfo.id;

    }

    void registerAdminCommands() {
        registerCommand(std::make_shared<AdmTeleport>());
    }
}