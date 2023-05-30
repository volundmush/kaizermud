#include "kaizermud/Database.h"
#include "kaizermud/Components.h"
#include "kaizermud/Api.h"
#include "kaizermud/Types.h"
#include <iostream>

namespace kaizer {
    std::unordered_map<std::string, std::shared_ptr<SQLite::Database>> extraDB;
    std::vector<std::function<void(const std::shared_ptr<SQLite::Database>&)>> preLoadFuncs, postLoadFuncs, preSaveFuncs, postSaveFuncs;

    std::vector<std::string> schema = {
            "CREATE TABLE IF NOT EXISTS objects (\n"
            "    id INTEGER PRIMARY KEY"
            ");",

            "CREATE TABLE IF NOT EXISTS objectTypes ("
            "   objectID INTEGER NOT NULL,"
            "   value TEXT NOT NULL,"
            "   PRIMARY KEY(objectId, value)"
            ");",

            "CREATE TABLE IF NOT EXISTS objectStrings (\n"
            "    objectId INTEGER NOT NULL,\n"
            "    key TEXT NOT NULL,\n"
            "    value TEXT NOT NULL,\n"
            "    PRIMARY KEY(objectId, key)\n"
            ");",

            "CREATE TABLE IF NOT EXISTS objectIntegers (\n"
            "    objectId INTEGER NOT NULL,\n"
            "    key TEXT NOT NULL,\n"
            "    value INTEGER NOT NULL,\n"
            "    PRIMARY KEY(objectId, key)\n"
            ");",

            "CREATE TABLE IF NOT EXISTS objectDoubles (\n"
            "    objectId INTEGER NOT NULL,\n"
            "    key TEXT NOT NULL,\n"
            "    value REAL NOT NULL,\n"
            "    PRIMARY KEY(objectId, key)\n"
            ");",

            "CREATE TABLE IF NOT EXISTS objectRelations (\n"
            "    objectId INTEGER NOT NULL,\n"
            "    relationType TEXT NOT NULL,\n"
            "    relationId INTEGER NOT NULL,\n"
            "    PRIMARY KEY(objectId, relationType)\n"
            ");",

            "CREATE TABLE IF NOT EXISTS objectStats (\n"
            "    objectId INTEGER NOT NULL,\n"
            "    statType TEXT NOT NULL,\n"
            "    value REAL NOT NULL,\n"
            "    PRIMARY KEY(objectId, statType)\n"
            ");",

            "CREATE TABLE IF NOT EXISTS objectEquipment (\n"
            "    objectId INTEGER NOT NULL,\n"
            "    slot TEXT NOT NULL,\n"
            "    itemId INTEGER NOT NULL,\n"
            "    PRIMARY KEY(objectId, slot)\n"
            ");"

            "CREATE TABLE IF NOT EXISTS objectAspect ("
            "   objectId INTEGER NOT NULL,"
            "   aspectType TEXT NOT NULL,"
            "   aspectKey TEXT NOT NULL,"
            "   PRIMARY KEY(objectId, aspectType)"
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

    void saveToDB(entt::entity ent, const std::shared_ptr<SQLite::Database>& db) {
        SQLite::Statement q1(*db, "INSERT INTO objects (id) VALUES (?)");


        auto &objinfo = registry.get<components::ObjectInfo>(ent);
        auto id = objinfo.id;
        q1.bind(1, id);
        q1.exec();

        SQLite::Statement q9(*db, "INSERT INTO objectTypes (objectId, value) VALUES (?,?);");
        for(const auto&[key, type] : objinfo.types) {
            q9.bind(1, id);
            q9.bind(2, key);
            q9.exec();
            q9.reset();
        }

        SQLite::Statement q2(*db, "INSERT INTO objectStrings (objectId, key, value) VALUES (?, ?, ?)");
        auto strings = registry.try_get<components::Strings>(ent);
        if (strings) {
            for (const auto& [key, value]: strings->data) {
                if(value.empty()) continue;
                q2.bind(1, objinfo.id);
                q2.bind(2, key);
                q2.bind(3, std::string(value));
                q2.exec();
                q2.reset();
            }
        }

        SQLite::Statement q3(*db, "INSERT INTO objectIntegers (objectId, key, value) VALUES (?, ?, ?)");
        auto ints = registry.try_get<components::Integers>(ent);
        if (ints) {
            for (const auto& [key, value]: ints->data) {
                q3.bind(1, objinfo.id);
                q3.bind(2, key);
                q3.bind(3, value);
                q3.exec();
                q3.reset();
            }
        }

        SQLite::Statement q4(*db, "INSERT INTO objectDoubles (objectId, key, value) VALUES (?, ?, ?)");
        auto doubles = registry.try_get<components::Doubles>(ent);
        if (doubles) {
            for (const auto& [key, value]: doubles->data) {
                q4.bind(1, objinfo.id);
                q4.bind(2, key);
                q4.bind(3, value);
                q4.exec();
                q4.reset();
            }
        }

        SQLite::Statement q5(*db, "INSERT INTO objectRelations (objectId, relationType, relationId) VALUES (?, ?, ?)");
        auto relations = registry.try_get<components::Relations>(ent);
        if (relations) {
            for (const auto& [key, value]: relations->data) {
                q5.bind(1, objinfo.id);
                q5.bind(2, key);
                q5.bind(3, getID(ent));
                q5.exec();
                q5.reset();
            }
        }

        SQLite::Statement q6(*db, "INSERT INTO objectStats (objectId, statType, value) VALUES (?, ?, ?)");
        auto stats = registry.try_get<components::Stats>(ent);
        if (stats) {
            for (const auto& [key, value]: stats->data) {
                q6.bind(1, objinfo.id);
                q6.bind(2, key);
                q6.bind(3, value);
                q6.exec();
                q6.reset();
            }
        }

        SQLite::Statement q7(*db, "INSERT INTO objectEquipment (objectId, slot, itemId) VALUES (?, ?, ?)");
        auto equip = registry.try_get<components::Equipment>(ent);
        if (equip) {
            for (const auto& [key, value]: equip->data) {
                q7.bind(1, objinfo.id);
                q7.bind(2, key);
                q7.bind(3, getID(value));
                q7.exec();
                q7.reset();
            }
        }

        SQLite::Statement q8(*db, "INSERT INTO objectAspect (objectId, aspectType, aspectKey) VALUES (?, ?, ?)");
        auto aspects = registry.try_get<components::Aspects>(ent);
        if (aspects) {
            for (const auto& [key, value]: aspects->data) {
                q8.bind(1, objinfo.id);
                q8.bind(2, key);
                q8.bind(3, std::string(value->getKey()));
                q8.exec();
                q8.reset();
            }
        }

    }

    void loadFromDB(entt::entity ent, const std::shared_ptr<SQLite::Database>& db) {
        // Let's load the components we saved in saveToDB. But, we only want to create
        // components when the saved data exists.

        auto &objinfo = registry.get<components::ObjectInfo>(ent);
        auto id = objinfo.id;

        SQLite::Statement q8(*db, "SELECT value FROM objectTypes WHHERE objectId = ?");
        q8.bind(1, id);

        while(q8.executeStep()) {
            auto value = q8.getColumn(0).getText();
            auto found = typeRegistry.find(value);
            if(found != typeRegistry.end()) {
                objinfo.types[std::string(found->second->getKey())] = found->second;
            }
        }
        objinfo.doSort();

        SQLite::Statement q1(*db, "SELECT * FROM objectStrings WHERE objectId = ?");
        q1.bind(1, id);

        // Now we check if we got any results...
        if (q1.executeStep()) {
            auto& strings = registry.emplace<components::Strings>(ent);
            do {
                auto key = q1.getColumn(1).getText();
                auto value = q1.getColumn(2).getText();
                strings.data[key] = value;
            } while (q1.executeStep());
        }
        // Thank you copilot.

        SQLite::Statement q2(*db, "SELECT * FROM objectIntegers WHERE objectId = ?");
        q2.bind(1, id);
        if (q2.executeStep()) {
            auto& ints = registry.emplace<components::Integers>(ent);
            do {
                auto key = q2.getColumn(1).getText();
                auto value = q2.getColumn(2).getInt();
                ints.data[key] = value;
            } while (q2.executeStep());
        }

        SQLite::Statement q3(*db, "SELECT * FROM objectDoubles WHERE objectId = ?");
        q3.bind(1, id);

        if (q3.executeStep()) {
            auto& doubles = registry.emplace<components::Doubles>(ent);
            do {
                auto key = q3.getColumn(1).getText();
                auto value = q3.getColumn(2).getDouble();
                doubles.data[key] = value;
            } while (q3.executeStep());
        }

        SQLite::Statement q4(*db, "SELECT * FROM objectRelations WHERE objectId = ?");
        q4.bind(1, id);

        if(q4.executeStep()) {
            do {
                auto key = q4.getColumn(1).getText();
                auto value = q4.getColumn(2).getInt();

                auto it = entities.find(value);
                if (it != entities.end()) {
                    setRelation(ent, key, it->second);
                }

            } while (q4.executeStep());
        }

        SQLite::Statement q5(*db, "SELECT * FROM objectStats WHERE objectId = ?");
        q5.bind(1, id);

        if(q5.executeStep()) {
            auto& stats = registry.emplace<components::Stats>(ent);
            do {
                auto key = q5.getColumn(1).getText();
                auto value = q5.getColumn(2).getInt();
                stats.data[key] = value;
            } while (q5.executeStep());
        }

        SQLite::Statement q6(*db, "SELECT * FROM objectEquipment WHERE objectId = ?");
        q6.bind(1, id);

        if(q6.executeStep()) {
            auto& equip = registry.emplace<components::Equipment>(ent);
            do {
                auto key = q6.getColumn(1).getText();
                auto value = q6.getColumn(2).getInt();

                auto it = entities.find(value);
                if (it != entities.end()) {
                    equip.data[key] = it->second;
                }

            } while (q6.executeStep());
        }

        SQLite::Statement q7(*db, "SELECT * FROM objectAspect WHERE objectId = ?");
        q7.bind(1, id);

        if(q7.executeStep()) {
            auto& aspects = registry.emplace<components::Aspects>(ent);
            do {
                auto key = q7.getColumn(1).getText();
                auto value = q7.getColumn(2).getText();

                auto [success, err] = setAspect(ent, key, value);

            } while (q7.executeStep());
        }




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