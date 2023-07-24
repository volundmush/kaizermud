#pragma once

#include "kaizermud/base.h"
#include <ctime>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <memory>
#include <list>
#include <boost/json.hpp>
#include <optional>
#include <boost/system.hpp>
#include <boost/asio/error.hpp>
#include "kaizermud/thermite.h"

namespace kaizer {

    enum class Protocol : uint8_t {
        Telnet = 0,
        WebSocket = 1
    };

    struct ProtocolCapabilities {
        Protocol protocol{Protocol::Telnet};
        bool encryption = false;
        std::string clientName = "UNKNOWN", clientVersion = "UNKNOWN";
        std::string hostAddress = "UNKNOWN";
        int16_t hostPort{0};
        std::vector<std::string> hostNames{};
        std::string encoding;
        bool utf8 = false;
        ColorType colorType = ColorType::NoColor;
        int width = 80, height = 52;
        bool gmcp = false, msdp = false, mssp = false, mxp = false;
        bool mccp2 = false, mccp2_active = false, mccp3 = false, mccp3_active = false;
        bool ttype = false, naws = true, sga = true, linemode = false;
        bool force_endline = false, oob = false, tls = false;
        bool screen_reader = false, mouse_tracking = false, vt100 = false;
        bool osc_color_palette = false, proxy = false, mnes = false;
        void deserialize(const boost::json::value &j);
    };

    class ClientConnection : public std::enable_shared_from_this<ClientConnection> {
        // They need this PURELY for access to that darned spsc_channel.
        friend class Link;
        friend class LinkManager;
    public:
        ClientConnection(LinkManager &lm, uint64_t conn_id, mpmc_channel<boost::json::value> chan)
        : lm(lm), connID(conn_id), fromLink(std::move(chan)) {}
        void sendText(const std::string &messg);
        //void sendMSSP(const std::vector<std::tuple<std::string, std::string>> &data);
        //void sendGMCP(const std::string &txt);
        virtual void onWelcome();
        virtual void onHeartbeat(double deltaTime);
        virtual void onNetworkDisconnected();
        [[nodiscard]] const ProtocolCapabilities& getCapabilities() const;
        [[nodiscard]] uint64_t getConnID() const;
        [[nodiscard]] ObjectID getAccount() const;
        virtual void handleText(const std::string& text);
        virtual void handleConnectCommand(const std::string& text);
        virtual void handleBadMatch(const std::string& text, std::unordered_map<std::string, std::string>& matches);


        // This is a wrapper around the unbound createAccount which will check for abuse.
        virtual OpResult<ObjectID> createAccount(std::string_view userName, std::string_view password);

        virtual void onCreateAccount(std::string_view userName, std::string_view password, ObjectID id);
        virtual OpResult<> handleLogin(const std::string &userName, const std::string &password);
        virtual void loginToAccount(ObjectID id);
        virtual void onLogin();
        virtual void handleLoginCommand(const std::string& text);
        virtual void createOrJoinSession(ObjectID id);

        virtual void displayAccountMenu();

    protected:
        uint64_t connID;

        // A session is a play instance where some game entity is in use
        // as a player character. Sessions multiplex possibly many connections.
        // If this pointer is null, we don't yet have a session.
        std::shared_ptr<Session> session;

        // A Connection might or might not be logged in.
        int64_t account{-1};
        // Some time structs to handle when we received connections.
        // These probably need some updating on this and Thermite side...
        LinkManager &lm;
        std::chrono::system_clock::time_point connected{};
        std::chrono::steady_clock::time_point connectedSteady{}, lastActivity{}, lastMsg{};


        // This is embedded for ease of segmentation but this struct isn't
        // actually used anywhere else.
        ProtocolCapabilities capabilities;

        // Later we'll need to handle more than just text commands. But this should handle
        std::list<std::string> pending_commands;

        mpmc_channel<boost::json::value> fromLink;

    };

    extern std::unordered_map<std::string, std::set<ObjectID>> accountsCreatedRecently;

    // Validator functions for checking whether a new/renamed account/character string is valid.
    // For generating new accounts/characters, set ent = entt::null. For renames, enter the target's entt::entity.
    extern std::vector<std::function<OpResult<>(std::string_view, ObjectID)>> accountUsernameValidators, playerCharacterNameValidators;
    OpResult<> validateAccountUsername(std::string_view username, ObjectID id);
    OpResult<> validatePlayerCharacterName(std::string_view name, ObjectID id);

    // The below will directly create an account, IF an account can be accounted.
    // For proper usage, you'll want to wrap this in additional checks, like whether
    // the same connection should be allowed to create 1,000 accounts in a minute.
    // Hint: it shouldn't be allowed.
    // the result type is entt::entity, so entt::null will be returned on failure.
    OpResult<ObjectID> createAccount(std::string_view username, std::string_view password);
}