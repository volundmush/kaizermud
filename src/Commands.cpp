#include "kaizermud/Commands.h"
#include "boost/algorithm/string.hpp"
#include "spdlog/spdlog.h"

namespace kaizer {

    boost::regex command_regex(R"((?i)^(?<full>(?<cmd>[^\s\/]+)(?<switches>(\/\w+){0,})?(?:\s+(?<args>(?<lsargs>[^=]+)(?:=(?<rsargs>.*))?))?))");

    OpResult<> Command::canExecute(entt::entity ent, std::unordered_map<std::string, std::string>& input) {
        return {true, std::nullopt};
    }

    std::set<std::string> Command::getKeys() {
        std::set<std::string> out;
        out.insert(std::string(getCmdName()));
        out.insert(getAliases().begin(), getAliases().end());
        return out;
    }

    void Command::execute(entt::entity ent, std::unordered_map<std::string, std::string>& input) {
        spdlog::warn("Command {} not implemented", input["cmd"]);
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Command>>> commandRegistry;

    OpResult<> registerCommand(std::shared_ptr<Command> entry) {
        if(entry->getType().empty()) {
            return {false, "CommandEntry Type cannot be empty"};
        }
        if(entry->getCmdName().empty()) {
            return {false, "CommandEntry cmdName cannot be empty"};
        }

        auto &reg = commandRegistry[std::string(entry->getType())];

        for(auto& key : entry->getKeys()) {
            auto lower = boost::algorithm::to_lower_copy(std::string(key));
            if(reg.find(lower) != reg.end()) {
                return {false, "CommandEntry key already registered"};
            }
            reg[lower] = entry;
        }

        return {true, std::nullopt};
    }

}