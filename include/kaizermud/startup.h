#pragma once
#include "base.h"
#include <boost/asio.hpp>

namespace kaizermud {

    extern std::vector<std::function<boost::asio::awaitable<void>()>> services;

    void startup(const boost::asio::ip::tcp::endpoint& endpoint);

}