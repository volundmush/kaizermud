#include "kaizermud/CallParameters.h"

namespace kaizermud {

    CallParameters& CallParameters::setObject(std::string_view name, const std::shared_ptr<Object> &obj) {
        objects[std::string(name)] = obj;
        return *this;
    }


    std::shared_ptr<game::Object> CallParameters::getObject(std::string_view name) const {
        auto it = objects.find(std::string(name));
        if(it == objects.end()) return nullptr;
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