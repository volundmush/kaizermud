#include "kaizermud/base/LoginCommands.h"
#include "kaizermud/Components.h"
#include "kaizermud/utils.h"
#include "kaizermud/Api.h"

namespace kaizer::base {

    void LoginCommandPlay::execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) {
        auto acc = connection->getAccount();
        auto &accdata = registry.get<components::Account>(acc);

        std::vector<entt::entity> characters;
        for(auto &character : accdata.characters) {
            auto found = entities.find(character);
            if(found == entities.end()) {
                continue;
            }
            characters.push_back(found->second);
        }

        auto name = input["args"];
        if(name.empty()) {
            connection->sendText("Please enter a name.\n");
            return;
        }

        auto c = partialMatch(name, characters.begin(), characters.end(), false, [](auto &c) {
            return std::string(getString(c, "name").value_or(""));
        });

        if(!c.has_value()) {
            connection->sendText("No character found with that name.\n");
            return;
        }

        connection->createOrJoinSession(c.value());
    }

    void LoginCommandNew::execute(const std::shared_ptr<ClientConnection> &connection,
                                  std::unordered_map<std::string, std::string> &input) {

    }

    void registerLoginCommands() {
        registerLoginCommand(std::make_shared<LoginCommandPlay>());
        registerLoginCommand(std::make_shared<LoginCommandNew>());
    }

}