#pragma once

#include "kaizermud/base.h"

namespace kaizermud {
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

        CallParameters& setString(const std::string& name, const std::string& val);
        CallParameters& setBool(const std::string& name, bool val);
        CallParameters& setInt(const std::string& name, int64_t val);
        CallParameters& setDouble(const std::string& name, double val);
        CallParameters& setEntity(const std::string& name, entt::entity ent);

        [[nodiscard]] std::optional<std::string> getString(const std::string& name) const;
        [[nodiscard]] std::optional<bool> getBool(const std::string& name) const;
        [[nodiscard]] std::optional<int64_t> getInt(const std::string& name) const;
        [[nodiscard]] std::optional<double> getDouble(const std::string& name) const;
        [[nodiscard]] entt::entity getEntity(const std::string& name) const;

    private:
        std::unordered_map<std::string, int64_t> ints;
        std::unordered_map<std::string, double> doubles;
        std::unordered_map<std::string, bool> bools;
        std::unordered_map<std::string, std::string> strings;
        std::unordered_map<std::string, entt::entity> entities;
    };
}