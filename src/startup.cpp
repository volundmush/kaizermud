#include "kaizermud/startup.h"
#include "kaizermud/thermite.h"
#include "kaizermud/game.h"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/strand.hpp>
#include "sodium.h"
#include "spdlog/spdlog.h"

namespace kaizer {

    std::vector<std::function<boost::asio::awaitable<void>()>> services;

    void startup(const boost::asio::ip::tcp::endpoint& endpoint) {
        try {
            // Initialize libsodium
            spdlog::info("Initializing libsodium...");
            if (sodium_init() == -1) {
                throw std::runtime_error("Could not initialize libsodium!");
            }

            // Create an ASIO executor
            boost::asio::io_context io_context;

            // Create an instance of LinkManager
            LinkManager link_manager(io_context, endpoint);

            // Co_spawn LinkManager's run() method on a strand
            spdlog::info("Starting the LinkManager...");
            boost::asio::co_spawn(boost::asio::make_strand(io_context), link_manager.run(), boost::asio::detached);

            spdlog::info("Preparing the game loop...");
            // Co_spawn the game main loop method on a strand
            boost::asio::co_spawn(boost::asio::make_strand(io_context), run(), boost::asio::detached);

            // Co_spawn each service on a strand
            spdlog::info("Starting services...");
            for (auto &service: services) {
                boost::asio::co_spawn(boost::asio::make_strand(io_context), service(), boost::asio::detached);
            }

            // Run the io_context
            spdlog::info("Entering main loop...");
            io_context.run();
            spdlog::info("Main loop exited.");
        }
        catch (std::exception &e) {
            spdlog::critical("Exception in startup(): {}", e.what());
        }
        catch (...) {
            spdlog::critical("Unknown exception in startup()!");
        }
        spdlog::info("Exiting...");
    }
}