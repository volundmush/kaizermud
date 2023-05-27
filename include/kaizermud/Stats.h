#pragma once
#include "kaizermud/base.h"
#include "SQLiteCpp/SQLiteCpp.h"


namespace kaizermud::game {

    class StatHandler;

    // Base type for all Stats. This implements the base API.
    class Stat {
    public:
        explicit Stat(StatHandler* handler);
        virtual ~Stat() = default;
        virtual void set(double value) = 0;
        virtual void modify(double value) = 0;
        [[nodiscard]] virtual double get();
        [[nodiscard]] virtual double getBase() = 0;
        [[nodiscard]] virtual std::string getSaveKey() const = 0;
        [[nodiscard]] virtual std::string getName() const = 0;
        [[nodiscard]] virtual bool shouldSave() const = 0;
    protected:
        StatHandler* handler;
    };

    // This stat has a real base value that's stored persistently to the database.
    class RealStat : public Stat {
    public:
        using Stat::Stat;
        void set(double value) override;
        void modify(double value) override;
        [[nodiscard]] double getBase() override;
        [[nodiscard]] bool shouldSave() const override;
    protected:
        double base;
    };

    // Virtual Stats have no real base, they are entirely derived from
    // other values.
    class VirtualStat : public Stat {
    public:
        using Stat::Stat;
        void set(double value) override;
        void modify(double value) override;
        [[nodiscard]] bool shouldSave() const override;
    };

    class StatEntry {
    public:
        std::string object_type, name;
        std::function<std::unique_ptr<Stat>(StatHandler*)> ctor;
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, StatEntry>> statRegistry;

    OpResult registerStat(StatEntry entry);

    class StatHandler {
    public:
        explicit StatHandler(const std::shared_ptr<Object>& obj);
        virtual void load();
        virtual void loadFromDB(const std::shared_ptr<SQLite::Database> &db);
        virtual void saveToDB(const std::shared_ptr<SQLite::Database> &db);
        std::shared_ptr<Object> obj;
    protected:
        bool loaded;
        std::unordered_map<std::string, std::unique_ptr<Stat>> stats{};
    };

}