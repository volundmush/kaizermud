#include "kaizermud/Systems.h"
#include "kaizermud/game.h"
#include "kaizermud/Session.h"
#include "kaizermud/Components.h"
#include "kaizermud/Commands.h"
#include "kaizermud/Api.h"
#include "boost/algorithm/string.hpp"

namespace kaizer {
    std::vector<std::shared_ptr<System>> sortedSystems;
    std::unordered_map<std::string, std::shared_ptr<System>> systemRegistry;

    void registerSystem(std::shared_ptr<System> system) {
        auto name = std::string(system->getName());
        if(name.empty()) {
            throw std::runtime_error("System name cannot be empty");
        }
        systemRegistry[name] = system;
    }

    void sortSystems() {
        sortedSystems.clear();
        for(auto& [name, system] : systemRegistry) {
            sortedSystems.push_back(system);
        }
        std::sort(sortedSystems.begin(), sortedSystems.end(), [](std::shared_ptr<System> a, std::shared_ptr<System> b) {
            return a->getPriority() < b->getPriority();
        });
    }

    boost::asio::awaitable<bool> System::shouldRun(double deltaTime) {
        co_return true;
    }

    boost::asio::awaitable<void> System::run(double deltaTime) {
        co_return;
    }

    boost::asio::awaitable<void> ProcessConnections::run(double deltaTime) {
        // First, handle any disconnected connections.
        auto disconnected = state::disconnected_connections;
        for (const auto &id : disconnected) {
            auto it = state::connections.find(id);
            if (it != state::connections.end()) {
                auto& conn = it->second;
                conn->onNetworkDisconnected();
                state::connections.erase(it);
                state::disconnected_connections.erase(id);
            }
        }

        // Second, welcome any new connections!
        auto pending = state::pending_connections;
        for(const auto& id : pending) {
            auto it = state::connections.find(id);
            if (it != state::connections.end()) {
                auto& conn = it->second;
                // Need a proper welcoming later....
                conn->onWelcome();
                state::pending_connections.erase(id);
            }
        }

        // Next, we must handle the heartbeat routine for each connection.
        for(auto& [id, conn] : state::connections) {
            conn->onHeartbeat(deltaTime);
        }

        co_return;
    }

    boost::asio::awaitable<void> ProcessSessions::run(double deltaTime) {
        for(auto& [id, session] : state::sessions) {
            session->onHeartbeat(deltaTime);
        }
        co_return;
    }

    boost::asio::awaitable<void> ProcessOutput::run(double deltaTime) {
        for(auto& [id, session] : state::sessions) {
            session->sendOutput(deltaTime);
        }
        co_return;
    }

    void ProcessCommands::handleBadMatch(entt::entity ent, std::unordered_map<std::string, std::string>& input) {
        sendText(ent, "I don't understand that command.\n");
    }

    void ProcessCommands::handleNotFound(entt::entity ent, std::unordered_map<std::string, std::string> &input) {
        sendText(ent, "I don't understand that command.\n");
    }

    bool ProcessCommands::checkHooks(entt::entity ent, std::unordered_map<std::string, std::string> &input) {
        // This one will check if something else is going to intercept the command. That might be a script
        // or other special circumstance.
        // Return true if the command was handled, else return false.
        return false;
    }

    bool ProcessCommands::checkCommands(entt::entity ent, std::unordered_map<std::string, std::string> &input) {
        // This one will check if the command is a valid command.
        // Return true if the command was handled, else return false.
        auto commands = getSortedCommands(ent);
        for(auto &[key, cmd] : commands) {
            if(boost::iequals(key, input["cmd"])) {
                // We have a match!
                auto [canExecute, err] = cmd->canExecute(ent, input);
                if(!canExecute) {
                    sendText(ent, err.value());
                } else {
                    // We can execute the command!
                    cmd->execute(ent, input);
                }
                return true;
            }
        }
        return false;
    }

    boost::asio::awaitable<void> ProcessCommands::run(double deltaTime) {
        auto view = registry.view<components::PendingCommand>();

        for(auto& entity : view) {
            auto &pending = view.get<components::PendingCommand>(entity);

            // The above commands are already sorted by category.
            // The vector of tuples may contain duplicate Command* pointers if
            // some entries are aliasess, but that's intentional.

            // First, we will parse pending.input against our command_regex...
            // If we have a match, we will then check if the command can be executed.
            auto match_map = parseCommand(pending.input);
            if(match_map.empty()) {
                handleBadMatch(entity, match_map);
                continue;
            }

            if(checkHooks(entity, match_map)) {
                // The hook handled the command.
                continue;
            }

            if(checkCommands(entity, match_map)) {
                // The command was handled.
                continue;
            }

            // If we get here, we have no idea what the command is trying to do.
            handleNotFound(entity, match_map);
        }
        // Clear all PendingCommands.
        registry.clear<components::PendingCommand>();
        co_return;
    }

    boost::asio::awaitable<void> ProcessMovement::run(double deltaTime) {
        auto view = registry.view<components::PendingMove>();

        for(auto& entity : view) {
            auto &pending = view.get<components::PendingMove>(entity);
            auto [res, err] = moveTo(entity, pending.params);
            if(!res) {
                sendText(pending.reportTo, err.value());
            }
        }
        registry.clear<components::PendingMove>();

        co_return;
    }

    boost::asio::awaitable<void> ProcessLook::run(double deltaTime) {
        auto view = registry.view<components::PendingLook>();

        for(auto& entity : view) {
            auto &pending = view.get<components::PendingLook>(entity);
            atLook(entity, pending.target);
        }
        registry.clear<components::PendingLook>();
        co_return;
    }

    void registerBaseSystems() {
        registerSystem(std::make_shared<ProcessConnections>());
        registerSystem(std::make_shared<ProcessSessions>());
        registerSystem(std::make_shared<ProcessOutput>());
        registerSystem(std::make_shared<ProcessCommands>());
        registerSystem(std::make_shared<ProcessMovement>());
        registerSystem(std::make_shared<ProcessLook>());

    }

}