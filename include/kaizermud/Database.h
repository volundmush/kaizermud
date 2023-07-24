#pragma once

#include "kaizermud/base.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "nlohmann/json.hpp"
#include "kaizermud/Types.h"

namespace kaizer {
    extern std::shared_ptr<SQLite::Database> db;
    extern std::unordered_map<std::string, std::shared_ptr<SQLite::Database>> extraDB;

    extern std::vector<std::string> schema;

    extern std::vector<std::function<void()>> loadFuncs;

    void runQuery(const std::shared_ptr<SQLite::Database>& db, std::string_view query);

    void createSchema();

    void readyDatabase();

    entt::entity getEntity(ObjectID id);

    Type* getType(const std::string& name);
    Type* getType(ObjectID id);
    Type* getType(entt::entity ent);

}