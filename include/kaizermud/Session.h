#pragma once
#include "kaizermud/ClientConnection.h"
#include "Message.h"

namespace kaizer {
    // The Session class represents a specific session of play while a Character is online.
    // A Session is created when a Character logs in, and is destroyed when the Character logs out.
    // It acts as a middle-man and also a repository for data relevant to just this session, and
    // as such it also allows multiple connections to be linked to the same Character simultaneously,
    // all seeing the same thing and accepting input from all of them in first-come-first-served order.
    // This is useful for allowing a player to be logged in from multiple devices at once, or from
    // multiple locations.
    class Session {
    public:
        Session(ObjectID id, entt::entity account, entt::entity character);
        virtual ~Session() = default;

        virtual void start();
        virtual void end();

        // Send a message to the linked connections.
        virtual void send(const Message &msg);
        virtual void atObjectDeleted(entt::entity ent);

        // Add a connection to this session.
        virtual void addConnection(uint64_t connID);
        virtual void addConnection(const std::shared_ptr<ClientConnection>& conn);
        virtual void onAddConnection(const std::shared_ptr<ClientConnection>& conn);
        // Remove a connection from this session.
        virtual void removeConnection(uint64_t connID);
        virtual void removeConnection(const std::shared_ptr<ClientConnection>& conn);
        virtual void onRemoveConnection(const std::shared_ptr<ClientConnection>& conn);
        // Remove all connections from this session.
        virtual void removeAllConnections();

        virtual void onFirstConnection();

        virtual void onLinkDead();

        virtual void onNetworkDisconnected(uint64_t connId);

        virtual void changePuppet(entt::entity ent);

        virtual void handleText(const std::string& text);

        virtual void sendText(std::string_view text);

        virtual void onHeartbeat(double deltaTime);

        virtual void sendOutput(double deltaTime);

        virtual entt::entity getCharacter();
        virtual entt::entity getPuppet();
        virtual entt::entity getAccount();

        virtual int16_t getAdminLevel();

        std::chrono::steady_clock::time_point getLastActivity();
        std::chrono::system_clock::time_point getCreated();

        ObjectID getID();


    protected:
        // The ID of the character is the ID of the session.
        ObjectID id;
        // The character this Session is linked to.
        entt::entity character;
        // The object this session is currently controlling. That's USUALLY going to be the Character,
        // but it might not be. For example, if the Character is in a vehicle, the vehicle might be
        // the puppet.
        entt::entity puppet;
        // The account we are using the permissions of.
        entt::entity account;

        // This is a map of all the connections that are currently linked to this session.
        std::unordered_map<uint64_t, std::shared_ptr<ClientConnection>> clients;

        std::chrono::steady_clock::time_point lastActivity{};
        std::chrono::system_clock::time_point created{};
        std::list<std::string> inputQueue;
        std::string outText;
        int totalConnections{0};
    };

    extern std::function<std::shared_ptr<Session>(ObjectID, entt::entity, entt::entity)> makeSession;
    std::shared_ptr<Session> defaultMakeSession(ObjectID id, entt::entity account, entt::entity character);

}