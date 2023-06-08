#include "kaizermud/Database.h"
#include "kaizermud/Components.h"
#include "kaizermud/Prototypes.h"
#include "kaizermud/Api.h"
#include "kaizermud/Types.h"
#include "kaizermud/utils.h"
#include <iostream>
#include "spdlog/spdlog.h"
#include "kaizermud/game.h"
#include "kaizermud/LoadMessages.h"
#include "kaizermud/Config.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <regex>
// let's get chrono literals?


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
        j["Types"] = objinfo.typeFlags.to_ulong();

        if(!asPrototype) {
            auto account = registry.try_get<components::Account>(ent);
            if(account) {
                j["account"] = account->serialize();
            }
        }

        auto quirks = registry.try_get<components::Quirks>(ent);
        if(quirks) {
            for(auto &[slot, qmap] : quirks->data)
                for(auto &[key, quirk] : qmap)
                    j["quirks"][slot].push_back(key);
        }

        auto name = registry.try_get<components::Name>(ent);
        if(name) {
            j["Name"] = std::string(name->data);
        }

        auto ldesc = registry.try_get<components::LookDescription>(ent);
        if(ldesc) {
            j["LookDescription"] = std::string(ldesc->data);
        }

        auto sdesc = registry.try_get<components::ShortDescription>(ent);
        if(sdesc) {
            j["ShortDescription"] = std::string(sdesc->data);
        }

        auto rdesc = registry.try_get<components::RoomDescription>(ent);
        if(rdesc) {
            j["RoomDescription"] = std::string(rdesc->data);
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


            auto ex = registry.try_get<components::Exit>(ent);
            if(ex) {
                nlohmann::json exdata;
                exdata["location"] = getID(ex->location);
                exdata["destination"] = getID(ex->destination);
                j["Exit"] = exdata;
            }
        }

        for(auto &func : serializeFuncs)
            func(ent, j, asPrototype);

        return j;

    }

    std::chrono::high_resolution_clock::duration saveToDB(entt::entity ent, const std::shared_ptr<SQLite::Database>& db) {
        SQLite::Statement q1(*db, "INSERT INTO objects (id, data) VALUES (?,?)");
        auto start = std::chrono::high_resolution_clock::now();
        auto data = serializeObject(ent, false);
        auto end = std::chrono::high_resolution_clock::now();

        q1.bind(1, getID(ent));
        q1.bind(2, data.dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
        q1.exec();
        return end - start;
    }

    void savePrototype(std::string_view name, const nlohmann::json& j) {

    }

    void deserializeObject(entt::entity ent, const nlohmann::json& j) {
        //spdlog::info("Deserializing object {}: {}", getID(ent), j.dump(4));
        if(j.count("account")) {
            auto &account = registry.emplace<components::Account>(ent, j["account"]);
        }

        if(j.count("Types")) {
            auto &objinfo = registry.get_or_emplace<components::ObjectInfo>(ent);
            std::bitset<32> bits(j["Types"].get<unsigned long>());
            objinfo.typeFlags = bits;
        }

        if(j.count("Name")) {
            auto &comp = registry.get_or_emplace<components::Name>(ent);
            comp.data = intern(j["Name"].get<std::string>());
        }

        if(j.count("ShortDescription")) {
            auto &comp = registry.get_or_emplace<components::ShortDescription>(ent);
            comp.data = intern(j["ShortDescription"].get<std::string>());
        }

        if(j.count("LookDescription")) {
            auto &comp = registry.get_or_emplace<components::LookDescription>(ent);
            comp.data = intern(j["LookDescription"].get<std::string>());
        }

        if(j.count("RoomDescription")) {
            auto &comp = registry.get_or_emplace<components::RoomDescription>(ent);
            comp.data = intern(j["RoomDescription"].get<std::string>());
        }

        if(j.count("integers")) {
            auto &ints = registry.get_or_emplace<components::Integers>(ent);
            for(auto &[key, value] : j["integers"].items())
                ints.data[key] = value.get<int64_t>();
        }

        if(j.count("doubles")) {
            auto &doubles = registry.get_or_emplace<components::Doubles>(ent);
            for(auto &[key, value] : j["doubles"].items())
                doubles.data[key] = value.get<double>();
        }

        if(j.count("stats")) {
            auto &stats = registry.get_or_emplace<components::Stats>(ent);
            for(auto &[key, value] : j["stats"].items())
                stats.data[key] = value.get<double>();
        }

        if(j.count("quirks")) {
            auto &quirks = registry.get_or_emplace<components::Quirks>(ent);
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
            auto &eq = registry.get_or_emplace<components::Equipment>(ent);
            for(auto &[slot, id] : j["equipment"].items()) {
                auto it = entities.find(id.get<int64_t>());
                if(it != entities.end()) {
                    eq.data[slot] = it->second;
                }
            }
        }

        if(j.count("Location")) {
            auto it = entities.find(j["Location"].get<ObjectID>());
            setLocation(ent, it->second);
        }

        if(j.count("relations")) {
            for(auto &[key, id] : j["relations"].items()) {
                auto it = entities.find(id.get<int64_t>());
                setRelation(ent, key, it->second);
            }
        }

        if(j.count("Exit")) {
            auto exdata = j["Exit"];
            auto loc = entities.find(exdata["location"].get<ObjectID>());
            if(loc != entities.end()) {
                auto location = loc->second;
                auto dest = entities.find(exdata["destination"].get<ObjectID>());
                if(dest != entities.end()) {
                    auto destination = dest->second;
                    auto &ex = registry.emplace<components::Exit>(ent);
                    ex.location = location;
                    ex.destination = destination;
                    auto name = getDisplayName(ent, ent);
                    auto &exits = registry.get_or_emplace<components::Exits>(location);
                    exits.data[name] = ent;
                    auto &entra = registry.get_or_emplace<components::Entrances>(destination);
                    entra.data[name] = ent;
                }
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

    void loadLatestSave() {
        namespace fs = std::filesystem;
         // There should be a folder named config::dbFolder off of cwd. Let's create a path into it.
        auto path = fs::path(config::dbFolder);
        // and if it doesn't exist, create it.
        fs::create_directories(path);

        // Now, let's find the most recent save file. The filenames tell when it was made. the format is
        // {config::dbName}_YYYYMMDDHHMMSS.{config::dbExt} such as "game_20200101120000.sqlite3"
        // it's possible that there won't BE any save files... I'll deal with that later.

        // First, let's get a list of all the files in the directory.
        std::vector<fs::path> files;
        for(auto &p : fs::directory_iterator(path)) {
            if(std::filesystem::is_regular_file(p.path()))
                files.push_back(p.path());
        }

        // next we need to make sure the filenames match the format...
        std::regex re(fmt::format("{}_\\d{{14}}\\.{}", config::dbName, config::dbExt));

        // Filter out everything that's not proper.
        std::vector<fs::path> filtered;
        std::copy_if(files.begin(), files.end(), std::back_inserter(filtered), [&re](const fs::path &p) {
            return std::regex_match(p.filename().string(), re);
        });

        // now we are interested in that last one, which is the most recent. We need to determine that based on the
        // YYYYMMDDHHMMSS part of the filename.
        std::sort(filtered.begin(), filtered.end(), [](const fs::path &a, const fs::path &b) {
            return a.filename().string() > b.filename().string();
        });

        // I'm skeptical of it being that easy, copilot, but okay.

        if(filtered.empty()) {
            // We'll do something fancier later but this is good for now.
            spdlog::warn("No save files found.");
            return;
        }
        auto latest = filtered.front();
        spdlog::info("Loading save file {}", latest.string());
        loadDatabase(latest.string());

        // Thank you, copilot.

    }

    void loadDatabase(std::string_view path) {
        auto loadDb = std::make_shared<SQLite::Database>(path.data(), SQLite::OPEN_READONLY);
        // let's time the operation. gimme a timestamp.
        auto start = std::chrono::high_resolution_clock::now();

        broadcast("Loading game database... please wait warmly...");
        broadcast("Going off the map... here be dragons...");
        for (const auto& func: preLoadFuncs) {
            func(loadDb);
        }
        std::size_t rows = 0, protos = 0;

        SQLite::Statement countQuery(*loadDb, "SELECT COUNT(*) FROM objects");
        if (countQuery.executeStep()) {
            rows = countQuery.getColumn(0).getInt();
        }
        auto objCount = rows / 5;

        SQLite::Statement protoCountQuery(*loadDb, "SELECT COUNT(*) FROM prototypes");
        if (protoCountQuery.executeStep()) {
            protos = protoCountQuery.getColumn(0).getInt();
        }
        auto protosCount = protos / 5;

        spdlog::info("Loading {} objects and {} prototypes", rows, protos);
        broadcast(fmt::format("Loading {} objects and {} prototypes", rows, protos));
        prototypes.reserve(protos);

        SQLite::Statement protoQuery(*loadDb, "SELECT name,data FROM prototypes");
        broadcast(fmt::format("{} (Loading prototypes: 0/{})", sillyLoadMessage(), protos));
        auto protoCounter = 0;
        while (protoQuery.executeStep()) {
            auto name = protoQuery.getColumn(0).getString();
            auto data = nlohmann::json::parse(protoQuery.getColumn(1).getString());
            registerPrototype(name, data);
            protoCounter++;
            if(protoCounter % protosCount == 0) {
                broadcast(fmt::format("{} (Loading prototypes: {}/{})", sillyLoadMessage(), protoCounter, protos));
            }
        }
        broadcast(fmt::format("{} (Loading prototypes: {}/{})", sillyLoadMessage(), protoCounter, protos));

        std::unordered_map<entt::entity, nlohmann::json> loadingObjects;
        loadingObjects.reserve(rows);
        entities.reserve(rows);
        SQLite::Statement stmt(*loadDb, "SELECT id,data FROM objects");
        auto objCounter = 0;
        broadcast(fmt::format("{} (Instantiating objects: 0/{})", sillyLoadMessage(), rows));
        while (stmt.executeStep()) {
            std::optional<ObjectID> id = stmt.getColumn(0).getInt();
            auto [ent, err] = createEntity(id);
            if(err) {
                spdlog::error("Error loading entity {}: {}", id.value(), err.value());
                continue;
            }
            loadingObjects[ent] = nlohmann::json::parse(stmt.getColumn(1).getString());
            objCounter++;
            if(objCounter % objCount == 0) {
                broadcast(fmt::format("{} (Instantiating objects: {}/{})", sillyLoadMessage(), objCounter, rows));
            }
        }
        broadcast(fmt::format("{} (Instantiating objects: {}/{})", sillyLoadMessage(), objCounter, rows));

        objCounter = 0;
        broadcast(fmt::format("{} (Hydrating objects: 0/{})", sillyLoadMessage(), rows));
        for(auto &[ent, data] : loadingObjects) {
            deserializeObject(ent, data);
            objCounter++;
            if(objCounter % objCount == 0) {
                broadcast(fmt::format("{} (Hydrating objects: {}/{})", sillyLoadMessage(), objCounter, objCount));
            }
        }
        broadcast(fmt::format("{} (Hydrating objects: {}/{})", sillyLoadMessage(), objCounter, objCount));

        broadcast("Tidying up bits and bobs...");
        for (const auto& func: postLoadFuncs) {
            func(loadDb);
        }

        // log how long it took.
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        spdlog::info("Loaded game database in {}ms", duration);
        broadcast(fmt::format("Done loading game database, hooray! It just took {} milliseconds!", duration));
    }

    void saveSnapShot() {
        // This is the opposite of loadLatestSave(). It will save the current state of the game to a new file.
        namespace fs = std::filesystem;
        // There should be a folder named config::dbFolder off of cwd. Let's create a path into it.
        auto path = fs::path(config::dbFolder);
        // and if it doesn't exist, create it.
        fs::create_directories(path);

        // Now we need to create a filename.
        auto filename = generateDbFilename();

        // Call saveDatabase with the combination of path and filename.
        saveDatabase((path / filename).string());
    }

    void saveDatabase(std::string_view path) {
        using namespace std::chrono_literals;
        spdlog::info("Saving game database to {}", path);
        // Let's time the save operation. gimme a timestamp.
        auto start = std::chrono::high_resolution_clock::now();
        auto saveDb = std::make_shared<SQLite::Database>(path.data(), SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        SQLite::Transaction transaction(*saveDb);

        createSchema(saveDb);

        for (const auto& func: preSaveFuncs) {
            func(saveDb);
        }

        // I want to save timestamps for how long it took to serialize each thing.
        // Afterwards I'll calculate mean, median, and mode.
        // Gimme a collection of high-resolution durations.
        std::vector<std::chrono::high_resolution_clock::duration> durations;
        durations.reserve(entities.size());
        for (const auto& [id, ent]: entities) {
            durations.push_back(saveToDB(ent, saveDb));
        }
        // First, the total duration.
        auto total = std::accumulate(durations.begin(), durations.end(), 0ns);
        // Then, the mean.
        auto mean = total / durations.size();


        spdlog::info("Serialization times: total: {}ms, mean: {}ms", total / 1ms, mean / 1ms);

        SQLite::Statement q1(*saveDb, "INSERT INTO prototypes (name,data) VALUES (?,?)");
        for(const auto& [key, proto] : prototypes) {
            q1.bind(1, key);
            q1.bind(2, proto->getData().dump(-1, ' ', false, nlohmann::json::error_handler_t::replace));
            q1.exec();
            q1.reset();
        }

        for (const auto& func: postSaveFuncs) {
            func(saveDb);
        }

        transaction.commit();
        // log the time it took to save.
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
        spdlog::info("Saved game database to {} in {}ms", path, duration);
    }

    std::string generateDbFilename() {
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now = *std::localtime(&time_t_now);

        std::stringstream ss;
        ss << config::dbName << "_";
        ss << std::put_time(&tm_now, "%Y%m%d%H%M%S");  // Format the timestamp
        ss << "." << config::dbExt;
        return ss.str();
    }
}