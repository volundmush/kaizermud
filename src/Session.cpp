#include "kaizermud/Session.h"
#include "kaizermud/game.h"

namespace kaizer {

    Session::Session(ObjectID id, entt::entity ent) {
        this->id = id;
        character = ent;
        puppet = ent;
        created = std::chrono::system_clock::now();
    }

    void Session::start() {

    }

    void Session::end() {

    }

    void Session::send(const Message &msg) {

    }

    void Session::atObjectDeleted(entt::entity ent) {

    }

    void Session::changePuppet(entt::entity ent) {
        puppet = ent;
    }

    void Session::handleText(const std::string &text) {

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

    void Session::onHeartbeat(double deltaTime) {
        lastActivity = std::chrono::steady_clock::now();
    }

}