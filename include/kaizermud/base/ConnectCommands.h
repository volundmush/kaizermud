#pragma once

#include "kaizermud/Commands.h"

namespace kaizer::base {
    struct ConnectCommandCreate : ConnectCommand {
        std::string_view getCmdName() override { return "create"; };
        std::set<std::string> getAliases() override { return {"cr", "register"}; };
        void execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) override;
    };

    struct ConnectCommandQuit : ConnectCommand {
        std::string_view getCmdName() override { return "quit"; };
        std::set<std::string> getAliases() override { return {"q", "qq", "exit", "logout"}; };
        void execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) override;
    };

    struct ConnectCommandHelp : ConnectCommand {
        std::string_view getCmdName() override { return "help"; };
        std::set<std::string> getAliases() override { return {"h", "?"}; };
        void execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) override;
    };

    struct ConnectCommandWho : ConnectCommand {
        std::string_view getCmdName() override { return "who"; };
        std::set<std::string> getAliases() override { return {"w", "wh"}; };
        void execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) override;
    };

    struct ConnectCommandLook : ConnectCommand {
        std::string_view getCmdName() override { return "look"; };
        std::set<std::string> getAliases() override { return {"l"}; };
        void execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) override;
    };

    struct ConnectCommandConnect : ConnectCommand {
        std::string_view getCmdName() override { return "connect"; };
        std::set<std::string> getAliases() override { return {"c", "co", "con", "cd", "ch"}; };
        void execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) override;
    };

    void registerConnectCommands();

}