#pragma once
#include "kaizermud/base.h"
#include <boost/asio.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast.hpp>
#include <boost/json.hpp>
#include <string>


namespace kaizermud::thermite {

    // This class manages a repeating connector to Thermite. It is launched via co_spawn
    // using run() as its entry point, and it will connect to Thermite using the provided
    // endpoint. If the connection is established then it will create a Link object
    // and pass control to that Link object's run() via co_await. If the connection drops,
    // it will attempt to reconnect endlessly until it re-establishes connection.
    class LinkManager {
    public:
        spsc_channel<boost::json::value> linkChan;
        explicit LinkManager(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint ep);

        boost::asio::awaitable<void> run();
        void stop();

    protected:
        boost::asio::ip::tcp::endpoint endpoint;
        bool is_stopped;
    };


    // this class is a link to the Thermite server, which accepts a WebSocket connection.
    // It is responsible for communicating with Thermite to handle incoming client events and
    // to relay game events to the client.
    class Link {
    public:
        explicit Link(LinkManager& lm, boost::beast::websocket::stream<boost::beast::tcp_stream>& ws);

        boost::asio::awaitable<void> run();
        void stop();

    protected:
        LinkManager& linkManager;
        boost::asio::awaitable<void> runReader();
        boost::asio::awaitable<void> runWriter();
        boost::asio::awaitable<void> createUpdateClient(boost::json::object &v);
        boost::beast::websocket::stream<boost::beast::tcp_stream>& conn;
        bool is_stopped;
    };

}