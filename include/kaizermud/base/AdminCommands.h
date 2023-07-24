#pragma once

#include "kaizermud/Commands.h"

namespace kaizer::base {

    struct AdmCmd : Command {
        [[nodiscard]] std::set<uint8_t> getCmdMask() override {return {0};};
    };

    struct AdmTeleport : AdmCmd {
        [[nodiscard]] std::string_view getCmdName() override {return "@teleport";};
        [[nodiscard]] std::set<std::string> getAliases() override {return {"@tel"};};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string>& input) override;
    };

    void registerAdminCommands();

}