#include "kaizermud/base/ConnectCommands.h"
#include "kaizermud/ClientConnection.h"
#include "boost/algorithm/string.hpp"
#include "fmt/format.h"

namespace kaizer::base {
    static boost::regex loginRegex(R"(^(?:(?<username>".*?"|\S+))(?:\s+(?<password>.*))?$)");

    void ConnectCommandLook::execute(const std::shared_ptr<ClientConnection> &connection,
                                     std::unordered_map<std::string, std::string> &input) {
        connection->onWelcome();
    }

    void ConnectCommandConnect::execute(const std::shared_ptr<ClientConnection> &connection,
                                        std::unordered_map<std::string, std::string> &input) {
        boost::smatch match;
        if (boost::regex_match(input["args"], match, loginRegex)) {
            std::string username = match["username"];
            std::string password = match["password"];
            boost::trim_if(username, boost::algorithm::is_any_of("\""));
            auto [res, err] = connection->handleLogin(username, password);
            if(!res) {
                connection->sendText(fmt::format("Error: {}", err.value()));
                return;
            }
        } else {
            connection->sendText("syntax: connect <username> <password>, or connect \"<username>\" <password>");
        }
    }

    void ConnectCommandCreate::execute(const std::shared_ptr<ClientConnection> &connection,
                                       std::unordered_map<std::string, std::string> &input) {
        boost::smatch match;
        if (boost::regex_match(input["args"], match, loginRegex)) {
            std::string username = match["username"];
            std::string password = match["password"];
            boost::trim_if(username, boost::algorithm::is_any_of("\""));
            auto [res, err] = connection->createAccount(username, password);
            if(!registry.valid(res)) {
                connection->sendText(fmt::format("Error: {}", err.value()));
                return;
            }
        } else {
            connection->sendText("syntax: create <username> <password>, or create \"<username>\" <password>");
        }
    }

    void ConnectCommandQuit::execute(const std::shared_ptr<ClientConnection> &connection,
                                     std::unordered_map<std::string, std::string> &input) {

    }

    void ConnectCommandHelp::execute(const std::shared_ptr<ClientConnection> &connection,
                                     std::unordered_map<std::string, std::string> &input) {

    }

    void ConnectCommandWho::execute(const std::shared_ptr<ClientConnection> &connection,
                                     std::unordered_map<std::string, std::string> &input) {

    }

    void registerConnectCommands() {
        std::vector<std::shared_ptr<ConnectCommand>> commands;
        commands.emplace_back(std::make_shared<ConnectCommandCreate>());
        commands.emplace_back(std::make_shared<ConnectCommandQuit>());
        commands.emplace_back(std::make_shared<ConnectCommandHelp>());
        commands.emplace_back(std::make_shared<ConnectCommandWho>());
        commands.emplace_back(std::make_shared<ConnectCommandLook>());
        commands.emplace_back(std::make_shared<ConnectCommandConnect>());

        for(auto& cmd : commands) {
            registerConnectCommand(cmd);
        }
    }
}