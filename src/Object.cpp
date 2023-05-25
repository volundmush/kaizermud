#include <memory>

#include "kaizermud/Object.h"
#include "kaizermud/utils.h"
#include "kaizermud/Session.h"
#include "kaizermud/game.h"
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
        if(session) {
            session->atObjectDeleted(shared_from_this());
        }
        if(location) {
            location->atObjectLeave(shared_from_this(), nullptr, "delete");
            location->removeFromContents(shared_from_this());
        }
    }

    void Object::onLoad() {

    }

    void Object::atServerReload() {

    }

    void Object::atServerShutdown() {

    }

    // Info API Hooks
    bool Object::isSuperuser() {
        return false;
        // TODO: Hook this up to Sessions.
    }

    std::string Object::getDisplayName(std::shared_ptr<Object> &looker, std::string_view type) {
        auto sh = getString("short_description");
        if(!sh.empty()) return std::string(sh);
        auto name = getString("name");
        if(!sh.empty()) return std::string(name);
        return fmt::format("Unnamed Object {}", id);
    }

    std::string Object::renderAppearance(std::shared_ptr<Object> &looker, std::string_view type) {
        auto look = getString("look_description");
        if(!look.empty()) return std::string(look);
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
    bool Object::matchType(std::string_view type) const {
        auto types = getTypes();
        return std::find(types.begin(), types.end(), type) != types.end();
    }

    std::vector<std::shared_ptr<Object>> Object::getContents() {
        std::vector<std::shared_ptr<Object>> out;
        std::copy(contents.begin(), contents.end(), std::back_inserter(out));
        return out;
    }

    std::vector<std::shared_ptr<Object>> Object::getContents(std::string_view type) {
        std::vector<std::shared_ptr<Object>> out;
        std::copy_if(contents.begin(), contents.end(), std::back_inserter(out), [type](const auto& obj) {
            return obj->matchType(type);
        });
        return out;
    }

    // Get
    OpResult Object::doGet(const std::shared_ptr<Object> &target) {
        auto [canGet, reas] = canGetObject(target);
        if(!canGet) {
            return {false, reas};
        }

        auto self = shared_from_this();
        auto [preGet, reason] = target->atPreGet(self);
        if(!preGet) {
            return {false, reason};
        }

        auto [moved, whynot] = target->moveTo(self, true, "get");
        if(!moved) {
            return {false, whynot};
        }

        target->atGet(self);
    }

    OpResult Object::canGetObject(const std::shared_ptr<Object> &target) {
        return {true, std::nullopt};
    }

    OpResult Object::atPreGet(const std::shared_ptr<Object> &getter) {
        return {true, std::nullopt};
    }

    void Object::atGet(const std::shared_ptr<Object> &getter) {

    }

    // Drop

    OpResult Object::doDrop(const std::shared_ptr<Object> &target) {
        auto [canDrop, reas] = canDropObject(target);
        if(!canDrop) {
            return {false, reas};
        }

        auto self = shared_from_this();
        auto [preDrop, reason] = target->atPreDrop(self);
        if(!preDrop) {
            return {false, reason};
        }

        auto [moved, whynot] = target->moveTo(location, true, "drop");
        if(!moved) {
            return {false, whynot};
        }

        target->atDrop(self);
    }

    OpResult Object::canDropObject(const std::shared_ptr<Object> &target) {
        return {true, std::nullopt};
    }

    OpResult Object::atPreDrop(const std::shared_ptr<Object> &dropper) {
        return {true, std::nullopt};
    }

    void Object::atDrop(const std::shared_ptr<Object> &dropper) {

    }

    // Give
    OpResult Object::doGive(const std::shared_ptr<Object> &target, const std::shared_ptr<Object> &recipient) {
        auto [canGive, reas] = canGiveObject(target, recipient);
        if(!canGive) {
            return {false, reas};
        }

        auto self = shared_from_this();
        auto [preGive, reason] = target->atPreGive(self, recipient);
        if(!preGive) {
            return {false, reason};
        }

        auto [moved, whynot] = target->moveTo(recipient, true, "give");
        if(!moved) {
            return {false, whynot};
        }

        target->atGive(self, recipient);
    }

    OpResult Object::canGiveObject(const std::shared_ptr<Object> &target, const std::shared_ptr<Object> &recipient) {
        return {true, std::nullopt};
    }

    OpResult Object::atPreGive(const std::shared_ptr<Object> &giver, const std::shared_ptr<Object> &recipient) {
        return {true, std::nullopt};
    }

    void Object::atGive(const std::shared_ptr<Object> &giver, const std::shared_ptr<Object> &recipient) {

    }

    // Put

OpResult Object::doPut(const std::shared_ptr<Object> &target, const std::shared_ptr<Object> &container) {
        auto [canPut, reas] = canPutObject(target, container);
        if(!canPut) {
            return {false, reas};
        }

        auto self = shared_from_this();
        auto [prePut, reason] = target->atPrePut(self, container);
        if(!prePut) {
            return {false, reason};
        }

        auto [moved, whynot] = target->moveTo(container, true, "put");
        if(!moved) {
            return {false, whynot};
        }

        target->atPut(self, container);
    }

    OpResult Object::canPutObject(const std::shared_ptr<Object> &target, const std::shared_ptr<Object> &container) {
        return {true, std::nullopt};
    }

    OpResult Object::atPrePut(const std::shared_ptr<Object> &putter, const std::shared_ptr<Object> &container) {
        return {true, std::nullopt};
    }

    void Object::atPut(const std::shared_ptr<Object> &putter, const std::shared_ptr<Object> &container) {

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
    OpResult Object::moveTo(kaizermud::ObjectID dest, bool quiet, std::string_view moveType) {
        auto it = objects.find(dest);
        if(it == objects.end()) {
            return {false, "Destination not found"};
        }
        return moveTo(it->second, quiet, moveType);
    }

    OpResult Object::moveTo(const std::shared_ptr<Object> &dest, bool quiet, std::string_view moveType) {

        auto [canMove, reason] = atPreMove(dest, moveType);
        if(!canMove) {
            return {false, reason};
        }

        auto self = shared_from_this();

        auto loc = location;
        if(loc) {
            auto [canLeave, reason] = loc->atPreObjectLeave(self, dest, moveType);
            if(!canLeave) {
                return {false, reason};
            }
        }

        if(dest) {
            if (dest->id == id) {
                return {false, "You cannot move into yourself! The universe might implode!"};
            }
            auto d = dest;
            while(true) {
                auto l = d->location;
                if(!l) break;
                if(l->id == id) {
                    return {false, "That would cause circular references! The universe might implode!"};
                }
                d = l;
            }


            auto [canEnter, reason] = dest->atPreObjectReceive(self, loc, moveType);
            if(!canEnter) {
                return {false, reason};
            }
        }

        if(loc) {
            loc->atObjectLeave(self, dest, moveType);
            if(!quiet) announceMoveFrom(loc, dest, moveType);
            loc->removeFromContents(self);
        }

        if(dest) {
            dest->atObjectReceive(self, loc, moveType);
            if(!quiet) announceMoveTo(loc, dest, moveType);
            dest->addToContents(self);
        }

        atPostMove(loc, moveType);
        return {true, std::nullopt};
    }

    OpResult Object::atPreMove(const std::shared_ptr<Object> &dest, std::string_view moveType) {
        return {true, std::nullopt};
    }

    OpResult Object::atPreObjectLeave(const std::shared_ptr<Object> &obj, const std::shared_ptr<Object> &dest,
                                      std::string_view moveType) {
        return {true, std::nullopt};
    }

    OpResult Object::atPreObjectReceive(const std::shared_ptr<Object>& obj, const std::shared_ptr<Object>& source, std::string_view moveType) {
        return {true, std::nullopt};
    }

    void Object::atObjectLeave(const std::shared_ptr<Object> &obj, const std::shared_ptr<Object> &dest,
                                   std::string_view moveType) {

    }

    void Object::announceMoveFrom(const std::shared_ptr<Object> &source, const std::shared_ptr<Object> &dest,
                                  std::string_view moveType) {

    }

    void Object::announceMoveTo(const std::shared_ptr<Object> &source, const std::shared_ptr<Object> &dest,
                                std::string_view moveType) {

    }

    void Object::atObjectReceive(const std::shared_ptr<Object>& obj, const std::shared_ptr<Object>& source, std::string_view moveType) {

    }

    void Object::atPostMove(const std::shared_ptr<Object> &loc, std::string_view moveType) {
    }

    const std::shared_ptr<Object>& Object::getLocation() const {
        return location;
    }

    const std::shared_ptr<Object>& Object::getDestination() const {
        return destination;
    }

    const std::array<double, 3>& Object::getCoordinates() const {
        return coordinates;
    }


    void Object::addToContents(kaizermud::ObjectID id) {
        auto it = objects.find(id);
        if(it != objects.end()) {
            addToContents(it->second);
        }
    }

    void Object::addToContents(const std::shared_ptr<Object> &obj) {
        contents.push_back(obj);
        obj->location = shared_from_this();
    }

    void Object::removeFromContents(kaizermud::ObjectID id) {
        auto it = objects.find(id);
        if(it != objects.end()) {
            removeFromContents(it->second);
        }
    }

    void Object::removeFromContents(const std::shared_ptr<Object> &obj) {
        auto it = std::find(contents.begin(), contents.end(), obj);
        if(it != contents.end()) {
            contents.erase(it);
            obj->location.reset();
        }
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

    std::string_view Object::getString(std::string_view key) const {
        auto it = strings.find(std::string(key));
        if(it != strings.end())
            return it->second;
        return "";
    }

    OpResult Object::setString(std::string_view key, const std::string &value) {
        strings[std::string(key)] = utils::intern(value);
        return {true, std::nullopt};
    }

    void Object::send(const Message &msg) {
        if(session)
            session->send(msg);
    }

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