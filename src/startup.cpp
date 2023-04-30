#include "kaizermud/startup.h"
#include "kaizermud/thermite.h"
#include "kaizermud/game.h"
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/strand.hpp>


namespace kaizermud {

    void startup(const boost::asio::ip::tcp::endpoint& endpoint) {
        // Create an ASIO executor
        boost::asio::io_context io_context;

        // Create an instance of LinkManager
        kaizermud::thermite::LinkManager link_manager(io_context, endpoint);

        // Co_spawn LinkManager's run() method on a strand
        boost::asio::co_spawn(boost::asio::make_strand(io_context), link_manager.run(), boost::asio::detached);

        // Co_spawn the game main loop method on a strand
        boost::asio::co_spawn(boost::asio::make_strand(io_context), kaizermud::game::run(), boost::asio::detached);

        // Run the io_context
        io_context.run();
    }
}