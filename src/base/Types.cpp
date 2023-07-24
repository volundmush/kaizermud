#include "kaizermud/base/Types.h"

namespace kaizer::base {


    Player::Player() {
        cmdMask[0] = true;
        cmdMask[2] = true;
        cmdMask[17] = true;
    }

    NPC::NPC() {
        cmdMask[0] = true;
        cmdMask[2] = true;
        cmdMask[16] = true;
    }

    Room::Room() {
        cmdMask[0] = true;
        cmdMask[3] = true;
    }

    Exit::Exit() {
        cmdMask[0] = true;
        cmdMask[4] = true;
    }

    Item::Item() {
        cmdMask[0] = true;
        cmdMask[1] = true;
    }

    Area::Area() {
        cmdMask[0] = true;
        cmdMask[7] = true;
    }

    Grid::Grid() {
        cmdMask[0] = true;
        cmdMask[5] = true;
    }

    Sector::Sector() {
        cmdMask[0] = true;
        cmdMask[6] = true;
    }

    void registerTypes() {
        registerType(std::make_shared<Sector>());
        registerType(std::make_shared<Player>());
        registerType(std::make_shared<NPC>());
        registerType(std::make_shared<Room>());
        registerType(std::make_shared<Exit>());
        registerType(std::make_shared<Item>());
        registerType(std::make_shared<Area>());
        registerType(std::make_shared<Grid>());
    }

}