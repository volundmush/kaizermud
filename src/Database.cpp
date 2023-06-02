#include "kaizermud/Database.h"
#include "kaizermud/Components.h"
#include "kaizermud/Api.h"
#include "kaizermud/Types.h"
#include "kaizermud/utils.h"
#include <iostream>

namespace kaizer {
    std::unordered_map<std::string, std::shared_ptr<SQLite::Database>> extraDB;
    std::vector<std::function<void(const std::shared_ptr<SQLite::Database>&)>> preLoadFuncs, postLoadFuncs, preSaveFuncs, postSaveFuncs;

    std::vector<std::function<void(entt::entity, nlohmann::json&, bool)>> serializeFuncs;
    std::vector<std::function<void(entt::entity, const nlohmann::json&)>> deserializeFuncs;

    std::vector<std::string> schema = {
            "CREATE TABLE IF NOT EXISTS objects (\n"
            "    id INTEGER PRIMARY KEY,"
            "    data TEXT NOT NULL"
            ");",

            "CREATE TABLE IF NOT EXISTS prototypes ("
            "   id INTEGER PRIMARY KEY,"
            "   name TEXT NOT NULL UNIQUE COLLATE NOCASE,"
            "   data TEXT NOT NULL"
            ");"
    };

    void runQuery(const std::shared_ptr<SQLite::Database>& db, std::string_view query) {
        try {
            db->exec(query.data());
        }
        catch (const std::exception& e) {
            std::cerr << "Error executing query: " << e.what() << std::endl;
            std::cerr << "For statement: " << query << std::endl;
            exit(1);
        }
    }

    void createSchema(const std::shared_ptr<SQLite::Database>& db) {
        for (const auto& query: schema) {
            runQuery(db, query);
        }
    }

    nlohmann::json serializeObject(entt::entity ent, bool asPrototype) {
        nlohmann::json j;
        auto &objinfo = registry.get<components::ObjectInfo>(ent);
        for(const auto&[key, type] : objinfo.types) {
            j["types"].push_back(key);
        }

        if(!asPrototype) {
            auto account = registry.try_get<components::Account>(ent);
            if(account) {
                j["account"] = account->serialize();
            }
        }

        auto aspects = registry.try_get<components::Aspects>(ent);
        if(aspects) {
            for(auto &[key, asp] : aspects->data)
                j["aspects"][key] = asp->getKey();
        }

        auto quirks = registry.try_get<components::Quirks>(ent);
        if(quirks) {
            for(auto &[slot, qmap] : quirks->data)
                for(auto &[key, quirk] : qmap)
                    j["quirks"][slot].push_back(key);
        }

        auto strings = registry.try_get<components::Strings>(ent);
        if(strings) {
            for(auto &[key, value] : strings->data)
                j["strings"][key] = value;
        }

        auto ints = registry.try_get<components::Integers>(ent);
        if(ints) {
            for(auto &[key, value] : ints->data)
                j["integers"][key] = value;
        }

        auto doubles = registry.try_get<components::Doubles>(ent);
        if(doubles) {
            for(auto &[key, value] : doubles->data)
                j["doubles"][key] = value;
        }

        auto stats = registry.try_get<components::Stats>(ent);
        if(stats) {
            for(auto &[key, value] : stats->data)
                j["stats"][key] = value;
        }

        if(!asPrototype) {
            auto eq = registry.try_get<components::Equipment>(ent);
            if(eq) {
                for(auto &[slot, item] : eq->data)
                    j["equipment"][slot] = getID(item);
            }

            auto rel = registry.try_get<components::Relations>(ent);
            if(rel) {
                for(auto &[key, value] : rel->data)
                    j["relations"][key] = getID(value);
            }
        }

        for(auto &func : serializeFuncs)
            func(ent, j, asPrototype);

        return j;

    }

    void saveToDB(entt::entity ent, const std::shared_ptr<SQLite::Database>& db) {
        SQLite::Statement q1(*db, "INSERT INTO objects (id, data) VALUES (?,?)");

        auto data = serializeObject(ent, false);

        q1.bind(1, getID(ent));
        q1.bind(2, data.dump());
        q1.exec();
    }

    void deserializeObject(entt::entity ent, const nlohmann::json& j) {

        if(j.count("types")) {
            auto &objinfo = registry.emplace<components::ObjectInfo>(ent);
            for(const auto& type : j["types"]) {
                auto found = typeRegistry.find(type.get<std::string>());
                if(found != typeRegistry.end()) {
                    objinfo.types[std::string(found->second->getKey())] = found->second.get();
                }
            }
            objinfo.doSort();
        }

        if(j.count("strings")) {
            auto &strings = registry.emplace<components::Strings>(ent);
            for(auto &[key, value] : j["strings"].items())
                strings.data[key] = intern(value.get<std::string>());
        }

        if(j.count("integers")) {
            auto &ints = registry.emplace<components::Integers>(ent);
            for(auto &[key, value] : j["integers"].items())
                ints.data[key] = value.get<int>();
        }

        if(j.count("doubles")) {
            auto &doubles = registry.emplace<components::Doubles>(ent);
            for(auto &[key, value] : j["doubles"].items())
                doubles.data[key] = value.get<double>();
        }

        if(j.count("stats")) {
            auto &stats = registry.emplace<components::Stats>(ent);
            for(auto &[key, value] : j["stats"].items())
                stats.data[key] = value.get<double>();
        }

        if(j.count("aspects")) {
            auto &aspects = registry.emplace<components::Aspects>(ent);
            for(auto &[slot, key] : j["aspects"].items()) {
                auto foundslot = aspectRegistry.find(slot);
                auto asp = foundslot->second;
                auto k = key.get<std::string>();
                auto found = asp.find(k);
                if(found != asp.end()) {
                    aspects.data[k] = found->second.get();
                }
            }
        }

        if(j.count("quirks")) {
            auto &quirks = registry.emplace<components::Quirks>(ent);
            for(auto &[slot, keys] : j["quirks"].items()) {
                auto foundslot = quirkRegistry.find(slot);
                auto quirkmap = foundslot->second;
                for(auto &key : keys) {
                    auto found = quirkmap.find(key.get<std::string>());
                    if(found != quirkmap.end()) {
                        quirks.data[slot][key.get<std::string>()] = found->second.get();
                    }
                }
            }
        }

        if(j.count("equipment")) {
            auto &eq = registry.emplace<components::Equipment>(ent);
            for(auto &[slot, id] : j["equipment"].items()) {
                auto it = entities.find(id.get<int>());
                if(it != entities.end()) {
                    eq.data[slot] = it->second;
                }
            }
        }

        if(j.count("relations")) {
            for(auto &[key, id] : j["relations"].items()) {
                auto it = entities.find(id.get<int>());
                setRelation(ent, key, it->second);
            }
        }

        for(auto &func : deserializeFuncs)
            func(ent, j);

    }

    void loadFromDB(entt::entity ent, const std::shared_ptr<SQLite::Database>& db) {
        // Let's load the components we saved in saveToDB. But, we only want to create
        // components when the saved data exists.

        SQLite::Statement q1(*db, "SELECT data FROM objects WHERE id = ?");
        q1.bind(1, getID(ent));
        q1.executeStep();

        auto data = nlohmann::json::parse(q1.getColumn(0).getString());
        deserializeObject(ent, data);
    }

    void loadDatabase(std::string_view path) {
        auto loadDb = std::make_shared<SQLite::Database>(path.data(), SQLite::OPEN_READONLY);

        for (const auto& func: preLoadFuncs) {
            func(loadDb);
        }

        SQLite::Statement stmt(*loadDb, "SELECT id FROM objects");
        while (stmt.executeStep()) {
            std::optional<ObjectID> id = stmt.getColumn(0).getInt();
            auto [ent, err] = createEntity(id);
        }

        // copy this just in case the load process alters it...
        auto copyObjects = entities;

        for(const auto& [id, ent]: copyObjects) {
            loadFromDB(ent, loadDb);
        }

        for (const auto& func: postLoadFuncs) {
            func(loadDb);
        }
    }

    void saveDatabase(std::string_view path) {
        auto saveDb = std::make_shared<SQLite::Database>(path.data(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        SQLite::Transaction transaction(*saveDb);

        createSchema(saveDb);

        for (const auto& func: preSaveFuncs) {
            func(saveDb);
        }

        for (const auto& [id, ent]: entities) {
            saveToDB(ent, saveDb);
        }

        for (const auto& func: postSaveFuncs) {
            func(saveDb);
        }

        transaction.commit();
    }
}