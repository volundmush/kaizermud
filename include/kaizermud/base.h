#pragma once

#include <cstdint>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <optional>
#include <string_view>

namespace kaizermud {
    template<typename T>
    using spsc_channel = boost::asio::experimental::channel<void(boost::system::error_code, T)>;

    template<typename T>
    using mpmc_channel = boost::asio::experimental::concurrent_channel<void(boost::system::error_code, T)>;

    namespace game {
        class Object;
        class Room;
        class Item;
        class Character;
        class Session;
        class Location;
        class Sector;
        class Grid;
        struct Message;
    }

    using ObjectID = uint64_t;
    using OpResult = std::pair<bool, std::optional<std::string_view>>;

}