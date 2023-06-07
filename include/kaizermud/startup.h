#pragma once
#include "base.h"
#include <boost/asio.hpp>
#include "kaizermud/thermite.h"

namespace kaizer {

    extern std::vector<std::function<boost::asio::awaitable<void>()>> services;
    extern std::unique_ptr<LinkManager> linkManager;

    void startup(const boost::asio::ip::tcp::endpoint& endpoint);
    void registerBaseResources();

}