#pragma once

#include "kaizermud/base.h"

namespace kaizer {
    /*
     * @brief A set of parameters to pass to a command. Inspired by Python **kwargs
     *
     * This class is used to pass parameters to a command. It's a bit like Python's
     * **kwargs, but with some type safety.
     *
     */
    class CallParameters {
    public:
        CallParameters() = default;

        CallParameters& setString(std::string_view name, std::string_view val);
        CallParameters& setBool(std::string_view name, bool val);
        CallParameters& setInt(std::string_view name, int64_t val);
        CallParameters& setDouble(std::string_view name, double val);
        CallParameters& setEntity(std::string_view name, entt::entity ent);

        [[nodiscard]] std::optional<std::string_view> getString(std::string_view name) const;
        [[nodiscard]] std::optional<bool> getBool(std::string_view name) const;
        [[nodiscard]] std::optional<int64_t> getInt(std::string_view name) const;
        [[nodiscard]] std::optional<double> getDouble(std::string_view name) const;
        [[nodiscard]] entt::entity getEntity(std::string_view name) const;

    private:
        std::unordered_map<std::string, int64_t> ints;
        std::unordered_map<std::string, double> doubles;
        std::unordered_map<std::string, bool> bools;
        std::unordered_map<std::string, std::string_view> strings;
        std::unordered_map<std::string, entt::entity> entities;
    };
}