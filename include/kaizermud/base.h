#pragma once

#include <cstdint>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>

namespace kaizermud {
    template<typename T>
    using spsc_channel = boost::asio::experimental::channel<void(boost::system::error_code, T)>;

    template<typename T>
    using mpmc_channel = boost::asio::experimental::concurrent_channel<void(boost::system::error_code, T)>;
}