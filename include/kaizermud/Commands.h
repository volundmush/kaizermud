#pragma once
#include "kaizermud/base.h"
#include <boost/regex.hpp>


namespace kaizer {

    struct Command {
        [[nodiscard]] virtual std::string_view getType() = 0;
        [[nodiscard]] virtual std::set<std::string> getAliases() {return {};};
        [[nodiscard]] virtual std::set<std::string> getKeys();
        [[nodiscard]] virtual std::string_view getCmdName() = 0;
        [[nodiscard]] virtual OpResult<> canExecute(entt::entity ent, std::unordered_map<std::string, std::string>& input);
        virtual void execute(entt::entity ent, std::unordered_map<std::string, std::string>& input);
        [[nodiscard]] virtual int getPriority() {return 0;};
        [[nodiscard]] virtual bool isAvailable(entt::entity ent) {return true;};
        [[nodiscard]] virtual std::string getHelp() {return "";};
        [[nodiscard]] virtual std::string getHelpCategory() {return "Uncategorized";};
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Command>>> commandRegistry;
    OpResult<> registerCommand(std::shared_ptr<Command> entry);

    extern boost::regex command_regex;
}