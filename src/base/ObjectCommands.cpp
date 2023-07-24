#include "kaizermud/base/ObjectCommands.h"
#include "boost/algorithm/string.hpp"
#include "kaizermud/CallParameters.h"
#include "kaizermud/Components.h"
#include "kaizermud/Database.h"
#include "kaizermud/Types.h"

namespace kaizer::base {

    void ObjLook::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {
        auto &objinfo = registry.get<components::ObjectInfo>(ent);
        auto loc = objinfo.type->getRelation(objinfo.id, static_cast<int>(RelationKind::Location));
        if(loc == -1) {
            objinfo.type->sendText(objinfo.id, "You are nowhere.");
            return;
        }
        auto &look = registry.get_or_emplace<components::PendingLook>(ent);
        look.target = loc;
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
        auto &objinfo = registry.get<components::ObjectInfo>(ent);
        auto loc = objinfo.type->getRelation(objinfo.id, static_cast<int>(RelationKind::Location));
        if(loc == -1) {
            return {false, "You are somewhere beyond space and time... ordinary movement will not avail you."};
        }
        return {true, std::nullopt};
    }

    void ObjMove::execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) {
        // Next we must determine the direction that the player wants to move.
        // If this command was called as move, go, or mv, we need to retrieve args
        // from input. Otherwise, input["cmd"] itself is the direction.
        auto &objinfo = registry.get<components::ObjectInfo>(ent);
        std::string dir;
        auto cmd = input["cmd"];
        if (boost::iequals(cmd, "move") || boost::iequals(cmd, "go") || boost::iequals(cmd, "mv")) {
            dir = input["args"];
            if(dir.empty()) {
                objinfo.type->sendText(objinfo.id, "Usage: move <direction>");
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
        auto loc = objinfo.type->getRelation(objinfo.id, static_cast<int>(RelationKind::Location));
        auto locType = ::kaizer::getType(loc);
        auto exits = locType->getReverseRelation(loc, static_cast<int>(RelationKind::Exit));
        if (exits.empty()) {
            objinfo.type->sendText(objinfo.id, "You can't go anywhere from here.");
            return;
        }

        // Now we have a list of exits. We need to find the one that matches the direction.
        auto exit = std::find_if(exits.begin(), exits.end(), [&dir](auto &ex) {
            auto ext = ::kaizer::getType(ex);
            return boost::iequals(ext->getName(ex), dir);
        });

        if (exit == exits.end()) {
            objinfo.type->sendText(objinfo.id, "You can't go that way.");
            return;
        }

        // Now that we have an exit, we schedule the move.
        CallParameters param;
        param.setEntity("mover", objinfo.id);
        param.setEntity("exit", *exit);
        auto ext = ::kaizer::getType(*exit);
        auto dest = ext->getRelation(*exit, static_cast<int>(RelationKind::Destination));
        param.setEntity("destination", dest);
        param.setString("moveType", "move");

        auto &pmove = registry.get_or_emplace<components::PendingMove>(ent);
        pmove.params = param;
        pmove.reportTo = objinfo.id;
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
        registerCommand(std::make_shared<ObjLook>());
        registerCommand(std::make_shared<ObjHelp>());
        registerCommand(std::make_shared<ObjMove>());
        registerCommand(std::make_shared<ObjQuit>());
        registerCommand(std::make_shared<ObjSay>());
        registerCommand(std::make_shared<ObjPose>());
        registerCommand(std::make_shared<ObjSemipose>());
        registerCommand(std::make_shared<ObjWhisper>());
        registerCommand(std::make_shared<ObjShout>());
        registerCommand(std::make_shared<ObjGet>());
        registerCommand(std::make_shared<ObjTake>());
        registerCommand(std::make_shared<ObjPut>());
        registerCommand(std::make_shared<ObjGive>());
        registerCommand(std::make_shared<ObjDrop>());
        registerCommand(std::make_shared<ObjInventory>());
        registerCommand(std::make_shared<ObjEquip>());
    }

}