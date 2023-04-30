#include "kaizermud/thermite.h"
#include "kaizermud/utils.h"
#include "kaizermud/net.h"
#include <iostream>
#include <utility>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>

namespace kaizermud::thermite {
    using namespace boost::asio::experimental::awaitable_operators;
    LinkManager::LinkManager(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint ep)
            : channel(ioc, 50), endpoint(std::move(ep)), is_stopped(false) {}

    boost::asio::awaitable<void> LinkManager::run() {
        bool do_standoff = false;
        boost::asio::steady_timer standoff(co_await boost::asio::this_coro::executor);
        while (!is_stopped) {
            if(do_standoff) {
                standoff.expires_after(std::chrono::seconds(5));
                co_await standoff.async_wait(boost::asio::use_awaitable);
                do_standoff = false;
            }
            try {
                // Create a new TCP socket
                boost::beast::websocket::stream<boost::beast::tcp_stream> ws(co_await boost::asio::this_coro::executor);

                // Connect to the endpoint
                co_await boost::beast::get_lowest_layer(ws).async_connect(endpoint, boost::asio::use_awaitable);
                // Initialize a WebSocket using the connected socket

                // Perform the WebSocket handshake
                co_await ws.async_handshake(endpoint.address().to_string() + ":" + std::to_string(endpoint.port()), "/", boost::asio::use_awaitable);

                // Construct a Link using the WebSocket

                Link link(*this, ws);

                // Run the Link
                co_await link.run();

            } catch (const boost::system::system_error& error) {
                // If there was an error, handle it (e.g., log the error message)
                std::cerr << "Error in LinkManager::run(): " << error.what() << std::endl;

                // You might want to add a delay before attempting to reconnect, e.g.,
                do_standoff = true;
            }
        }
        co_return;
    }

    void LinkManager::stop() {
        is_stopped = true;
    }

    Link::Link(LinkManager& lm, boost::beast::websocket::stream<boost::beast::tcp_stream>& ws)
            : linkManager(lm), conn(ws), is_stopped(false) {}

    boost::asio::awaitable<void> Link::run() {
        try {
            co_await (runReader() || runWriter());
        } catch (const boost::system::system_error& e) {
            // Handle exceptions here if necessary
        }
        co_return;
    }

    void Link::stop() {
        is_stopped = true;
    }

    boost::asio::awaitable<void> Link::createUpdateClient(boost::json::object &v) {
        uint64_t id = v["id"].as_uint64();
        std::string addr = v["addr"].as_string().c_str();
        boost::json::object capabilities = v["capabilities"].as_object();

        // Do something with id, addr, and capabilities
        auto it = kaizermud::net::connections.find(id);
        if (it == kaizermud::net::connections.end()) {
            // Create a new ClientConnection
            Channel clientChan(co_await boost::asio::this_coro::executor, 50);
            auto cc = std::make_shared<kaizermud::net::ClientConnection>(id, std::move(clientChan));
            kaizermud::net::connections[id] = cc;
        } else {
            // Update the existing ClientConnection
            auto& client_connection = it->second;
            client_connection->capabilities.deserialize(capabilities);
        }
        co_return;
    }

    boost::asio::awaitable<void> Link::runReader() {
        try {
            while (true) {
                // Read a message from the WebSocket
                boost::beast::flat_buffer buffer;
                co_await conn.async_read(buffer, boost::asio::use_awaitable);

                // Deserialize the JSON string
                auto ws_str = boost::beast::buffers_to_string(buffer.data());
                auto json_data = boost::json::parse(ws_str);
                boost::json::object j = json_data.as_object();

                // Access the "kind" field in the JSON object
                auto kind = j["kind"].as_string();

                // Implement your routing logic here

                if (kind == "client_list") {
                    // This message is sent by Thermite when the game establishes a fresh connection with it.
                    // It should be the first thing a Link sees.

                    // Get the "data" object
                    boost::json::object data = j["data"].as_object();

                    // Iterate over the contents of the "data" object
                    for (const auto& entry : data) {
                        boost::json::object v = entry.value().as_object();
                        co_await createUpdateClient(v);
                    }

                } else if (kind == "client_ready") {
                    // This message is sent by Thermite when a new client has connected.
                    boost::json::object data = j["protocol"].as_object();
                    co_await createUpdateClient(data);
                } else {
                    // Extract the "id" field from the JSON object
                    uint64_t id = j["id"].as_uint64();

                    // Look up the specific ClientConnection in the std::map
                    auto it = kaizermud::net::connections.find(id);
                    if (it != kaizermud::net::connections.end()) {
                        // Found the client connection
                        auto& client_connection = it->second;

                        if (kind == "client_capabilities") {
                            boost::json::object capabilities = j["capabilities"].as_object();
                            client_connection->capabilities.deserialize(capabilities);
                        } else if (kind == "client_line") {
                            co_await client_connection->fromLink.async_send(boost::system::error_code{}, j, boost::asio::use_awaitable);
                        } else if (kind == "client_gmcp") {
                            co_await client_connection->fromLink.async_send(boost::system::error_code{}, j, boost::asio::use_awaitable);
                        } else if (kind == "client_disconnected") {
                            // worrying about this later...
                        }
                    } else {
                        // Handle the case when the client is not found in the map
                        // ...
                    }
                }
            }
        } catch (const boost::system::system_error& e) {
            // Handle exceptions (e.g., WebSocket close or error)
        }
        co_return;
    }

    boost::asio::awaitable<void> Link::runWriter() {
        while (true) {
            try {
                // Receive a message from the channel asynchronously
                boost::json::value message = co_await linkManager.channel.async_receive(boost::asio::use_awaitable);

                try {
                    // Serialize the JSON message to text
                    std::string serialized_msg = boost::json::serialize(message);

                    // Send the message across the WebSocket
                    co_await conn.async_write(boost::asio::buffer(serialized_msg), boost::asio::use_awaitable);
                } catch (const boost::system::system_error& e) {
                    // Handle exceptions (e.g., WebSocket close or error) when sending the message
                }
            } catch (const boost::system::system_error& e) {
                // Handle exceptions (e.g., error receiving the message from the channel)
            }
        }
        co_return;
    }

}