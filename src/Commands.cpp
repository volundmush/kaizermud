#include "kaizermud/Commands.h"

namespace kaizermud::game {

    OpResult<> Command::canExecute() {
        return {true, std::nullopt};
    }

    void Command::execute() {

    }

    void Command::parse(const std::string& input) {

    }

    int Command::priority() {
        return 0;
    }

    bool Command::isAvailable() {
        return true;
    }

    std::string Command::getHelp() {
        return "";
    }

    void Command::reset() {
        args.clear();
    }

    bool Command::match(const std::string& input) {
        return false;
    }

    std::unordered_map<std::string, std::unordered_map<std::string, CommandEntry>> commandRegistry;

    OpResult<> registerCommand(CommandEntry entry) {
        if(entry.objType.empty()) {
            return {false, "CommandEntry objType cannot be empty"};
        }
        if(entry.cmdName.empty()) {
            return {false, "CommandEntry cmdName cannot be empty"};
        }

        if(entry.ctor == nullptr) {
            return {false, "CommandEntry ctor cannot be null"};
        }

        auto &reg = commandRegistry[entry.objType];

        if(reg.find(entry.cmdName) != reg.end()) {
            return {false, "CommandEntry already registered"};
        }

        reg[entry.cmdName] = entry;
        return {true, std::nullopt};
    }

    bool CommandHandler::execute(const std::string& input) {
        // We only load the commands when we need them.
        // Otherwise, every object in the game would have
        // to load all the commands, even if they're not
        // going to use 'em.
        if(!loaded) {
            load();
        }

        // Commands have already been sorted by priority
        // and other factors, so just  iterate through them.
        for(auto &cmd : commands) {
            if(cmd->match(input)) {
                try {
                    cmd->parse(input);
                    auto [res, err] = cmd->canExecute();
                    if(res) {
                        cmd->execute();
                    }
                    else {
                        // TODO: send error message to player
                    }
                }
                catch(std::exception &e) {
                    // TODO: send error to player and log it;
                }
                cmd->reset();
                return true;
            }
        }
        return false;
    }

    void CommandHandler::load() {
        /*
        loaded = true;

        std::unordered_map<std::string, CommandEntry> cmdsToLoad;

        // First, we load the commands for the object's type
        for(const auto& type : obj->getTypes()) {
            auto it = commandRegistry.find(std::string(type));
            if(it != commandRegistry.end()) {
                for(const auto& [cmdName, entry] : it->second) {
                    cmdsToLoad[cmdName] = entry;
                }
            }
        }

        // Now we must instantiate them...
        for(const auto& [cmdName, entry] : cmdsToLoad) {
            commands.push_back(std::move(entry.ctor(this)));
        }

        // Last, we must sort the vector by the Command's priority.
        std::sort(commands.begin(), commands.end(), [](const auto& a, const auto& b) {
            return a->priority() < b->priority();
        });

        // Thank you copilot. I love you.
*/
    }

}