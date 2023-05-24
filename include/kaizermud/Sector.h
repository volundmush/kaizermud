#pragma once
#include "kaizermud/Object.h"

namespace kaizermud::game {

    // The Sector class is an Object which encloses a 3D grid using floating point XYZ coordinates.
    // It is intended to be used for dynamic "outer space" regions full of spaceships and asteroids
    // and other things, in which Structures, Characters, and Items might be floating around.
    // For example, a Structure might serve as a Planet or a Space Station, or even a Spaceship that
    // you can dock another spaceship in.

    // This is probably gonna require an octree...
    // TODO: Implement the Sector class.
    class Sector final : public Object {
    public:
        [[nodiscard]] ObjectType getType() const override {
            return ObjectType::Sector;
        }
    };
}