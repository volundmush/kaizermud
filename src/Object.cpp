#include <memory>

#include "kaizermud/Object.h"
#include "kaizermud/utils.h"
#include "kaizermud/Session.h"

namespace kaizermud::game {

    Object::Object(ObjectID id) {
        this->id = id;
        statHandler = std::make_unique<StatHandler>(this);
        aspectHandler = std::make_unique<AspectHandler>(this);
    }

    ObjectID Object::getId() const {
        return id;
    }

    std::string_view Object::getSubType() const {
        return "_";
    }

    std::list<std::string_view> Object::getTypes() const {
        return {getType(), getSubType()};
    }

    void Object::setSession(const std::shared_ptr<Session>& sess) {
        this->session = sess;
    }

    void Object::removeSession() {
        this->session.reset();
    }

    const std::shared_ptr<Session>& Object::getSession() const {
        return session;
    }

    std::list<std::string> Object::getAspectSlots() const {
        return {};
    }

    std::list<std::string> Object::getStatKeys() const {
        return {};
    }

    std::unordered_map<std::string, std::unordered_map<std::string, ObjectEntry>> objectRegistry;

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

}