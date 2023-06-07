#include "kaizermud/base/AdminCommands.h"
#include "kaizermud/Search.h"
#include "kaizermud/Api.h"
#include "fmt/format.h"

namespace kaizer::base {
    void AdmTeleport::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {
        // lsargs is who or what's being teleported and rsargs is the destination...
        auto lsargs = input["lsargs"];
        auto rsargs = input["rsargs"];

        if (lsargs.empty() || rsargs.empty()) {
            sendText(ent, "Usage: @teleport <target>=<destination>");
            return;
        }

        auto loc = getRelation(ent, "location");

        auto lsearch = Search(ent).useID(true).useSelf(true).useAll(false).useHere(true);
        if(registry.valid(loc)) {
            lsearch.in(loc);
        }
        auto target = lsearch.find(lsargs);
        if (target.empty()) {
            sendText(ent, "No target found.");
            return;
        }

        auto rsearch = Search(ent).useID(true).useSelf(true).useAll(false).useHere(true);
        if(registry.valid(loc)) {
            rsearch.in(loc);
        }
        auto destination = rsearch.find(rsargs);
        if (destination.empty()) {
            sendText(ent, "No destination found.");
            return;
        }

        CallParameters params;
        params.setEntity("destination", destination[0]);
        params.setEntity("mover", target[0]);
        params.setString("moveType", "system");
        auto [res, err] = moveTo(target[0], params);

        if (res) {
            sendText(ent, "Teleported.");
        } else {
            sendText(ent, fmt::format("Teleport failed: {}", err.value()));
        }

    }

    void registerAdminCommands() {
        registerCommand(std::make_shared<AdmTeleport>());
    }
}