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
#include "lualib.h"
#include "luacode.h"
#include "LuaBridge/LuaBridge.h"
#include "kaizermud/Stats.h"
#include "kaizermud/Aspects.h"

namespace kaizermud::game {

    class Object {
    public:
        explicit Object(ObjectID id);
        virtual ~Object() = default;

        // Retrieve the Object's unique ID. IDs should never be re-used.
        [[nodiscard]] ObjectID getId() const;

        // All Objects have a base Type and, possibly, a Sub-type.
        // These together determine which class this is identified as, what
        // stats to give it, which commands it can access, and so on.
        [[nodiscard]] virtual std::string_view getType() const = 0;
        [[nodiscard]] virtual std::string_view getSubType() const;

        // This returns the above two in order by default, but can be used to
        // specify additional subtypes to load data from for stats, commands, etc.
        // It's best for this to not change once the object is created and loaded.
        [[nodiscard]] virtual std::list<std::string_view> getTypes() const;

        // All Objects can support a connection to a Session, its link to a player.
        // Sessions themselves might be linked to an arbitrary amount of network connections.
        // Ideally, Anything, not just a Player Character, can be directly puppeted.
        // Not sure what use it is to puppet a room, but whatever.
        [[nodiscard]] const std::shared_ptr<Session>& getSession() const;
        virtual void setSession(const std::shared_ptr<Session>& sess);
        virtual void removeSession();

        std::unique_ptr<StatHandler> statHandler{};
        std::unique_ptr<AspectHandler> aspectHandler{};

        [[nodiscard]] virtual std::list<std::string> getAspectSlots() const;
        [[nodiscard]] virtual std::list<std::string> getStatKeys() const;

        [[nodiscard]] virtual std::string_view getString(std::string_view key) const;
        virtual OpResult setString(std::string_view key, const std::string &value);

        virtual void send(const Message &msg);

    protected:
        ObjectID id{0};
        std::weak_ptr<Object> location{};
        std::optional<std::string_view> equippedAt{};
        std::array<double, 3> coordinates{};
        std::list<std::weak_ptr<Object>> contents{};
        // 0 is no session, 1 is main puppet, 2 is sub puppet
        int sessionMode{0};
        std::shared_ptr<Session> session{};
        std::unordered_map<std::string, std::string_view> strings{};

    };

    extern std::unordered_map<ObjectID, std::shared_ptr<Object>> objects;

    struct ObjectEntry {
        std::string objType, subType;
        std::function<std::shared_ptr<Object>(ObjectID, const std::string&)> ctor;
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, ObjectEntry>> objectRegistry;

    OpResult registerObject(ObjectEntry entry);

}