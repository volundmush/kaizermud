#include "kaizermud/Commands.h"
#include "boost/algorithm/string.hpp"
#include "spdlog/spdlog.h"

namespace kaizer {

    boost::regex command_regex(R"((?i)^(?<full>(?<cmd>[^\s\/]+)(?<switches>(\/\w+){0,})?(?:\s+(?<args>(?<lsargs>[^=]+)(?:=(?<rsargs>.*))?))?))");

    std::unordered_map<std::string, std::string> parseCommand(std::string_view input) {
        std::unordered_map<std::string, std::string> out;
        boost::smatch match;
        auto input_str = std::string(input);
        if(boost::regex_match(input_str, match, command_regex)) {
            out["full"] = match["full"];
            out["cmd"] = match["cmd"];
            out["switches"] = match["switches"];
            out["args"] = match["args"];
            out["lsargs"] = match["lsargs"];
            out["rsargs"] = match["rsargs"];
        }
        return out;
    }

    OpResult<> Command::canExecute(entt::entity ent, std::unordered_map<std::string, std::string>& input) {
        return {true, std::nullopt};
    }

    std::set<std::string> BaseCommand::getKeys() {
        std::set<std::string> out;
        out.insert(std::string(getCmdName()));
        auto aliases = getAliases();
        out.insert(aliases.begin(), aliases.end());
        return out;
    }

    void Command::execute(entt::entity ent, std::unordered_map<std::string, std::string>& input) {
        spdlog::warn("Command {} not implemented", input["cmd"]);
    }

    std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Command>>> commandRegistry, expandedCommandRegistry;

    OpResult<> registerCommand(const std::shared_ptr<Command>& entry) {
        if(entry->getType().empty()) {
            return {false, "CommandEntry Type cannot be empty"};
        }
        if(entry->getCmdName().empty()) {
            return {false, "CommandEntry cmdName cannot be empty"};
        }
        auto &reg = commandRegistry[std::string(entry->getType())];
        reg[std::string(entry->getCmdName())] = entry;
        return {true, std::nullopt};
    }

    OpResult<> registerLoginCommand(const std::shared_ptr<LoginCommand>& entry) {
        if(entry->getCmdName().empty()) {
            return {false, "CommandEntry cmdName cannot be empty"};
        }
        loginCommandRegistry[std::string(entry->getCmdName())] = entry;
        return {true, std::nullopt};
    }

    OpResult<> registerConnectCommand(const std::shared_ptr<ConnectCommand>& entry) {
        if(entry->getCmdName().empty()) {
            return {false, "CommandEntry cmdName cannot be empty"};
        }
        connectCommandRegistry[std::string(entry->getCmdName())] = entry;
        return {true, std::nullopt};
    }

    std::unordered_map<std::string, std::shared_ptr<ConnectCommand>> connectCommandRegistry, expandedConnectCommandRegistry;
    std::unordered_map<std::string, std::shared_ptr<LoginCommand>> loginCommandRegistry, expandedLoginCommandRegistry;

    OpResult<> ConnectCommand::canExecute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) {
        return {true, std::nullopt};
    }

    void ConnectCommand::execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) {
        spdlog::warn("ConnectCommand {} not implemented", input["cmd"]);
    }

    OpResult<> LoginCommand::canExecute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) {
        return {true, std::nullopt};
    }

    void LoginCommand::execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) {
        spdlog::warn("LoginCommand {} not implemented", input["cmd"]);
    }

    void expandCommands() {
        for(auto& [type, commands] : commandRegistry) {
            for(auto& [cmdName, cmd] : commands) {
                for(auto& key : cmd->getKeys()) {
                    expandedCommandRegistry[type][boost::algorithm::to_lower_copy(key)] = cmd;
                }
            }
        }
        for(auto& [cmdName, cmd] : connectCommandRegistry) {
            for(auto& key : cmd->getKeys()) {
                expandedConnectCommandRegistry[boost::algorithm::to_lower_copy(key)] = cmd;
            }
        }
        for(auto& [cmdName, cmd] : loginCommandRegistry) {
            for(auto& key : cmd->getKeys()) {
                expandedLoginCommandRegistry[boost::algorithm::to_lower_copy(key)] = cmd;
            }
        }
    }

}