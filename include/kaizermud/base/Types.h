#pragma once

#include "kaizermud/Types.h"

namespace kaizer::base {

    enum class ObjectType : std::size_t {
        // Tier 0 exclusively contains the base Object type. Nothing else should be a Tier 0 type.
        // Object represents any in-game entity with a "physical presence" in the game world, though
        // that might be slightly abstract, like "the room you are in."
        Object     = 0,
        // Tier 1 ObjectTypes are the base Types.
        // Generally, everything will be a combination of Object and a tier 1 type.
        Item       = 1,
        Character  = 2,
        Room       = 3,
        Exit       = 4,
        Grid       = 5,
        Sector     = 6,
        Area       = 7,
        
        // Tier 2 types. Some objects might have a Tier 2. By default that's Character->NPC or
        // Character->Player.
        NPC        = 16,
        Player     = 17,
        
        // Tier 3? These should probably start at 24 or 28... there should be very, very few of these.
    };
    
    /*
     * The base "object" Type is used for all in-game entities. Whenever a resource
     * is shared by all kinds of things, it should be tied to this Type.
     */
    struct Object : Type {
        [[nodiscard]] std::size_t getID() const override {return 0;}
        [[nodiscard]] std::string_view getName() const override {return "Object";}

    };

    /*
     * The base "character" Type is used for all in-game characters. Whenever a resource
     * is shared by all kinds of characters, it should be tied to this Type.
     */
    struct Character : Type {
        [[nodiscard]] std::size_t getID() const override {return 2;}
        [[nodiscard]] std::string_view getName() const override {return "Character";}
    };

    /*
     * The base "player" Type is used for all player characters.
     */
    struct Player : Type {
        [[nodiscard]] std::size_t getID() const override {return 17;}
        [[nodiscard]] std::string_view getName() const override {return "Player";}
    };

    /*
     * This is for NPCs, aka, "Non-Player Characters".
     */
    struct NPC : Type {
        [[nodiscard]] std::size_t getID() const override {return 16;}
        [[nodiscard]] std::string_view getName() const override {return "NPC";}
    };

    /*
     * The base "room" Type is used for all in-game rooms. Whenever a resource
     * is shared by all kinds of rooms, it should be tied to this Type.
     */
    struct Room : Type {
        [[nodiscard]] std::size_t getID() const override { return 3; }
        [[nodiscard]] std::string_view getName() const override { return "Room"; }
    };

    /*
     * The base "exit" Type is used for all in-game exits. Whenever a resource
     * is shared by all kinds of exits, it should be tied to this Type.
     */
    struct Exit : Type {
        [[nodiscard]] std::size_t getID() const override { return 4; }
        [[nodiscard]] std::string_view getName() const override { return "Exit"; }
    };

    /*
     * The base "item" Type is used for all in-game items. Whenever a resource
     * is shared by all kinds of items, it should be tied to this Type.
     */
    struct Item : Type {
        [[nodiscard]] std::size_t getID() const override { return 1; }
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
        [[nodiscard]] std::size_t getID() const override { return 7; }
        [[nodiscard]] std::string_view getName() const override { return "Area"; }
    };

    /*
     * Grids are meant to be objects which contain a grid of tiles to serve as an
     * overworld map instead of Rooms. Implementation TBD.
     */
    struct Grid : Type {
        [[nodiscard]] std::size_t getID() const override { return 5; }
        [[nodiscard]] std::string_view getName() const override { return "Grid"; }
    };

    /*
     * Sectors are meant to be objects which contain a floating point 3D space full
     * of objects. This meant to be used for space ships, space systems, planets,
     * and other things floating in an endless void. Implementation TBD.
     */
    struct Sector : Type {
        [[nodiscard]] std::size_t getID() const override { return 6; }
        [[nodiscard]] std::string_view getName() const override { return "Sector"; }
    };

    void registerTypes();

}