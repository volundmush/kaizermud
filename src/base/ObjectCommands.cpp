#include "kaizermud/base/ObjectCommands.h"
#include "kaizermud/Api.h"
#include "boost/algorithm/string.hpp"
#include "kaizermud/CallParameters.h"
#include "kaizermud/Components.h"

namespace kaizer::base {

    void ObjLook::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {
        auto loc = getRelation(ent, "location");
        if(!registry.valid(loc)) {
            sendText(ent, "You are nowhere.");
            return;
        }
        atLook(ent, loc);
    }

    void ObjHelp::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    static const std::unordered_map<std::string, std::string> exitAliases = {
            {"n", "north"},
            {"s", "south"},
            {"e", "east"},
            {"w", "west"},
            {"u", "up"},
            {"d", "down"},
            {"nw", "northwest"},
            {"ne", "northeast"},
            {"sw", "southwest"},
            {"se", "southeast"},
            {"in", "inside"},
            {"out", "outside"}
    };

    OpResult<> ObjMove::canExecute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {
        // This command requires that ent be in a location.
        auto loc = getRelation(ent, "location");
        if(!registry.valid(loc)) {
            return {false, "You are somewhere beyond space and time... ordinary movement will not avail you."};
        }
        return {true, std::nullopt};
    }

    void ObjMove::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {
        // Next we must determine the direction that the player wants to move.
        // If this command was called as move, go, or mv, we need to retrieve args
        // from input. Otherwise, input["cmd"] itself is the direction.

        std::string dir;
        auto cmd = input["cmd"];
        if (boost::iequals(cmd, "move") || boost::iequals(cmd, "go") || boost::iequals(cmd, "mv")) {
            dir = input["args"];
            if(dir.empty()) {
                sendText(ent, "Usage: move <direction>");
                return;
            }
        } else {
            dir = cmd;
        }
        boost::to_lower(dir);

        // now that we have a dir, we check to see if it is in exitAliases and replace it if so.
        auto it = exitAliases.find(dir);
        if (it != exitAliases.end()) {
            dir = it->second;
        }

        // Now we have a direction. We need to find the exit that matches it.
        auto loc = getRelation(ent, "location");
        auto exits = registry.try_get<components::Exits>(loc);
        if (exits->data.empty()) {
            sendText(ent, "You can't go anywhere from here.");
            return;
        }

        auto &ex = exits->data;

        // Now we have a list of exits. We need to find the one that matches the direction.
        auto exit = ex.find(dir);

        if (exit == ex.end()) {
            sendText(ent, "You can't go that way.");
            return;
        }

        // Now that we have an exit, we schedule the move.
        CallParameters param;
        param.setEntity("mover", ent);
        param.setEntity("exit", exit->second);
        auto &exdata = registry.get<components::Exit>(exit->second);
        param.setEntity("destination", exdata.destination);
        param.setString("moveType", "move");

        auto &pmove = registry.get_or_emplace<components::PendingMove>(ent);
        pmove.params = param;
        pmove.reportTo = ent;
    }


    void ObjQuit::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjSay::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjPose::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjSemipose::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjWhisper::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjShout::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjGet::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjTake::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjPut::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjGive::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjDrop::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjInventory::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void ObjEquip::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {

    }

    void registerObjectCommands() {
        std::vector<std::shared_ptr<Command>> commands;
        commands.push_back(std::make_shared<ObjLook>());
        commands.push_back(std::make_shared<ObjHelp>());
        commands.push_back(std::make_shared<ObjMove>());
        commands.push_back(std::make_shared<ObjQuit>());
        commands.push_back(std::make_shared<ObjSay>());
        commands.push_back(std::make_shared<ObjPose>());
        commands.push_back(std::make_shared<ObjSemipose>());
        commands.push_back(std::make_shared<ObjWhisper>());
        commands.push_back(std::make_shared<ObjShout>());
        commands.push_back(std::make_shared<ObjGet>());
        commands.push_back(std::make_shared<ObjTake>());
        commands.push_back(std::make_shared<ObjPut>());
        commands.push_back(std::make_shared<ObjGive>());
        commands.push_back(std::make_shared<ObjDrop>());
        commands.push_back(std::make_shared<ObjInventory>());
        commands.push_back(std::make_shared<ObjEquip>());

        for(auto &command : commands) {
            registerCommand(command);
        }
    }

}