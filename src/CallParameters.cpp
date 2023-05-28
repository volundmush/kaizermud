#include "kaizermud/CallParameters.h"

namespace kaizermud {

    CallParameters& CallParameters::setEntity(std::string_view name, entt::entity ent) {
        entities[std::string(name)] = ent;
        return *this;
    }


    entt::entity CallParameters::getEntity(std::string_view name) const {
        auto it = entities.find(std::string(name));
        if(it == entities.end()) return entt::null;
        return it->second;
    }

    std::optional<std::string_view> CallParameters::getString(std::string_view name) const {
        auto it = strings.find(std::string(name));
        if(it == strings.end()) return std::nullopt;
        return it->second;
    }

    std::optional<bool> CallParameters::getBool(std::string_view name) const {
        auto it = bools.find(std::string(name));
        if(it == bools.end()) return std::nullopt;
        return it->second;
    }

    std::optional<int64_t> CallParameters::getInt(std::string_view name) const {
        auto it = ints.find(std::string(name));
        if(it == ints.end()) return std::nullopt;
        return it->second;
    }

    std::optional<double> CallParameters::getDouble(std::string_view name) const {
        auto it = doubles.find(std::string(name));
        if(it == doubles.end()) return std::nullopt;
        return it->second;
    }

    CallParameters& CallParameters::setString(std::string_view name, std::string_view val) {
        strings[std::string(name)] = val;
        return *this;
    }

    CallParameters& CallParameters::setBool(std::string_view name, bool val) {
        bools[std::string(name)] = val;
        return *this;
    }

    CallParameters& CallParameters::setInt(std::string_view name, int64_t val) {
        ints[std::string(name)] = val;
        return *this;
    }

    CallParameters& CallParameters::setDouble(std::string_view name, double val) {
        doubles[std::string(name)] = val;
        return *this;
    }

}