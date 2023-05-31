#pragma once
#include "kaizermud/Commands.h"

namespace kaizer::base {

    // Object Commands.
    struct ObjCmd : Command {
        [[nodiscard]] std::string_view getType() override {return "object";};
    };

    /*
     * The look command is a staple of MUDs; it should show you where you are and what the room looks like.
     * It could also be used to look AT something, like a character or item in the room or in your inventory.
     */
    struct ObjLook : ObjCmd {
        [[nodiscard]] std::string_view getCmdName() override {return "look";};
        [[nodiscard]] std::set<std::string> getAliases() override {return {"l"};};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string>& input) override;
        [[nodiscard]] std::string getHelp() override;
    };

    /*
     * This help command will be able to get help on any command you have access to.
     */
    struct ObjHelp : ObjCmd {
        [[nodiscard]] std::string_view getCmdName() override {return "help";};
        [[nodiscard]] std::set<std::string> getAliases() override {return {"h"};};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string>& input) override;
        [[nodiscard]] std::string getHelp() override;
    };

    /*
     * This command will allow you to move from room to room.
     * This is an "object" command because many things should have this functionality
     * if an object is being possessed for normal control by a player or admin.
     * For example, controls might be passed to a spaceship or other vehicle.
     * Or, an admin might be possessing a statue (technically an item) and
     * wants to move to a different room...
     */
    struct ObjMove : ObjCmd {
        [[nodiscard]] std::string_view getCmdName() override { return "move"; };
        [[nodiscard]] std::set<std::string> getAliases() override { return {"mv", "go",
                                                                            "north", "n",
                                                                            "south", "s",
                                                                            "east", "e",
                                                                            "west", "w",
                                                                            "up", "u",
                                                                            "down", "d",
                                                                            "in", "out",
                                                                            "northwest", "nw",
                                                                            "northeast", "ne",
                                                                            "southwest", "sw",
                                                                            "southeast", "se"}; };
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
        [[nodiscard]] std::string getHelp() override;
    };

}