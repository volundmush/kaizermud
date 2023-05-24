#pragma once
#include "kaizermud/base.h"

namespace kaizermud::game {

    class StatHandler;

    // Base type for all Stats. This implements the base API.
    class Stat {
    public:
        explicit Stat(StatHandler* handler);
        virtual ~Stat();
        virtual void set(double value) = 0;
        virtual void modify(double value) = 0;
        virtual double get() = 0;
        virtual double getBase() = 0;
        virtual std::string getName() const = 0;
        virtual bool shouldSave() const = 0;
    protected:
        StatHandler* handler;
    };

    // This stat has a real base value that's stored persistently to the database.
    class RealStat : public Stat {
    public:
        using Stat::Stat;
        ~RealStat() override;
        void set(double value) override;
        void modify(double value) override;
        double get() override;
        double getBase() override;
        bool shouldSave() const override;
    protected:
        double base;
    };

    // Virtual Stats have no real base, they are entirely derived from
    // other values.
    class VirtualStat : public Stat {
    public:
        using Stat::Stat;
        ~VirtualStat() override;
        void set(double value) override;
        void modify(double value) override;
        bool shouldSave() const override;
    };

    class StatEntry {
    public:
        std::string object_type, name;
        std::function<std::unique_ptr<Stat>(StatHandler*)> ctor;
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, StatEntry>> statRegistry;

    void registerStat(StatEntry entry);

    class StatHandler {
        friend class Stat;
    public:
        explicit StatHandler(Object *obj);
        void load();
    protected:
        Object *obj;
        std::unordered_map<std::string, std::unique_ptr<Stat>> stats{};
    };

}