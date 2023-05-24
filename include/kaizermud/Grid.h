#pragma once
#include "kaizermud/Object.h"

namespace kaizermud::game {

    // The Grid class is a kind of Object which is specialized for representing a 2D grid of tiles, such
    // as for making an overworld map. It is not intended to be used for representing the rooms of a
    // MUD, which are better represented by the Room class.

    // TODO: Implement the Grid class.
    class Grid final : public Object {
    public:
        [[nodiscard]] ObjectType getType() const override {
            return ObjectType::Grid;
        }
    };
}