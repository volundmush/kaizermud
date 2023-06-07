#include "kaizermud/Prototypes.h"

namespace kaizer {
    Prototype::Prototype(std::string_view name, nlohmann::json data) : name(name), data(std::move(data)) {

    }

    OpResult<entt::entity> Prototype::spawn() {
       return {entt::null, "Not implemented"};
    }

    std::string_view Prototype::getName() {
       return name;
    }

    const nlohmann::json &Prototype::getData() {
       return data;
    }

    std::shared_ptr<Prototype> defaultMakePrototype(std::string_view name, nlohmann::json data) {
         return std::make_shared<Prototype>(name, std::move(data));
    }

    std::function<std::shared_ptr<Prototype>(std::string_view, nlohmann::json)> makePrototype(defaultMakePrototype);

    std::unordered_map<std::string, std::shared_ptr<Prototype>> prototypes;

    void registerPrototype(std::string_view name, nlohmann::json data) {
       prototypes.emplace(name, makePrototype(name, std::move(data)));
    }


}