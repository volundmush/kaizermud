#pragma once

#include <cstdint>
#include <boost/asio/experimental/channel.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <optional>
#include <chrono>
#include "entt/entt.hpp"

namespace kaizer {

    class ClientConnection;
    class Session;

    template<typename T>
    using mpmc_channel = boost::asio::experimental::concurrent_channel<void(boost::system::error_code, T)>;

    // I WOULD use uint64_t but then sqlite3 would hate me.
    using ObjectID = int64_t;

    template<typename T = bool>
    using OpResult = std::pair<T, std::optional<std::string>>;

    extern entt::registry registry;
    extern std::unordered_map<ObjectID, entt::entity> entities;

    OpResult<entt::entity> createEntity(std::optional<ObjectID> id = std::nullopt);

    enum class ColorType : uint8_t {
        NoColor = 0,
        Standard = 1,
        Xterm256 = 2,
        TrueColor = 3
    };


}