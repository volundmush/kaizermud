#include "kaizermud/base/Types.h"

namespace kaizer::base {

    void registerTypes() {
        registerType(std::make_shared<Sector>());
        registerType(std::make_shared<Object>());
        registerType(std::make_shared<Character>());
        registerType(std::make_shared<Player>());
        registerType(std::make_shared<Character>());
        registerType(std::make_shared<NPC>());
        registerType(std::make_shared<Room>());
        registerType(std::make_shared<Exit>());
        registerType(std::make_shared<Item>());
        registerType(std::make_shared<Area>());
        registerType(std::make_shared<Grid>());
    }

}