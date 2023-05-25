#pragma once
#include "kaizermud/base.h"
#include <set>
#include <optional>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <boost/asio/experimental/channel.hpp>
#include <utility>
#include "kaizermud/thermite.h"
#include "kaizermud/ClientConnection.h"
#include "kaizermud/Stats.h"
#include "kaizermud/Aspects.h"
#include "kaizermud/Equip.h"

namespace kaizermud::game {

    /**
     *  @brief Base class for all game objects.
     *
     *  This class is the base class for all game objects. It provides a unique ID,
     *  a type system, and a few other things.
     * */
    class Object : public std::enable_shared_from_this<Object> {
    public:
        explicit Object(ObjectID id);
        virtual ~Object() = default;

        // Retrieve the Object's unique ID. IDs should never be re-used.
        [[nodiscard]] ObjectID getId() const;

        // All Objects have a base Type and, possibly, a Sub-type.
        // These together determine which class this is identified as, what
        // stats to give it, which commands it can access, and so on.
        [[nodiscard]] virtual std::string_view getMainType() const;
        [[nodiscard]] virtual std::string_view getSubType() const;

        // This returns the above two in order by default, but can be used to
        // specify additional subtypes to load data from for stats, commands, etc.
        // It's best for this to not change once the object is created and loaded.
        [[nodiscard]] virtual std::vector<std::string_view> getTypes() const;
        [[nodiscard]] virtual bool matchType(std::string_view type) const;

        // All Objects can support a connection to a Session, its link to a player.
        // Sessions themselves might be linked to an arbitrary amount of network connections.
        // Ideally, Anything, not just a Player Character, can be directly puppeted.
        // Not sure what use it is to puppet a room, but whatever.
        [[nodiscard]] const std::shared_ptr<Session>& getSession() const;
        virtual void setSession(const std::shared_ptr<Session>& sess);
        virtual void onPreSessionLink(const std::shared_ptr<Session>& sess);
        virtual void onPostSessionLink(const std::shared_ptr<Session>& sess);
        virtual void removeSession();
        virtual void onPreSessionUnlink(const std::shared_ptr<Session>& sess);
        virtual void onPostSessionUnlink(const std::shared_ptr<Session>& sess);

        std::unique_ptr<StatHandler> statHandler{};
        std::unique_ptr<AspectHandler> aspectHandler{};
        std::unique_ptr<EquipHandler> equipHandler{};

        [[nodiscard]] virtual std::string_view getString(std::string_view key) const;
        virtual OpResult setString(std::string_view key, const std::string &value);

        virtual void send(const Message &msg);

        // Administration Hooks
        virtual void atCreate();
        virtual void atDelete();
        virtual void onLoad();
        virtual void atServerReload();
        virtual void atServerShutdown();

        // Info API Hooks
        [[nodiscard]] virtual bool isSuperuser();
        [[nodiscard]] virtual std::string getDisplayName(std::shared_ptr<Object>& looker, std::string_view type = "normal");
        [[nodiscard]] virtual std::string renderAppearance(std::shared_ptr<Object>& looker, std::string_view type = "normal");

        [[nodiscard]] virtual std::string atLook(std::shared_ptr<Object>& target);
        virtual void atDesc(const std::shared_ptr<Object>& looker);


        // Inventory API Hooks
        [[nodiscard]] virtual std::vector<std::shared_ptr<Object>> getContents();
        [[nodiscard]] virtual std::vector<std::shared_ptr<Object>> getContents(std::string_view type);

        // Get
        [[nodiscard]] virtual OpResult doGet(const std::shared_ptr<Object>& target);
        [[nodiscard]] virtual OpResult canGetObject(const std::shared_ptr<Object>& target);
        [[nodiscard]] virtual OpResult atPreGet(const std::shared_ptr<Object>& getter);
        virtual void atGet(const std::shared_ptr<Object>& getter);

        // Drop
        [[nodiscard]] virtual OpResult doDrop(const std::shared_ptr<Object>& target);
        [[nodiscard]] virtual OpResult canDropObject(const std::shared_ptr<Object>& target);
        [[nodiscard]] virtual OpResult atPreDrop(const std::shared_ptr<Object>& dropper);
        virtual void atDrop(const std::shared_ptr<Object>& dropper);

        // Give
        [[nodiscard]] virtual OpResult doGive(const std::shared_ptr<Object>& target, const std::shared_ptr<Object>& recipient);
        [[nodiscard]] virtual OpResult canGiveObject(const std::shared_ptr<Object>& target, const std::shared_ptr<Object>& recipient);
        [[nodiscard]] virtual OpResult atPreGive(const std::shared_ptr<Object>& giver, const std::shared_ptr<Object>& recipient);
        virtual void atGive(const std::shared_ptr<Object>& giver, const std::shared_ptr<Object>& recipient);

        // Put
        [[nodiscard]] virtual OpResult doPut(const std::shared_ptr<Object>& target, const std::shared_ptr<Object>& container);
        [[nodiscard]] virtual OpResult canPutObject(const std::shared_ptr<Object>& target, const std::shared_ptr<Object>& container);
        [[nodiscard]] virtual OpResult atPrePut(const std::shared_ptr<Object>& putter, const std::shared_ptr<Object>& container);
        virtual void atPut(const std::shared_ptr<Object>& putter, const std::shared_ptr<Object>& container);


        virtual void atEquipItem(const std::shared_ptr<Object>& item, std::string_view slot);
        virtual void atRemoveItem(const std::shared_ptr<Object>& item, std::string_view slot);

        // Exit API hooks
        // Target is an exit Object. Or anything with a Destination Set.
        [[nodiscard]] virtual OpResult doTraverse(const std::shared_ptr<Object>& target);
        [[nodiscard]] virtual OpResult atTraverse(const std::shared_ptr<Object>& traverser, const std::shared_ptr<Object>& dest);
        virtual void atPostTraverse(const std::shared_ptr<Object>& traverser, const std::shared_ptr<Object>& source);
        virtual void atFailedTraverse(const std::shared_ptr<Object>& traverser, const std::shared_ptr<Object>& dest);

        // Interaction and Filtering Hooks
        virtual bool canDetect(const std::shared_ptr<Object>& target, std::string_view detectType = "see");
        virtual std::vector<std::shared_ptr<Object>> filterDetect(std::vector<std::shared_ptr<Object>>& objects, std::string_view detectType = "see");

        // Communications and Social Hooks
        [[nodiscard]] virtual OpResult doSay(std::string_view speech);
        [[nodiscard]] virtual OpResult doWhisper(std::string_view speech, std::shared_ptr<Object>& target);
        [[nodiscard]] virtual OpResult doEmote(std::string_view speech);

        virtual void act(std::string_view message, int targets, const std::unordered_map<std::string, std::shared_ptr<Object>>& actors, std::string_view actType = "act", bool excludeSource = false);

        // Movement and Location API hooks.
        [[nodiscard]] virtual OpResult moveTo(ObjectID dest, bool quiet = false, std::string_view moveType = "move");
        [[nodiscard]] virtual OpResult moveTo(const std::shared_ptr<Object>& dest, bool quiet = false, std::string_view moveType = "move");
        [[nodiscard]] virtual OpResult atPreMove(const std::shared_ptr<Object>& dest, std::string_view moveType);
        [[nodiscard]] virtual OpResult atPreObjectLeave(const std::shared_ptr<Object>& obj, const std::shared_ptr<Object>& dest, std::string_view moveType);
        [[nodiscard]] virtual OpResult atPreObjectReceive(const std::shared_ptr<Object>& obj, const std::shared_ptr<Object>& source, std::string_view moveType);
        virtual void atObjectLeave(const std::shared_ptr<Object>& obj, const std::shared_ptr<Object>& dest, std::string_view moveType);
        virtual void announceMoveFrom(const std::shared_ptr<Object>& source, const std::shared_ptr<Object>& dest, std::string_view moveType);
        virtual void announceMoveTo(const std::shared_ptr<Object>& source, const std::shared_ptr<Object>& dest, std::string_view moveType);
        virtual void atObjectReceive(const std::shared_ptr<Object>& obj, const std::shared_ptr<Object>& source, std::string_view moveType);
        virtual void atPostMove(const std::shared_ptr<Object>& source, std::string_view moveType);

        virtual const std::shared_ptr<Object>& getLocation() const;
        virtual const std::shared_ptr<Object>& getDestination() const;
        virtual const std::array<double, 3>& getCoordinates() const;

    protected:

        void addToContents(ObjectID id);
        void addToContents(const std::shared_ptr<Object>& obj);

        void removeFromContents(ObjectID id);
        void removeFromContents(const std::shared_ptr<Object>& obj);
    private:
        ObjectID id{0};
        std::array<double, 3> coordinates{};
        std::shared_ptr<Object> location{}, destination{};
        std::list<std::shared_ptr<Object>> contents{};
        std::unordered_map<std::string, std::string_view> strings{};

        // 0 is no session, 1 is main puppet, 2 is sub puppet
        int sessionMode{0};
        std::shared_ptr<Session> session{};

        std::optional<std::string_view> equippedAt{};

    };

    extern std::unordered_map<ObjectID, std::shared_ptr<Object>> objects;

    struct ObjectEntry {
        std::string objType, subType;
        std::function<std::shared_ptr<Object>(ObjectID)> ctor;
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, ObjectEntry>> objectRegistry;

    OpResult registerObject(ObjectEntry entry);

    std::pair<std::shared_ptr<Object>, std::optional<std::string_view>> createObject(std::string_view objType, std::string_view subType, std::optional<ObjectID> id = std::nullopt);

}