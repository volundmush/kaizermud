#include "kaizermud/Commands.h"

namespace kaizer {

    OpResult<> Command::canExecute(entt::entity ent) {
        return {true, std::nullopt};
    }

    void Command::execute(entt::entity ent, std::string_view input) {

    }

    int Command::getPriority(entt::entity ent) {
        return priority;
    }

    bool Command::isAvailable(entt::entity ent) {
        return true;
    }

    std::string Command::getHelp() {
        return "";
    }

    bool Command::match(entt::entity ent, std::string_view input) {
        return false;
    }

    std::unordered_map<std::string, std::unordered_map<std::string, Command*>> commandRegistry;

    OpResult<> registerCommand(Command* entry) {
        if(entry->objType.empty()) {
            return {false, "CommandEntry objType cannot be empty"};
        }
        if(entry->cmdName.empty()) {
            return {false, "CommandEntry cmdName cannot be empty"};
        }

        auto &reg = commandRegistry[entry->objType];

        if(reg.find(entry->cmdName) != reg.end()) {
            return {false, "CommandEntry already registered"};
        }

        reg[entry->cmdName] = entry;
        return {true, std::nullopt};
    }

}