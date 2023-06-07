#pragma once

#include "kaizermud/Commands.h"

namespace kaizer::base {

    struct AdmCmd : Command {
        [[nodiscard]] std::string_view getType() override {return "object";};
    };

    struct AdmTeleport : AdmCmd {
        [[nodiscard]] std::string_view getCmdName() override {return "@teleport";};
        [[nodiscard]] std::set<std::string> getAliases() override {return {"@tel"};};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string>& input) override;
    };

    void registerAdminCommands();

}