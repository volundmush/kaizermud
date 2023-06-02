#pragma once
#include "kaizermud/base.h"
#include <boost/regex.hpp>


namespace kaizer {

    /*
     * The base Command class used for all commands that'll be used by
     * game objects.
     */

    struct BaseCommand {
        [[nodiscard]] virtual std::set<std::string> getAliases() {return {};};
        [[nodiscard]] virtual std::set<std::string> getKeys();
        [[nodiscard]] virtual std::string_view getCmdName() = 0;
        [[nodiscard]] virtual std::string getHelp() {return "";};
        [[nodiscard]] virtual std::string getHelpCategory() {return "Uncategorized";};
        [[nodiscard]] virtual int getPriority() {return 0;};
    };

    struct Command : BaseCommand {
        [[nodiscard]] virtual std::string_view getType() = 0;
        [[nodiscard]] virtual OpResult<> canExecute(entt::entity ent, std::unordered_map<std::string, std::string>& input);
        virtual void execute(entt::entity ent, std::unordered_map<std::string, std::string>& input);
        [[nodiscard]] virtual bool isAvailable(entt::entity ent) {return true;};

    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Command>>> commandRegistry, expandedCommandRegistry;
    OpResult<> registerCommand(const std::shared_ptr<Command>& entry);

    extern boost::regex command_regex;

    std::unordered_map<std::string, std::string> parseCommand(std::string_view input);

    /*
     * The base Command class used for all commands that'll be used by
     * connections at the welcome screen, including logging in and creating
     * accounts.
     */
    struct ConnectCommand : BaseCommand {
        [[nodiscard]] virtual OpResult<> canExecute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input);
        virtual void execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input);
        [[nodiscard]] virtual bool isAvailable(const std::shared_ptr<ClientConnection>& connection) {return true;};
    };

    extern std::unordered_map<std::string, std::shared_ptr<ConnectCommand>> connectCommandRegistry, expandedConnectCommandRegistry;
    OpResult<> registerConnectCommand(const std::shared_ptr<ConnectCommand>& entry);
    /*
     * After logging into an Account, players will have access to these
     * commands.
     */
    struct LoginCommand : BaseCommand {
        [[nodiscard]] virtual OpResult<> canExecute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input);
        virtual void execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input);
        [[nodiscard]] virtual bool isAvailable(const std::shared_ptr<ClientConnection>& connection) {return true;};
    };

    extern std::unordered_map<std::string, std::shared_ptr<LoginCommand>> loginCommandRegistry, expandedLoginCommandRegistry;
    OpResult<> registerLoginCommand(const std::shared_ptr<LoginCommand>& entry);

    void expandCommands();
}