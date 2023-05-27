#include <memory>

#include "kaizermud/Object.h"
#include "kaizermud/utils.h"
#include "kaizermud/Session.h"
#include "kaizermud/game.h"
#include "kaizermud/Database.h"
#include "fmt/format.h"

namespace kaizermud::game {


    Object::Object(ObjectID id) {
        this->id = id;
    }

    ObjectID Object::getId() const {
        return id;
    }

    std::string_view Object::getMainType() const {
        return "basic";
    }

    std::string_view Object::getSubType() const {
        return "basic";
    }

    std::vector<std::string_view> Object::getTypes() const {
        return {getMainType(), getSubType()};
    }

    void Object::setSession(const std::shared_ptr<Session>& sess) {
        onPreSessionLink(sess);
        this->session = sess;
        onPostSessionLink(sess);
    }

    void Object::onPreSessionLink(const std::shared_ptr<Session> &sess) {

    }

    void Object::onPostSessionLink(const std::shared_ptr<Session> &sess) {

    }

    void Object::removeSession() {
        if(!session) return;
        auto sess = session;
        onPreSessionUnlink(sess);
        this->session.reset();
        onPostSessionUnlink(sess);
    }

    void Object::onPreSessionUnlink(const std::shared_ptr<Session> &sess) {

    }

    void Object::onPostSessionUnlink(const std::shared_ptr<Session> &sess) {

    }

    const std::shared_ptr<Session>& Object::getSession() const {
        return session;
    }

    // Administration API Hooks
    void Object::atCreate() {
        auto self = shared_from_this();
        statHandler = std::make_unique<StatHandler>(self);
        aspectHandler = std::make_unique<AspectHandler>(self);
        equipHandler = std::make_unique<EquipHandler>(self);
    }

    void Object::atDelete() {
        auto self = shared_from_this();
        if(session) {
            session->atObjectDeleted(self);
        }
        auto loc = getRelation("location");
        if(loc) {
            auto param = CallParameters().setString("movetype", "delete").setBool("quiet", true);
            loc->atObjectLeave(param);
            clearRelation("location");
        }
    }

    void Object::onLoad() {

    }

    void Object::atServerReload() {

    }

    void Object::atServerShutdown() {

    }

    void Object::loadFromDB(const std::shared_ptr<SQLite::Database> &db) {
        // By this point we have already been instantiated with our ID so...

        // Load our strings
        SQLite::Statement q1(*db, "SELECT key, value FROM objectStrings WHERE objectId = ?;");
        q1.bind(1, getId());

        while(q1.executeStep()) {
            auto key = q1.getColumn(0).getString();
            auto val = q1.getColumn(1).getString();
            strings[key] = utils::intern(val);
        }

        // Load our relations
        SQLite::Statement q2(*db, "SELECT relationType, relationId FROM objectRelations WHERE objectId = ?;");
        q2.bind(1, getId());

        while(q2.executeStep()) {
            auto type = q2.getColumn(0).getString();
            auto relID = q2.getColumn(1).getInt64();

            auto it = objects.find(relID);
            if(it == objects.end()) {
                continue;
            }
            setRelation(type, it->second);
        }

        // Load our stats
        statHandler->loadFromDB(db);
        equipHandler->loadFromDB(db);
        aspectHandler->loadFromDB(db);

    }

    void Object::saveToDB(const std::shared_ptr<SQLite::Database> &db) {
        SQLite::Statement q1(*db, "INSERT INTO objects (id, mainType, subType) VALUES (?, ?, ?);");

        q1.bind(1, getId());
        q1.bind(2, std::string(getMainType()));
        q1.bind(3, std::string(getSubType()));
        q1.exec();

        SQLite::Statement q2(*db, "INSERT INTO objectStrings (objectId, key, value) VALUES (?, ?, ?);");
        for(auto &it : strings) {
            q2.bind(1, getId());
            q2.bind(2, it.first);
            q2.bind(3, std::string(it.second));
            q2.exec();
            q2.reset();
        }

        SQLite::Statement q3(*db, "INSERT INTO objectRelations (objectId, relationType, relationId) VALUES (?, ?, ?);");
        for(auto &it : relations) {
            q3.bind(1, getId());
            q3.bind(2, it.first);
            q3.bind(3, it.second->getId());
            q3.exec();
            q3.reset();
        }

        statHandler->saveToDB(db);
        aspectHandler->saveToDB(db);
        equipHandler->saveToDB(db);

    }

    // Info API Hooks
    std::string Object::getDisplayName(std::shared_ptr<Object> &looker, std::string_view type) {
        auto short_description = getString("short_description");
        if(short_description.has_value() && !short_description.value().empty()) return std::string(short_description.value());
        auto name = getString("name");
        if(name.has_value() && !name.value().empty()) return std::string(name.value());
        return fmt::format("Unnamed Object {}", id);
    }

    std::string Object::renderAppearance(std::shared_ptr<Object> &looker, std::string_view type) {
        auto look_description = getString("look_description");
        if(look_description.has_value() && !look_description.value().empty()) return std::string(look_description.value());
        return "You see nothing particularly special...";
    }

    std::string Object::atLook(std::shared_ptr<Object> &target) {
        if(!target) {
            return "Nothing to look at!";
        }
        auto self = shared_from_this();
        auto desc = target->renderAppearance(self);
        target->atDesc(self);
        return desc;
    }

    void Object::atDesc(const std::shared_ptr<Object> &looker) {

    }


    // Inventory API Hooks
    void Object::atEquipItem(const std::shared_ptr<Object> &item, std::string_view slot) {

    }

    void Object::atEquipped(const std::shared_ptr<Object> &user, std::string_view slot) {
        setString("equippedAt", slot);
    }

    void Object::atRemoveItem(const std::shared_ptr<Object> &item, std::string_view slot) {

    }

    void Object::atUnequip(const std::shared_ptr<Object> &user, std::string_view slot) {
        clearString("equippedAt");
    }

    bool Object::matchType(std::string_view type) const {
        auto types = getTypes();
        return std::find(types.begin(), types.end(), type) != types.end();
    }

    std::vector<std::shared_ptr<Object>> Object::getContents() {
        auto contents = getReverseRelation("location");
        if(!contents.has_value()) return {};
        return contents.value().get();
    }

    std::vector<std::shared_ptr<Object>> Object::getContents(std::string_view type) {
        std::vector<std::shared_ptr<Object>> out;
        auto contents = getReverseRelation("location");
        if(!contents.has_value()) return out;
        std::copy_if(contents.value().get().begin(), contents.value().get().end(), std::back_inserter(out), [type](const auto& obj) {
            return obj->matchType(type);
        });
        return out;
    }


    // Interaction and Filtering
    bool Object::canDetect(const std::shared_ptr<Object> &target, std::string_view detectType) {
        return true;
    }

    std::vector<std::shared_ptr<Object>> Object::filterDetect(std::vector<std::shared_ptr<Object>> &objects, std::string_view detectType) {
        std::vector<std::shared_ptr<Object>> out;
        std::copy_if(objects.begin(), objects.end(), std::back_inserter(out), [this, detectType](const auto& obj) {
            return canDetect(obj, detectType);
        });
        return out;
    }

    // Movement API Hooks
    OpResult Object::moveTo(CallParameters& param) {

        auto self = shared_from_this();
        auto loc = getRelation("location");
        if(loc) param.setObject("origin", loc);
        param.setObject("mover", self);

        auto [canMove, reas] = atPreMove(param);
        if(!canMove) {
            return {false, reas};
        }


        if(loc) {
            auto [canLeave, reason] = loc->atPreObjectLeave(param);
            if(!canLeave) {
                return {false, reason};
            }
        }

        auto dest = param.getObject("destination");

        if(dest) {
            if (dest->id == id) {
                return {false, "You cannot move into yourself! The universe might implode!"};
            }
            auto d = dest;
            while(true) {
                auto l = d->getRelation("location");
                if(!l) break;
                if(l->id == id) {
                    return {false, "That would cause circular references! The universe might implode!"};
                }
                d = l;
            }


            auto [canEnter, reason] = dest->atPreObjectReceive(param);
            if(!canEnter) {
                return {false, reason};
            }
        }

        auto quiet = param.getBool("quiet");

        if(loc) {
            loc->atObjectLeave(param);
            if(!quiet) announceMoveFrom(param);
            clearRelation("location");
        }

        if(dest) {
            setRelation("location", dest);
            dest->atObjectReceive(param);
            if(!quiet) announceMoveTo(param);
        }

        atPostMove(param);
        return {true, std::nullopt};
    }

    OpResult Object::atPreMove(const CallParameters& param) {
        return {true, std::nullopt};
    }

    OpResult Object::atPreObjectLeave(const CallParameters& param) {
        return {true, std::nullopt};
    }

    OpResult Object::atPreObjectReceive(const CallParameters& param) {
        return {true, std::nullopt};
    }

    void Object::atObjectLeave(const CallParameters& param) {

    }

    void Object::announceMoveFrom(const CallParameters& param) {

    }

    void Object::announceMoveTo(const CallParameters& param) {

    }

    void Object::atObjectReceive(const CallParameters& param) {

    }

    void Object::atPostMove(const CallParameters& param) {
    }

    const std::array<double, 3>& Object::getCoordinates() const {
        return coordinates;
    }

    OpResult Object::setRelation(std::string_view name, const std::shared_ptr<Object> &val) {
        if(!val) {
            return clearRelation(name);
        }
        auto key = std::string(name);
        auto rel = relations.find(key);
        if(rel != relations.end()) {
            auto result = clearRelation(name);
            if(!result.first) return result;
        }
        relations[key] = val;
        auto &reverse = val->reverseRelations;
        auto rev = reverse.find(key);
        if(rev == reverse.end()) {
            auto &rel2 = reverse[key];
            rel2.push_back(shared_from_this());
        } else {
            rev->second.push_back(shared_from_this());
        }
        return {true, std::nullopt};
    }

    OpResult Object::clearRelation(std::string_view name) {
        auto key = std::string(name);
        auto rel = relations.find(key);
        if(rel == relations.end()) {
            return {true, std::nullopt};
        }
        auto relation = rel->second;
        relations.erase(rel);
        auto &reverse = relation->reverseRelations;
        auto rev = reverse.find(key);
        if(rev != reverse.end()) {
            auto ent = std::find(rev->second.begin(), rev->second.end(), shared_from_this());
            rev->second.erase(ent);
        }
    }

    std::shared_ptr<Object> Object::getRelation(std::string_view name) {
        auto key = std::string(name);
        auto it = relations.find(key);
        if(it != relations.end()) return it->second;
        return nullptr;
    }

    std::optional<std::reference_wrapper<const std::vector<std::shared_ptr<Object>>>> Object::getReverseRelation(std::string_view name) {
        auto key = std::string(name);
        auto rev = reverseRelations.find(key);
        if(rev != reverseRelations.end()) {
            return rev->second;
        }
        return std::nullopt;
    }


    OpResult Object::setString(std::string_view key, std::string_view value) {
        if(key.empty()) {
            return {false, "Key cannot be empty"};
        }
        if(value.empty()) {
            strings.erase(std::string(key));
            return {true, std::nullopt};
        }
        strings[std::string(key)] = utils::intern(value);
        return {true, std::nullopt};
    }

    OpResult Object::clearString(std::string_view key) {
        if(key.empty()) {
            return {false, "Key cannot be empty"};
        }
        strings.erase(std::string(key));
        return {true, std::nullopt};
    }

    std::optional<std::string_view> Object::getString(std::string_view key) const {
        if(key.empty()) {
            return std::nullopt;
        }
        auto it = strings.find(std::string(key));
        if(it == strings.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    // utility stuff...

    std::unordered_map<std::string, std::unordered_map<std::string, ObjectEntry>> objectRegistry;

    std::unordered_map<ObjectID, std::shared_ptr<Object>> objects;

    OpResult registerObject(ObjectEntry entry) {
        if(entry.objType.empty()) {
            return {false, "Object type cannot be empty"};
        }
        if(entry.subType.empty()) {
            return {false, "Object sub-type cannot be empty"};
        }
        if(entry.ctor == nullptr) {
            return {false, "Object constructor cannot be null"};
        }

        auto &objreg = objectRegistry[entry.objType];
        objreg[entry.subType] = entry;
        return {true, std::nullopt};
    }

    void Object::send(const Message &msg) {
        if(session)
            session->send(msg);
    }

    // Permission check
    bool Object::isSuperuser() {
        return false;
        // TODO: Hook this up to Sessions.
    }
    bool Object::checkPermission(const std::shared_ptr<Object>& actor, std::string_view permission) {
        return true;
    }

    // End of Object class

    std::pair<std::shared_ptr<Object>, std::optional<std::string_view>> createObject(std::string_view objType, std::string_view subType, std::optional<ObjectID> id) {
        auto it = objectRegistry.find(std::string(objType));
        if(it == objectRegistry.end()) {
            return {nullptr, "Object type not found"};
        }

        auto &objreg = it->second;
        auto it2 = objreg.find(std::string(subType));
        if(it2 == objreg.end()) {
            return {nullptr, "Object sub-type not found"};
        }

        auto &entry = it2->second;

        ObjectID newID;

        if(id.has_value()) {
            if(objects.count(id.value())) {
                return {nullptr, "Object ID already in use"};
            }
            newID = id.value();
        } else {
            newID = getNextAvailableID();
        }

        auto obj = entry.ctor(newID);
        objects[newID] = obj;
        obj->atCreate();
        return {obj, std::nullopt};
    }
}