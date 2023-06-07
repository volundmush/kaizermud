#pragma once
#include "kaizermud/base.h"
#include "nlohmann/json.hpp"

namespace kaizer {
    class Prototype {
    public:
        Prototype(std::string_view name, nlohmann::json data);
        virtual OpResult<entt::entity> spawn();
        std::string_view getName();
        const nlohmann::json &getData();
    protected:
        std::string name;
        nlohmann::json data;
    };


    extern std::unordered_map<std::string, std::shared_ptr<Prototype>> prototypes;

    std::shared_ptr<Prototype> defaultMakePrototype(std::string_view name, nlohmann::json data);

    extern std::function<std::shared_ptr<Prototype>(std::string_view, nlohmann::json)> makePrototype;

    void registerPrototype(std::string_view name, nlohmann::json data);

}