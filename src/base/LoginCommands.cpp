#include "kaizermud/base/LoginCommands.h"
#include "kaizermud/Components.h"
#include "kaizermud/utils.h"
#include "kaizermud/Database.h"

namespace kaizer::base {

    void LoginCommandPlay::execute(const std::shared_ptr<ClientConnection>& connection, std::unordered_map<std::string, std::string>& input) {
        auto acc = connection->getAccount();
        std::vector<std::pair<ObjectID, std::string>> characters;
        SQLite::Statement q(*db, "SELECT character FROM playerCharacters WHERE account=?");
        q.bind(1, acc);
        while(q.executeStep()) {
            auto id = q.getColumn(0).getInt64();
            auto t = getType(id);
            auto name = t->getDisplayName(id, id);
            characters.emplace_back(id, name);
        }

        auto name = input["args"];
        if(name.empty()) {
            connection->sendText("Please enter a name.\n");
            return;
        }

        auto c = partialMatch(name, characters.begin(), characters.end(), false, [](auto &c) {
            return c.second;
        });

        if(!c.has_value()) {
            connection->sendText("No character found with that name.\n");
            return;
        }

        connection->createOrJoinSession(c.value().first);
    }

    void LoginCommandNew::execute(const std::shared_ptr<ClientConnection> &connection,
                                  std::unordered_map<std::string, std::string> &input) {

    }

    void registerLoginCommands() {
        registerLoginCommand(std::make_shared<LoginCommandPlay>());
        registerLoginCommand(std::make_shared<LoginCommandNew>());
    }

}