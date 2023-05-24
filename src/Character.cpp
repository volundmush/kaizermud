#include "kaizermud/Character.h"

namespace kaizermud::game {

    std::string_view Character::getType() const {
        return "character";
    }
}