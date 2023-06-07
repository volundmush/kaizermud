#include "kaizermud/base/Types.h"

namespace kaizer::base {

    void registerTypes() {
        std::vector<std::shared_ptr<Type>> types;
        types.emplace_back(std::make_shared<Object>());
        types.emplace_back(std::make_shared<Character>());
        types.emplace_back(std::make_shared<Player>());
        types.emplace_back(std::make_shared<Character>());
        types.emplace_back(std::make_shared<NPC>());
        types.emplace_back(std::make_shared<Room>());
        types.emplace_back(std::make_shared<Exit>());
        types.emplace_back(std::make_shared<Item>());
        types.emplace_back(std::make_shared<Area>());
        types.emplace_back(std::make_shared<Grid>());
        types.emplace_back(std::make_shared<Sector>());

        for (auto& type : types) {
            typeRegistry[std::string(type->getKey())] = type;
        }

    }

}