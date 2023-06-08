#include "kaizermud/Components.h"
#include "kaizermud/utils.h"

namespace kaizer::components {

    OpResult<> Account::setPassword(std::string_view newPassword) {
        auto [res, err] = hashPassword(newPassword);
        if(!res) {
            return {false, err};
        }

        password = err.value();
        return {true, std::nullopt};
    }

    OpResult<> Account::checkPassword(std::string_view check) {
        return ::kaizer::checkPassword(password, check);
    }

    nlohmann::json Account::serialize() {
        nlohmann::json j;

        j["username"] = username;
        j["password"] = password;
        if(!email.empty()) j["email"] = email;
        j["created"] = std::chrono::duration_cast<std::chrono::seconds>(created.time_since_epoch()).count();
        j["lastLogin"] = std::chrono::duration_cast<std::chrono::seconds>(lastLogin.time_since_epoch()).count();
        j["lastLogout"] = std::chrono::duration_cast<std::chrono::seconds>(lastLogout.time_since_epoch()).count();
        j["lastPasswordChanged"] = std::chrono::duration_cast<std::chrono::seconds>(lastPasswordChanged.time_since_epoch()).count();
        if(!disabledReason.empty()) j["disabledReason"] = disabledReason;
        if(totalPlayTime > 0) j["totalPlayTime"] = totalPlayTime;
        if(totalLoginTime > 0) j["totalLoginTime"] = totalLoginTime;
        if(!characters.empty()) j["characters"] = characters;
        if(level) j["level"] = level;

        return j;
    }

    void Account::deserialize(const nlohmann::json& j) {
        if(j.count("username")) username = j["username"];
        if(j.count("password")) password = j["password"];
        if(j.count("email")) email = j["email"];
        if(j.count("created")) created = std::chrono::system_clock::from_time_t(j["created"]);
        if(j.count("lastLogin")) lastLogin = std::chrono::system_clock::from_time_t(j["lastLogin"]);
        if(j.count("lastLogout")) lastLogout = std::chrono::system_clock::from_time_t(j["lastLogout"]);
        if(j.count("lastPasswordChanged")) lastPasswordChanged = std::chrono::system_clock::from_time_t(j["lastPasswordChanged"]);
        if(j.count("disabledReason")) disabledReason = j["disabledReason"];
        if(j.count("totalPlayTime")) totalPlayTime = j["totalPlayTime"];
        if(j.count("totalLoginTime")) totalLoginTime = j["totalLoginTime"];
        if(j.count("characters")) {
            for(auto& character : j["characters"]) {
                characters.insert(character.get<ObjectID>());
            }
        }
        if(j.count("level")) level = j["level"];
    }

    Account::Account(const nlohmann::json &j) {
        deserialize(j);
    }
}