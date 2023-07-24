#pragma once

#include "kaizermud/Types.h"

namespace kaizer::base {
        /*
     * The base "player" Type is used for all player characters.
     * In practice, player and npc should probably share a base class...
     */
    struct Player : Type {
        Player();
        [[nodiscard]] std::string getTypeName() const override {return "player";}
    };

    /*
     * This is for NPCs, aka, "Non-Player Characters".
     */
    struct NPC : Type {
        NPC();
        [[nodiscard]] std::string getTypeName() const override {return "npc";}
    };

    /*
     * The base "room" Type is used for all in-game rooms. Whenever a resource
     * is shared by all kinds of rooms, it should be tied to this Type.
     */
    struct Room : Type {
        Room();
        [[nodiscard]] std::string getTypeName() const override { return "room"; }
    };

    /*
     * The base "exit" Type is used for all in-game exits. Whenever a resource
     * is shared by all kinds of exits, it should be tied to this Type.
     */
    struct Exit : Type {
        Exit();
        [[nodiscard]] std::string getTypeName() const override { return "exit"; }
    };

    /*
     * The base "item" Type is used for all in-game items. Whenever a resource
     * is shared by all kinds of items, it should be tied to this Type.
     */
    struct Item : Type {
        Item();
        [[nodiscard]] std::string getTypeName() const override { return "item"; }
    };

    /*
     * An "area" is a type of object which groups Rooms together. This type
     * has logic which allows for logical operations that target groups of
     * rooms. It is likely to be combined with other kinds of Types, like
     * "house" or "city" or "dungeon". "Area" by itself is also possible,
     * in which case they are unlikely to have any sort of in-game location
     * and are more likely to be a purely logical container of Rooms.
     *
     * Areas can also be used to group other Areas in an "area tree".
     */
    struct Area : Type {
        Area();
        [[nodiscard]] std::string getTypeName() const override { return "area"; }
    };

    /*
     * Grids are meant to be objects which contain a grid of tiles to serve as an
     * overworld map instead of Rooms. Implementation TBD.
     */
    struct Grid : Type {
        Grid();
        [[nodiscard]] std::string getTypeName() const override { return "grid"; }
    };

    /*
     * Sectors are meant to be objects which contain a floating point 3D space full
     * of objects. This meant to be used for space ships, space systems, planets,
     * and other things floating in an endless void. Implementation TBD.
     */
    struct Sector : Type {
        Sector();
        [[nodiscard]] std::string getTypeName() const override { return "sector"; }
    };

    void registerTypes();

}