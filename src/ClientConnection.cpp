#include "kaizermud/ClientConnection.h"
#include <set>

namespace kaizermud::net {

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

    }

    void ClientConnection::onHeartbeat(boost::asio::steady_timer::duration deltaTime) {

    }

    void ClientConnection::sendText(const std::string &text) {
        boost::json::object jobj;
        jobj["kind"] = "session_text";
        jobj["id"] = this->conn_id;
        jobj["text"] = text;

        lm.linkChan.try_send(boost::system::error_code{}, static_cast<boost::json::value>(jobj));
    }

    const ProtocolCapabilities& ClientConnection::getCapabilities() const {
        return capabilities;
    }

    uint64_t ClientConnection::getConnID() const {
        return connID;
    }

    std::optional<ObjectID> ClientConnection::getAccountID() const {
        return accountID;
    }

}