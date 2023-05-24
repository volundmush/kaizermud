#include "kaizermud/Room.h"

namespace kaizermud::game {

    std::string_view Room::getType() const {
        return "room";
    }
}