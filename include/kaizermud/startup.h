#pragma once
#include "base.h"
#include <boost/asio.hpp>

namespace kaizermud {

    void startup(const boost::asio::ip::tcp::endpoint& endpoint);

}