#pragma once

#include "kaizermud/Commands.h"

namespace kaizer::base {
    struct LoginCommandPlay : LoginCommand {
        std::string_view getCmdName() override { return "play"; };
        std::set<std::string> getAliases() override { return {"select", "p"}; };
        void execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) override;
    };

    struct LoginCommandNew : LoginCommand {
        std::string_view getCmdName() override { return "new"; };
        std::set<std::string> getAliases() override { return {"create", "register", "reg", "cr"}; };
        void execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) override;
    };

    void registerLoginCommands();
}