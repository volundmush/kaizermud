#include "kaizermud/net.h"
#include <set>

namespace kaizermud::net {

    ClientConnection::ClientConnection(uint64_t conn_id, Channel chan) : conn_id(conn_id), fromLink(std::move(chan)) {}

    void ProtocolCapabilities::deserialize(const boost::json::value &j) {
        using namespace boost::json;
        object obj = j.as_object();

        std::string protocol_str = obj["protocol"].as_string().c_str();
        if (protocol_str == "Telnet") {
            protocol = Telnet;
        } else if (protocol_str == "WebSocket") {
            protocol = WebSocket;
        }

        encryption = obj["encryption"].as_bool();
        clientName = obj["client_name"].as_string().c_str();
        clientVersion = obj["client_version"].as_string().c_str();
        encoding = obj["encoding"].as_string().c_str();
        utf8 = obj["utf8"].as_bool();

        std::string color_str = obj["color"].as_string().c_str();
        if (color_str == "NoColor") {
            colorType = NoColor;
        } else if (color_str == "Standard") {
            colorType = Standard;
        } else if (color_str == "Xterm256") {
            colorType = Xterm256;
        } else if (color_str == "TrueColor") {
            colorType = TrueColor;
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

}