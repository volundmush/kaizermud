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
#include "SQLiteCpp/SQLiteCpp.h"
#include "kaizermud/CallParameters.h"

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

        virtual void send(const Message &msg);

        // Administration Hooks
        virtual void atCreate();
        virtual void atDelete();
        virtual void onLoad();
        virtual void atServerReload();
        virtual void atServerShutdown();
        virtual void loadFromDB(const std::shared_ptr<SQLite::Database>& db);
        virtual void saveToDB(const std::shared_ptr<SQLite::Database>& db);

        // Info API Hooks
        [[nodiscard]] virtual std::string getDisplayName(std::shared_ptr<Object>& looker, std::string_view type = "normal");
        [[nodiscard]] virtual std::string renderAppearance(std::shared_ptr<Object>& looker, std::string_view type = "normal");
        [[nodiscard]] virtual std::string atLook(std::shared_ptr<Object>& target);
        virtual void atDesc(const std::shared_ptr<Object>& looker);

        // Inventory API Hooks
        [[nodiscard]] virtual std::vector<std::shared_ptr<Object>> getContents();
        [[nodiscard]] virtual std::vector<std::shared_ptr<Object>> getContents(std::string_view type);

        virtual void atEquipItem(const std::shared_ptr<Object>& item, std::string_view slot);
        virtual void atEquipped(const std::shared_ptr<Object>& user, std::string_view slot);
        virtual void atRemoveItem(const std::shared_ptr<Object>& item, std::string_view slot);
        virtual void atUnequip(const std::shared_ptr<Object>& user, std::string_view slot);

        // Interaction and Filtering Hooks
        virtual bool canDetect(const std::shared_ptr<Object>& target, std::string_view detectType = "see");
        virtual std::vector<std::shared_ptr<Object>> filterDetect(std::vector<std::shared_ptr<Object>>& objects, std::string_view detectType = "see");

        // Movement and Location API hooks.
        [[nodiscard]] virtual OpResult moveTo(CallParameters& param);
        [[nodiscard]] virtual OpResult atPreMove(const CallParameters& param);
        [[nodiscard]] virtual OpResult atPreObjectLeave(const CallParameters& param);
        [[nodiscard]] virtual OpResult atPreObjectReceive(const CallParameters& param);
        virtual void atObjectLeave(const CallParameters& param);
        virtual void announceMoveFrom(const CallParameters& param);
        virtual void announceMoveTo(const CallParameters& param);
        virtual void atObjectReceive(const CallParameters& param);
        virtual void atPostMove(const CallParameters& param);

        virtual const std::array<double, 3>& getCoordinates() const;

        // Relations
        [[nodiscard]] virtual OpResult setRelation(std::string_view name, const std::shared_ptr<Object>& val);
        [[nodiscard]] virtual std::shared_ptr<Object> getRelation(std::string_view name);
        virtual OpResult clearRelation(std::string_view name);
        virtual std::optional<std::reference_wrapper<const std::vector<std::shared_ptr<Object>>>> getReverseRelation(std::string_view name);


        // Strings
        [[nodiscard]] virtual OpResult setString(std::string_view key, std::string_view value);
        [[nodiscard]] virtual OpResult clearString(std::string_view key);
        [[nodiscard]] virtual std::optional<std::string_view> getString(std::string_view key) const;

        // Permission check system framework.
        [[nodiscard]] virtual bool isSuperuser();
        [[nodiscard]] virtual bool checkPermission(const std::shared_ptr<Object>& actor, std::string_view permission);

    protected:
        ObjectID id{0};
        std::unique_ptr<StatHandler> statHandler{};
        std::unique_ptr<AspectHandler> aspectHandler{};
        std::unique_ptr<EquipHandler> equipHandler{};

        std::unordered_map<std::string, std::string_view> strings{};
        std::unordered_map<std::string, std::shared_ptr<Object>> relations{};
        std::unordered_map<std::string, std::vector<std::shared_ptr<Object>>> reverseRelations{};

        std::array<double, 3> coordinates{};

        // 0 is no session, 1 is main puppet, 2 is sub puppet
        int sessionMode{0};
        std::shared_ptr<Session> session{};

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