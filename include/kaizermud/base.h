#pragma once

#include <cstdint>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <optional>
#include "entt/entt.hpp"

namespace kaizermud {
    template<typename T>
    using spsc_channel = boost::asio::experimental::channel<void(boost::system::error_code, T)>;

    template<typename T>
    using mpmc_channel = boost::asio::experimental::concurrent_channel<void(boost::system::error_code, T)>;

    // I WOULD use uint64_t but then sqlite3 would hate me.
    using ObjectID = int64_t;

    template<typename T = bool>
    using OpResult = std::pair<T, std::optional<std::string>>;

    extern entt::registry registry;
    extern std::unordered_map<ObjectID, entt::entity> entities;

    OpResult<entt::entity> createEntity(std::string_view objType, std::string_view subType, std::optional<ObjectID> id = std::nullopt);


}