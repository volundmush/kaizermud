#include "kaizermud/ClientConnection.h"
#include "kaizermud/Commands.h"
#include <set>
#include "kaizermud/utils.h"
#include "kaizermud/Components.h"
#include "fmt/format.h"
#include "boost/algorithm/string.hpp"
#include "spdlog/spdlog.h"
#include "kaizermud/Color.h"
#include "kaizermud/game.h"
#include "kaizermud/Database.h"

namespace kaizer {

    void ProtocolCapabilities::deserialize(const boost::json::value &j) {
        using namespace boost::json;
        object obj = j.as_object();

        std::string protocol_str = obj["protocol"].as_string().c_str();
        if (protocol_str == "Telnet") {
            protocol = Protocol::Telnet;
        } else if (protocol_str == "WebSocket") {
            protocol = Protocol::WebSocket;
        }

        encryption = obj["encryption"].as_bool();
        clientName = obj["client_name"].as_string().c_str();
        clientVersion = obj["client_version"].as_string().c_str();
        hostAddress = obj["host_address"].as_string().c_str();
        hostPort = obj["host_port"].as_int64();
        for(auto &hn : obj["host_names"].as_array()) {
            hostNames.emplace_back(hn.as_string().c_str());
        }
        encoding = obj["encoding"].as_string().c_str();
        utf8 = obj["utf8"].as_bool();

        std::string color_str = obj["color"].as_string().c_str();
        if (color_str == "NoColor") {
            colorType = ColorType::NoColor;
        } else if (color_str == "Standard") {
            colorType = ColorType::Standard;
        } else if (color_str == "Xterm256") {
            colorType = ColorType::Xterm256;
        } else if (color_str == "TrueColor") {
            colorType = ColorType::TrueColor;
        }

        width = obj["width"].as_int64();
        height = obj["height"].as_int64();
        gmcp = obj["gmcp"].as_bool();
        msdp = obj["msdp"].as_bool();
        mssp = obj["mssp"].as_bool();
        mxp = obj["mxp"].as_bool();
        mccp2 = obj["mccp2"].as_bool();
        mccp3 = obj["mccp3"].as_bool();
        ttype = obj["ttype"].as_bool();
        naws = obj["naws"].as_bool();
        sga = obj["sga"].as_bool();
        linemode = obj["linemode"].as_bool();
        force_endline = obj["force_endline"].as_bool();
        oob = obj["oob"].as_bool();
        tls = obj["tls"].as_bool();
        screen_reader = obj["screen_reader"].as_bool();
        mouse_tracking = obj["mouse_tracking"].as_bool();
        vt100 = obj["vt100"].as_bool();
        osc_color_palette = obj["osc_color_palette"].as_bool();
        proxy = obj["proxy"].as_bool();
        mnes = obj["mnes"].as_bool();
    }

    void ClientConnection::onWelcome() {
        sendText("Welcome to KaizerMUD!\r\n");
    }

    void ClientConnection::onNetworkDisconnected() {
        if(session) {
            session->removeConnection(connID);
        }
    }

    void ClientConnection::handleBadMatch(const std::string& text, std::unordered_map<std::string, std::string>& matches) {
        sendText("Sorry, that's not a command.\r\n");
    }

    void ClientConnection::handleConnectCommand(const std::string& text) {
        auto match_map = parseCommand(text);

        if(match_map.empty()) {
            handleBadMatch(text, match_map);
            return;
        }
        auto self = shared_from_this();
        auto ckey = match_map["cmd"];
        for(auto &[key, cmd] : expandedConnectCommandRegistry) {
            if(!cmd->isAvailable(self))
                continue;
            if(boost::iequals(ckey, key)) {
                auto [can, err] = cmd->canExecute(self, match_map);
                if(!can) {
                    sendText(fmt::format("Sorry, you can't do that: {}\r\n", err.value()));
                    return;
                }
                cmd->execute(self, match_map);
                return;
            }
        }
        handleBadMatch(text, match_map);
    }

    void ClientConnection::handleLoginCommand(const std::string& text) {
        auto match_map = parseCommand(text);
        if(match_map.empty()) {
            handleBadMatch(text, match_map);
            return;
        }
        auto self = shared_from_this();
        auto ckey = match_map["cmd"];
        for(auto &[key, cmd] : expandedLoginCommandRegistry) {
            if(!cmd->isAvailable(self))
                continue;
            if(boost::iequals(ckey, key)) {
                auto [can, err] = cmd->canExecute(self, match_map);
                if(!can) {
                    sendText(fmt::format("Sorry, you can't do that: {}\r\n", err.value()));
                    return;
                }
                cmd->execute(self, match_map);
                return;
            }
        }
        handleBadMatch(text, match_map);
    }

    void ClientConnection::handleText(const std::string &str) {
        // the IDLE command should be intercepted here.
        // It's a convention which allows clients behind wonky NAS
        // to keep their connections alive.
        if(str == "IDLE") {
            return;
        }

        // If we have a connected session, then we relay this to it. Else,
        // we treat it as a login Command.

        if(session)
            session->handleText(str);
        else if(account != -1)
            handleLoginCommand(str);
        else
            handleConnectCommand(str);
    }

    void ClientConnection::onHeartbeat(double deltaTime) {
        // Every time the heartbeat runs, we want to pull everything out of fromLink
        // first of all.

        // We need to do this in a loop, because we may have multiple messages in the
        // channel.

        std::error_code ec;
        boost::json::value value;
        while (fromLink.ready()) {
            if(!fromLink.try_receive([&](std::error_code ec2, boost::json::value value2) {
                ec = ec2;
                value = value2;
            })) {
                break;
            }
            if (ec) {
                // TODO: Handle any errors..
            } else {
                // If we got a json value, it should look like this.
                // {"kind": "client_data", "id": 123, "data": [{"cmd": "text", "args": ["hello world!"], "kwargs": {}}]}
                // We're only interested in iterating over the contents of the "data" array.

                boost::json::object jobj = value.as_object();
                boost::json::array jarr = jobj["data"].as_array();

                // Now we must for-each over the contents of jarr, extract the cmd, args, and kwargs data, and call
                // the appropriate handle routine.

                for (auto &jval : jarr) {
                    boost::json::object jobj2 = jval.as_object();
                    std::string cmd = jobj2["cmd"].as_string().c_str();
                    boost::json::array jargs = jobj2["args"].as_array();
                    boost::json::object jkwargs = jobj2["kwargs"].as_object();

                    if (cmd == "text") {
                        // Treat each argument in jargs as a separate string.
                        // We need to for-each jargs and handleText(each).

                        for (auto &jarg : jargs) {
                            std::string text = jarg.as_string().c_str();
                            handleText(text);
                        }

                    } else if (cmd == "mssp") {
                        // We should never be receiving mssp like this.
                    } else {
                        // Anything that's not text or mssp is generic GMCP data.
                        // TODO: handle GMCP.
                    }
                }
                lastActivity = std::chrono::steady_clock::now();

            }
        }

    }

    void ClientConnection::sendText(const std::string &text) {
        boost::json::object jobj;
        jobj["kind"] = "client_data";
        jobj["id"] = this->connID;

        boost::json::array jarr;
        boost::json::object jobj2;
        jobj2["cmd"] = "text";
        jobj2["args"] = {renderAnsi(text, this->capabilities.colorType)};
        jobj2["kwargs"] = {};
        jarr.push_back(jobj2);
        jobj["data"] = jarr;

        boost::json::value v = jobj;

        lm.linkChan.try_send(boost::system::error_code{}, v);
    }

    const ProtocolCapabilities& ClientConnection::getCapabilities() const {
        return capabilities;
    }

    uint64_t ClientConnection::getConnID() const {
        return connID;
    }

    ObjectID ClientConnection::getAccount() const {
        return account;
    }

    OpResult<ObjectID> ClientConnection::createAccount(std::string_view userName, std::string_view password) {
        // First, get the IP without the port from this->capabilities.
        // Then, get the set of accounts created recently for that IP.

        auto &created = accountsCreatedRecently[this->capabilities.hostAddress];
        // If the set is empty, then we can create an account.
        if (!created.empty())
            return {-1, "You have created too many accounts recently. Please try again later."};

        auto [id, err] = kaizer::createAccount(userName, password);
        if(err.has_value())
            return {-1, err};

        created.insert(id);

        onCreateAccount(userName, password, id);
        return {id, std::nullopt};
    }

    void ClientConnection::onCreateAccount(std::string_view userName, std::string_view password, ObjectID id) {
        sendText(fmt::format("Account created successfully. Welcome, {}!\r\n", userName));
        sendText(fmt::format("Please keep your password safe and secure. If you forget it, contact staff.\r\n"));
        loginToAccount(id);
    }

    void ClientConnection::loginToAccount(ObjectID id) {
        account = id;
        onLogin();
    }

    void ClientConnection::onLogin() {
        SQLite::Statement q(*db, "UPDATE accounts SET lastLogin = datetime('now') WHERE id = ?");
        q.bind(1, account);
        q.exec();

        SQLite::Statement q2(*db, "SELECT username FROM accounts WHERE id = ?");
        q2.bind(1, account);
        std::string name;
        while(q2.executeStep()) {
            name = q2.getColumn(0).getString();
        }

        sendText(fmt::format("Welcome back, {}!\r\n", name));
        displayAccountMenu();
    }

    void ClientConnection::displayAccountMenu() {
        std::string username, email;
        int level;
        SQLite::Statement q(*db, "SELECT username, email, adminLevel FROM accounts WHERE id = ?");
        q.bind(1, account);
        while(q.executeStep()) {
            username = q.getColumn(0).getString();
            email = q.getColumn(1).getString();
            level = q.getColumn(2).getInt();
        }
        sendText("                 @RAccount Menu@n\n");
        sendText("=============================================\n");
        sendText(fmt::format("|@g{:<14}@n:  {:<27}|\n", "Username", username));
        if(!email.empty())
            sendText(fmt::format("|@g{:<14}@n:  {:<27}|\n", "Email Address", email));
        if(level > 0)
            sendText(fmt::format("|@g{:<14}@n:  {:<27}|\n", "Admin Level", level));
        sendText("=============================================\n\n");

        std::vector<ObjectID> characters;
        SQLite::Statement q2(*db, "SELECT character,lastLogin,lastLogout,totalPlayTime FROM playerCharacters WHERE account = ?");
        q2.bind(1, account);
        while(q2.executeStep()) {
            auto character = q2.getColumn(0).getInt64();
            auto lastLogin = q2.getColumn(1).getString();
            auto lastLogout = q2.getColumn(2).getString();
            auto totalPlayTime = q2.getColumn(3).getString();

            characters.push_back(character);
        }

        if(!characters.empty()) {
            sendText("[@y----@YAvailable Characters@y----@n]\n");
            for(auto c : characters) {
                auto ct = getType(c);
                sendText(ct->getName(c) + "\n");
            }
        }

        sendText("\n[@y----@YCommands@y----@n]\n");
        sendText("@Wcreate <character>@n - Create a new character.\n");
        sendText("@Wplay <character>@n - Log in to a character.\n");
        sendText("@Wdelete <character>@n - Delete a character.\n");
        sendText("@Wpassword <old password>=<new password>@n - Change your password.\n");
        sendText("@Wemail <email address>@n - Change your email address.\n");
        sendText("@Wquit@n - Quit the game.\n");
    }

    OpResult<> ClientConnection::handleLogin(const std::string &userName, const std::string &password) {
        SQLite::Statement q(*db, "SELECT id,password FROM accounts WHERE username = ?");
        q.bind(1, userName);
        while(q.executeStep()) {
            auto id = q.getColumn(0).getInt64();
            auto pass = q.getColumn(1).getString();
            auto [check, err] = kaizer::checkPassword(pass, password);
            if(!check) {
                return {false, err};
            }
            loginToAccount(id);
            return {true, std::nullopt};
        }
        return {false, "No such account.\n"};
    }

    void ClientConnection::createOrJoinSession(ObjectID id) {
        auto ent = getEntity(id);
        auto sessholder = registry.try_get<components::SessionHolder>(ent);

        if(sessholder) {
            // the character already has a session, so we need to join this ClientConnection
            // to it.
            session = sessholder->data;
        } else {
            // The character has no session, so we need to create one and join the clientConnection to it.
            session = makeSession(id, account, ent);
            state::sessions[id] = session;
            auto &sholder = registry.emplace<components::SessionHolder>(ent);
            sholder.data = session;
            sholder.sessionMode = 1;
        }
        session->addConnection(shared_from_this());
    }


    std::unordered_map<std::string, std::set<ObjectID>> accountsCreatedRecently;

    // Validator functions for checking whether a new/renamed account/character string is valid.
    // For generating new accounts/characters, set ent = entt::null. For renames, enter the target's entt::entity.
    std::vector<std::function<OpResult<>(std::string_view, ObjectID)>> accountUsernameValidators, playerCharacterNameValidators;

    OpResult<> validateAccountUsername(std::string_view username, ObjectID id) {
        for (auto &validator : accountUsernameValidators) {
            auto [res, err] = validator(username, id);
            if (!res) return {res, err};
        }
        return {true, std::nullopt};
    }

    OpResult<> validatePlayerCharacterName(std::string_view name, ObjectID id) {
        for (auto &validator : playerCharacterNameValidators) {
            auto [res, err] = validator(name, id);
            if (!res) return {res, err};
        }
        return {true, std::nullopt};
    }

    OpResult<ObjectID> createAccount(std::string_view username, std::string_view password) {
        auto [res, err] = validateAccountUsername(username, -1);
        if (!res) return {-1, err};
        auto [res2, err2] = hashPassword(password);
        if (!res2) return {-1, err2};

        SQLite::Statement q(*db, "INSERT INTO accounts (username, password) VALUES (?, ?)");
        q.bind(1, std::string(username));
        q.bind(2, err2.value());
        q.exec();
        auto id = db->getLastInsertRowid();
        return {id, std::nullopt};
    }
}