#include "kaizermud/CallParameters.h"

namespace kaizermud {

    CallParameters& CallParameters::setEntity(const std::string& name, entt::entity ent) {
        entities[name] = ent;
        return *this;
    }


    entt::entity CallParameters::getEntity(const std::string& name) const {
        auto it = entities.find(name);
        if(it == entities.end()) return entt::null;
        return it->second;
    }

    std::optional<std::string> CallParameters::getString(const std::string& name) const {
        auto it = strings.find(name);
        if(it == strings.end()) return std::nullopt;
        return it->second;
    }

    std::optional<bool> CallParameters::getBool(const std::string& name) const {
        auto it = bools.find(name);
        if(it == bools.end()) return std::nullopt;
        return it->second;
    }

    std::optional<int64_t> CallParameters::getInt(const std::string& name) const {
        auto it = ints.find(name);
        if(it == ints.end()) return std::nullopt;
        return it->second;
    }

    std::optional<double> CallParameters::getDouble(const std::string& name) const {
        auto it = doubles.find(name);
        if(it == doubles.end()) return std::nullopt;
        return it->second;
    }

    CallParameters& CallParameters::setString(const std::string& name, const std::string& val) {
        strings[name] = val;
        return *this;
    }

    CallParameters& CallParameters::setBool(const std::string& name, bool val) {
        bools[name] = val;
        return *this;
    }

    CallParameters& CallParameters::setInt(const std::string& name, int64_t val) {
        ints[name] = val;
        return *this;
    }

    CallParameters& CallParameters::setDouble(const std::string& name, double val) {
        doubles[name] = val;
        return *this;
    }

}