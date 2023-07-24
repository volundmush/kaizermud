#include "kaizermud/Session.h"
#include "kaizermud/game.h"
#include "kaizermud/Components.h"
#include "fmt/format.h"
#include "kaizermud/Types.h"
#include "kaizermud/Database.h"

namespace kaizer {

    Session::Session(ObjectID id, ObjectID account, entt::entity character) {
        this->id = id;
        this->character = character;
        puppet = character;
        this->account = account;
        created = std::chrono::system_clock::now();
    }

    void Session::start() {
        auto &objinfo = registry.get<components::ObjectInfo>(character);
        sendText(fmt::format("You have joined the game as {}\n", objinfo.type->getDisplayName(objinfo.id, objinfo.id)));
        objinfo.type->unstow(objinfo.id);
    }

    void Session::end() {
        auto &objinfo = registry.get<components::ObjectInfo>(character);
        if(puppet != character) {
            changePuppet(character);
        }
        objinfo.type->stow(objinfo.id);
    }

    void Session::send(const Message &msg) {

    }

    void Session::atObjectDeleted(entt::entity ent) {

    }

    void Session::changePuppet(entt::entity ent) {
        puppet = ent;
    }

    void Session::handleText(const std::string &text) {
        lastActivity = std::chrono::steady_clock::now();
        if(text == "--") {
            // clear the queue.
            inputQueue.clear();
            sendText("Your input queue has been cleared of all pending commands.\n");
            return;
        }
        inputQueue.push_back(text);
    }

    void Session::sendText(std::string_view text) {
        outText += text;
    }

    void Session::onNetworkDisconnected(uint64_t connId) {

        auto conn = clients.find(connId);
        if (conn != clients.end()) {
            removeConnection(conn->second);
        }
        if(clients.empty()) {
            onLinkDead();
        }
    }

    void Session::onLinkDead() {
        // Should the last remaining session be removed due to loss of a network connection,
        // we should declare the character link-dead and begin a process of removal from the world.
        // This may need to operate as some sort of timer, so that if the player reconnects within
        // the limit they can rejoin their Session, and to avoid potential abuse of any link-dead
        // mechanics.
    }

    void Session::addConnection(uint64_t connID) {
        auto found = kaizer::state::connections.find(connID);
        if (found != kaizer::state::connections.end()) {
            addConnection(found->second);
        }
    }

    void Session::addConnection(const std::shared_ptr<ClientConnection> &conn) {
        clients[conn->getConnID()] = conn;
        onAddConnection(conn);
        totalConnections++;
    }

    void Session::onAddConnection(const std::shared_ptr<ClientConnection>& conn) {
        // If we have only one connection then we need to launch a special hook.
        if(totalConnections == 0) {
            onFirstConnection();
        }
    }

    void Session::onFirstConnection() {
        start();
    }

    void Session::removeConnection(uint64_t connID) {
        auto found = clients.find(connID);
        if (found != clients.end()) {
            removeConnection(found->second);
        }
    }

    void Session::removeConnection(const std::shared_ptr<ClientConnection> &conn) {
        clients.erase(conn->getConnID());
        onRemoveConnection(conn);
    }

    void Session::removeAllConnections() {
        auto tempClients = clients;
        for (auto& [clientId, conn] : tempClients) {
            removeConnection(conn);
        }
    }

    void Session::onRemoveConnection(const std::shared_ptr<ClientConnection>& conn) {

    }

    void Session::onHeartbeat(double deltaTime) {
        if(inputQueue.empty()) {
            return;
        }
        auto& input = inputQueue.front();
        auto &cmd = registry.get_or_emplace<components::PendingCommand>(puppet);
        cmd.input = input;
        // Now we remove input from inputQueue.
        inputQueue.pop_front();

    }

    void Session::sendOutput(double deltaTime) {
         // This will later need to handle prompts.
        if (!outText.empty()) {
            for(auto &[id, c] : clients) {
                c->sendText(outText);
            }
            outText.clear();
        }
    }

    entt::entity Session::getCharacter() {
        return character;
    }

    entt::entity Session::getPuppet() {
        return puppet;
    }

    ObjectID Session::getAccount() {
        return account;
    }

    ObjectID Session::getID() {
        return id;
    }

    std::chrono::system_clock::time_point Session::getCreated() {
        return created;
    }

    std::chrono::steady_clock::time_point Session::getLastActivity() {
        return lastActivity;
    }

    int16_t Session::getAdminLevel() {
        SQLite::Statement q(*db, "SELECT adminLevel FROM accounts WHERE id = ?");
        q.bind(1, account);
        if(q.executeStep()) {
            return q.getColumn(0).getInt();
        }
        return 0;
    }

    std::shared_ptr<Session> defaultMakeSession(ObjectID id, ObjectID account, entt::entity character) {
        return std::make_shared<Session>(id, account, character);
    }
    std::function<std::shared_ptr<Session>(ObjectID, ObjectID, entt::entity)> makeSession(defaultMakeSession);

}