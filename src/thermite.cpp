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
        while (!is_stopped) {
            try {
                // Create a new TCP socket
                boost::asio::ip::tcp::socket socket(co_await boost::asio::this_coro::executor);

                // Connect to the endpoint
                co_await socket.async_connect(endpoint, boost::asio::use_awaitable);

                // Initialize a WebSocket using the connected socket
                boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws(std::move(socket));

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
                // co_await boost::asio::steady_timer(co_await boost::asio::this_coro::executor, std::chrono::seconds(5)).async_wait(boost::asio::use_awaitable);
            }
        }
        co_return;
    }

    void LinkManager::stop() {
        is_stopped = true;
    }

    Link::Link(LinkManager& lm, boost::beast::websocket::stream<boost::asio::ip::tcp::socket>& ws)
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

    boost::asio::awaitable<void> Link::runReader() {
        try {
            while (true) {
                // Read a message from the WebSocket
                boost::beast::flat_buffer buffer;
                co_await conn.async_read(buffer, boost::asio::use_awaitable);

                // Deserialize the JSON string
                auto json_data = boost::json::parse(boost::beast::buffers_to_string(buffer.data()));
                boost::json::object j = json_data.as_object();

                // Access the "kind" field in the JSON object
                auto kind = j["kind"].as_string();

                // Implement your routing logic here

                if (kind == "client_list") {
                    // Handle client_list
                    // ...
                } else if (kind == "client_ready") {
                    // Handle client_ready
                    // ...
                } else {
                    // Extract the "id" field from the JSON object
                    uint64_t id = j["id"].as_uint64();

                    // Look up the specific ClientConnection in the std::map
                    auto it = kaizermud::net::connections.find(id);
                    if (it != kaizermud::net::connections.end()) {
                        // Found the client connection
                        auto& client_connection = it->second;

                        if (kind == "client_capabilities") {
                            // Handle client_capabilities
                            // ...
                        } else if (kind == "client_line") {
                            // Handle client_line
                            // ...
                        } else if (kind == "client_gmcp") {
                            // Handle client_gmcp
                            // ...
                        } else if (kind == "client_disconnected") {
                            // Handle client_disconnected
                            // ...
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
            // Receive a message from the channel asynchronously
            boost::system::error_code ec;
            boost::json::value message = co_await linkManager.channel.async_receive(boost::asio::use_awaitable);

            if (!ec) {
                // Serialize the JSON message to text
                std::string serialized_msg = boost::json::serialize(message);

                // Send the message across the WebSocket
                co_await conn.async_write(boost::asio::buffer(serialized_msg), boost::asio::use_awaitable);
            } else {
                // Handle errors if necessary
            }
        }
        co_return;
    }

}