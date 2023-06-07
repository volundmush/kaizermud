#include "kaizermud/thermite.h"
#include "kaizermud/ClientConnection.h"
#include "kaizermud/game.h"
#include <iostream>
#include <utility>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include "spdlog/spdlog.h"

namespace kaizer {
    using namespace boost::asio::experimental::awaitable_operators;
    LinkManager::LinkManager(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint ep)
            : linkChan(ioc, 50), endpoint(std::move(ep)), is_stopped(false) {}

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
                spdlog::info("LinkManager: Connecting to {}:{}...", endpoint.address().to_string(), endpoint.port());
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
                spdlog::info("LinkManager: Link established! Running Link...");
                co_await link.run();

            } catch (const boost::system::system_error& error) {
                // If there was an error, handle it (e.g., log the error message)
                spdlog::error("Error in LinkManager::run(): {}", error.what());

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
            spdlog::error("Error in Link::run(): {}", e.what());
            // Handle exceptions here if necessary
        } catch(...) {
            spdlog::error("Unknown error in Link::run()");
        }
        co_return;
    }

    void Link::stop() {
        is_stopped = true;
    }

    boost::asio::awaitable<void> Link::createUpdateClient(boost::json::object &v) {
        uint64_t id = static_cast<uint64_t>(v["id"].as_int64());
        std::string addr = v["addr"].as_string().c_str();
        boost::json::object capabilities = v["capabilities"].as_object();

        // Do something with id, addr, and capabilities
        auto it = state::connections.find(id);
        if (it == state::connections.end()) {
            // Create a new ClientConnection
            mpmc_channel<boost::json::value> clientChan(co_await boost::asio::this_coro::executor, 50);
            auto cc = makeConnection(linkManager, id, std::move(clientChan));
            state::connections[id] = cc;
            state::pending_connections.insert(id);
            cc->capabilities.deserialize(capabilities);
        } else {
            // Update the existing ClientConnection
            auto& client_connection = it->second;
            client_connection->capabilities.deserialize(capabilities);
        }
        co_return;
    }

    boost::asio::awaitable<void> Link::runReader() {
        while (true) {
            try {
                // Read a message from the WebSocket
                boost::beast::flat_buffer buffer;
                co_await conn.async_read(buffer, boost::asio::use_awaitable);

                // Deserialize the JSON string
                auto ws_str = boost::beast::buffers_to_string(buffer.data());
                //std::cout << "Received: " << ws_str << std::endl;
                auto json_data = boost::json::parse(ws_str);
                boost::json::object j = json_data.as_object();

                // Access the "kind" field in the JSON object
                std::string kind = j["kind"].as_string().c_str();

                // Implement your routing logic here

                if (kind == "client_list") {
                    // This message is sent by Thermite when the game establishes a fresh connection with it.
                    // It should be the first thing a Link sees.
                    spdlog::info("Link: Received client_list message");
                    // Get the "data" object
                    boost::json::object data = j["data"].as_object();

                    // Iterate over the contents of the "data" object
                    for (const auto &entry : data) {
                        spdlog::debug("Link: Processing client_list entry: {}", boost::json::serialize(entry.value()));
                        boost::json::object v = entry.value().as_object();
                        co_await createUpdateClient(v);
                    }

                } else if (kind == "client_ready") {
                    // This message is sent by Thermite when a new client has connected.
                    boost::json::object data = j["protocol"].as_object();
                    spdlog::debug("Link: Received client_ready message: {}", boost::json::serialize(data));
                    co_await createUpdateClient(data);

                } else {
                    // Extract the "id" field from the JSON object
                    uint64_t id = j["id"].as_int64();

                    // Look up the specific ClientConnection in the std::map
                    auto it = state::connections.find(id);
                    if (it != state::connections.end()) {
                        // Found the client connection
                        auto &client_connection = it->second;

                        if (kind == "client_capabilities") {
                            boost::json::object capabilities = j["capabilities"].as_object();
                            spdlog::debug("Link: Received client_capabilities message: {}", boost::json::serialize(capabilities));
                            client_connection->capabilities.deserialize(capabilities);

                        } else if (kind == "client_data") {
                            try {
                                co_await client_connection->fromLink.async_send(boost::system::error_code{}, j, boost::asio::use_awaitable);
                            } catch (const boost::system::system_error &e) {
                                // Handle exceptions (e.g., WebSocket close or error)
                            }

                        } else if (kind == "client_disconnected") {
                            spdlog::info("Link: Received client_disconnected message");
                            state::disconnected_connections.insert(id);
                        }
                    } else {
                        spdlog::info("Link: Received message for unknown client: {}", id);
                        // Handle the case when the client is not found in the map
                        // ...
                    }
                }
            } catch (const boost::system::system_error &e) {
                spdlog::error("Link RunReader flopped at: {}", e.what());
                // Handle exceptions (e.g., WebSocket close or error)
            } catch (...) {
                spdlog::error("Unknown error in Link RunReader");
            }
        }
        co_return;
    }

    boost::asio::awaitable<void> Link::runWriter() {
        while (true) {
            try {
                // Receive a message from the channel asynchronously
                boost::json::value message = co_await linkManager.linkChan.async_receive(boost::asio::use_awaitable);

                try {
                    // Serialize the JSON message to text
                    std::string serialized_msg = boost::json::serialize(message);
                    spdlog::info("Link runWriter(): Sending message: {}", serialized_msg);

                    // Send the message across the WebSocket
                    co_await conn.async_write(boost::asio::buffer(serialized_msg), boost::asio::use_awaitable);
                } catch (const boost::system::system_error& e) {
                    spdlog::error("Link runWriter flopped 1: {}", e.what());
                    // Handle exceptions (e.g., WebSocket close or error) when sending the message
                } catch (...) {
                    spdlog::error("Unknown error in Link runWriter 1");
                }
            } catch (const boost::system::system_error& e) {
                spdlog::error("Link runWriter flopped 2: {}", e.what());
                // Handle exceptions (e.g., error receiving the message from the channel)
            } catch (...) {
                spdlog::error("Unknown error in Link runWriter 2");
            }
        }
        co_return;
    }

    std::shared_ptr<ClientConnection> defaultMakeConnection(LinkManager& lm, uint64_t conn_id, mpmc_channel<boost::json::value> chan) {
        return std::make_shared<ClientConnection>(lm, conn_id, std::move(chan));
    }
    std::function<std::shared_ptr<ClientConnection>(LinkManager&, uint64_t, mpmc_channel<boost::json::value>)> makeConnection(defaultMakeConnection);


}