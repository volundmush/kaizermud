#pragma once

#include "kaizermud/base.h"
#include "kaizermud/CallParameters.h"
#include "kaizermud/Commands.h"
#include <bitset>

namespace kaizer {

    enum class RelationKind : int {
        Location = 0,
        Exit = 1,
        Destination = 2,
        Equipment = 3,
        Home = 4,
        Stow = 5,
        Area = 6

    };

    // A suggestion for how to use Command Type bits...
    // Pretty much all Types should have Object set
    // in order to gain the basic set of commands.
    // A Player type should have Object, Character, and Player set, as an example of usage.
    // These will be checked against a bitmask in the Command class.
    enum class CommandTypes : int {
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
    };


    class Type {
    public:
        [[nodiscard]] virtual std::string getTypeName() const = 0;

        virtual std::bitset<32> getCmdMask() { return cmdMask; }

        virtual ObjectID create();
        // DO NOT USE CREATE WITH AN ID UNLESS YOU KNOW WHAT YOU ARE DOING.
        // IT CAN FUCK UP SQLITE3!
        virtual ObjectID create(ObjectID id);
        virtual ObjectID createProto(const std::string& name);
        virtual ObjectID createRef();

        virtual void atCreate(ObjectID id);
        virtual void atCreateProto(ObjectID proto);
        virtual void atCreateRef(ObjectID ref);

        virtual int16_t getAdminLevel();

        virtual void setRelation(ObjectID id, int relationKind, ObjectID relation);
        virtual ObjectID getRelation(ObjectID id, int relationKind);
        virtual std::vector<ObjectID> getReverseRelation(ObjectID id, int relationKind);

        virtual ObjectID getRefID(ObjectID id);
        virtual ObjectID getRefIDProto(ObjectID proto);

        virtual void setString(ObjectID id, const std::string& key, const std::string& value);
        virtual void setStringProto(ObjectID proto, const std::string& key, const std::string& value);
        virtual void setStringRef(ObjectID ref, const std::string& key, const std::string& value);
        virtual std::string getString(ObjectID id, const std::string& key, bool clean = false);
        virtual std::string getStringProto(ObjectID proto, const std::string& key, bool clean = false);
        virtual std::string getStringRef(ObjectID ref, const std::string& key, bool clean = false);

        virtual void setInt(ObjectID id, const std::string& key, int64_t value);
        virtual void setIntProto(ObjectID proto, const std::string& key, int64_t value);
        virtual void setIntRef(ObjectID ref, const std::string& key, int64_t value);
        virtual int64_t getInt(ObjectID id, const std::string& key);
        virtual int64_t getIntProto(ObjectID proto, const std::string& key);
        virtual int64_t getIntRef(ObjectID ref, const std::string& key);

        virtual void setRealRef(ObjectID ref, const std::string& key, double value);
        virtual void setReal(ObjectID id, const std::string& key, double value);
        virtual void setRealProto(ObjectID proto, const std::string& key, double value);

        virtual double getReal(ObjectID id, const std::string& key);
        virtual double getRealProto(ObjectID proto, const std::string& key);
        virtual double getRealRef(ObjectID ref, const std::string& key);

        virtual void setStatRef(ObjectID ref, const std::string& key, double value);
        virtual void setStat(ObjectID id, const std::string& key, double value);
        virtual void setStatProto(ObjectID proto, const std::string& key, double value);

        virtual double getStat(ObjectID id, const std::string& key);
        virtual double getStatProto(ObjectID proto, const std::string& key);
        virtual double getStatRef(ObjectID ref, const std::string& key);

        virtual std::string getName(ObjectID id);

        virtual void sendText(ObjectID id, const std::string& text);

        virtual OpResult<> moveTo(ObjectID id, CallParameters& param);
        virtual OpResult<> atPreMove(ObjectID id, CallParameters& param);
        virtual OpResult<> atPreObjectLeave(ObjectID id, CallParameters& param);
        virtual OpResult<> atPreObjectReceive(ObjectID id, CallParameters& param);
        virtual void atObjectLeave(ObjectID id, CallParameters& param);
        virtual void atObjectReceive(ObjectID id, CallParameters& param);
        virtual void atPostMove(ObjectID id, CallParameters& param);
        virtual void announceMoveFrom(ObjectID id, CallParameters& param);
        virtual void announceMoveTo(ObjectID id, CallParameters& param);

        virtual void stow(ObjectID id);
        virtual OpResult<ObjectID> getUnstowLocation(ObjectID id);
        virtual OpResult<> unstow(ObjectID id);

        virtual std::string getDisplayName(ObjectID id, ObjectID looker);
        virtual std::string getRoomLine(ObjectID id, ObjectID looker);

        virtual void atLook(ObjectID id, ObjectID target);
        virtual void atDesc(ObjectID id, ObjectID looker);

        virtual std::string renderAppearance(ObjectID id, ObjectID looker);

        virtual std::set<std::string> getSearchWords(ObjectID id, ObjectID looker);
        virtual bool checkSearch(ObjectID id, const std::string& term, ObjectID looker);

        virtual bool canDetect(ObjectID id, ObjectID target, const std::string& sense);

        virtual const std::vector<std::pair<std::string, Command*>>& getSortedCommands();
        virtual const std::unordered_map<std::string, Command*>& getCommands();

    protected:
        std::bitset<32> cmdMask;
        std::vector<std::pair<std::string, Command*>> sortedCommands;
        std::unordered_map<std::string, Command*> commands;
        bool commandsCached{false}, sortedCommandsCached{false};
    };

    extern std::unordered_map<std::string , std::shared_ptr<Type>> typeRegistry;

    void registerType(const std::shared_ptr<Type>& entry);

}
