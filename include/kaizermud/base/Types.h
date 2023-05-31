#pragma once

#include "kaizermud/Types.h"

namespace kaizer::base {

    /*
     * The base "object" Type is used for all in-game entities. Whenever a resource
     * is shared by all kinds of things, it should be tied to this Type.
     */
    struct Object : Type {
        [[nodiscard]] std::string_view getKey() const override {return "object";}
        [[nodiscard]] std::string_view getName() const override {return "Object";}

        // the default Object type should always be last on any priority...
        [[nodiscard]] int getSortPriority() const override {return -10000;}
    };

    /*
     * The base "character" Type is used for all in-game characters. Whenever a resource
     * is shared by all kinds of characters, it should be tied to this Type.
     */
    struct Character : Type {
        [[nodiscard]] std::string_view getKey() const override {return "character";}
        [[nodiscard]] std::string_view getName() const override {return "Character";}
    };

    /*
     * The base "player" Type is used for all player characters.
     */
    struct Player : Type {
        [[nodiscard]] std::string_view getKey() const override {return "player";}
        [[nodiscard]] std::string_view getName() const override {return "Player";}

        // the default Player type should be higher than Character.
        [[nodiscard]] int getSortPriority() const override {return 100;}
    };

    /*
     * This is for NPCs, aka, "Non-Player Characters".
     */
    struct NPC : Type {
        [[nodiscard]] std::string_view getKey() const override {return "npc";}
        [[nodiscard]] std::string_view getName() const override {return "NPC";}

        // the default NPC type should be higher than Character.
        [[nodiscard]] int getSortPriority() const override {return 80;}
    };

    /*
     * The base "room" Type is used for all in-game rooms. Whenever a resource
     * is shared by all kinds of rooms, it should be tied to this Type.
     */
    struct Room : Type {
        [[nodiscard]] std::string_view getKey() const override { return "room"; }

        [[nodiscard]] std::string_view getName() const override { return "Room"; }

        // the default Room type should always be last on any priority...
        [[nodiscard]] int getSortPriority() const override { return -10000; }
    };

    /*
     * The base "exit" Type is used for all in-game exits. Whenever a resource
     * is shared by all kinds of exits, it should be tied to this Type.
     */
    struct Exit : Type {
        [[nodiscard]] std::string_view getKey() const override { return "exit"; }
        [[nodiscard]] std::string_view getName() const override { return "Exit"; }
    };

    /*
     * The base "item" Type is used for all in-game items. Whenever a resource
     * is shared by all kinds of items, it should be tied to this Type.
     */
    struct Item : Type {
        [[nodiscard]] std::string_view getKey() const override { return "item"; }
        [[nodiscard]] std::string_view getName() const override { return "Item"; }
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
        [[nodiscard]] std::string_view getKey() const override { return "area"; }
        [[nodiscard]] std::string_view getName() const override { return "Area"; }
    };

    /*
     * Grids are meant to be objects which contain a grid of tiles to serve as an
     * overworld map instead of Rooms. Implementation TBD.
     */
    struct Grid : Type {
        [[nodiscard]] std::string_view getKey() const override { return "grid"; }
        [[nodiscard]] std::string_view getName() const override { return "Grid"; }
    };

    /*
     * Sectors are meant to be objects which contain a floating point 3D space full
     * of objects. This meant to be used for space ships, space systems, planets,
     * and other things floating in an endless void. Implementation TBD.
     */
    struct Sector : Type {
        [[nodiscard]] std::string_view getKey() const override { return "sector"; }
        [[nodiscard]] std::string_view getName() const override { return "Sector"; }
    };

}