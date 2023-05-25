#pragma once
#include "kaizermud/ClientConnection.h"

namespace kaizermud::game {
    // The Session class represents a specific session of play while a Character is online.
    // A Session is created when a Character logs in, and is destroyed when the Character logs out.
    // It acts as a middle-man and also a repository for data relevant to just this session, and
    // as such it also allows multiple connections to be linked to the same Character simultaneously,
    // all seeing the same thing and accepting input from all of them in first-come-first-served order.
    // This is useful for allowing a player to be logged in from multiple devices at once, or from
    // multiple locations.
    class Session {
    public:
        explicit Session(const std::shared_ptr<Object> obj);
        virtual ~Session() = default;

        // Send a message to the linked connections.
        virtual void send(const Message &msg);
        virtual void atObjectDeleted(const std::shared_ptr<Object>& obj);
    protected:
        // The character this Session is linked to.
        std::weak_ptr<Object> character;
        // The object this session is currently controlling. That's USUALLY going to be the Character,
        // but it might not be. For example, if the Character is in a vehicle, the vehicle might be
        // the puppet.
        std::weak_ptr<Object> puppet;
        // This is a map of all the connections that are currently linked to this session.
        std::unordered_map<uint64_t, std::weak_ptr<kaizermud::net::ClientConnection>> clients;

    };
}