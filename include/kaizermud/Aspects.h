#pragma once

#include <optional>
#include "kaizermud/base.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "kaizermud/CallParameters.h"

namespace kaizermud::game {
    class AspectHandler;

    class AspectSlot;
    class AspectEntry;

    class Aspect {
    public:
        std::string objType, slotType, saveKey, name;
        virtual void onRemove(AspectHandler *handler);
        virtual void onAdd(AspectHandler *handler);
        virtual void onLoad(AspectHandler *handler);
    };

    class AspectEntry : public CallParameters {
    public:
        std::string objType, slotType, saveKey, name;
        std::function<std::unique_ptr<Aspect>(AspectSlot*)> ctor;
        std::unordered_map<std::string, std::function<CallParameters(const CallParameters&, const AspectEntry&)>> funcs;
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<Aspect>>>> aspectRegistry;

    OpResult registerAspect(AspectEntry entry);

    class AspectSlotEntry;

    class AspectSlot : public CallParameters {
    public:
        std::string objType, slotType;
        [[nodiscard]] virtual OpResult setAspect(AspectHandler *handler, const std::string& saveKey, bool isLoading);
        [[nodiscard]] virtual OpResult atPostLoad(AspectHandler *handler);
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, std::shared_ptr<AspectSlot>>> aspectSlotRegistry;

    OpResult registerAspectSlot(std::shared_ptr<AspectSlot> entry);

    class AspectHandler {
    public:
        explicit AspectHandler(const std::shared_ptr<Object>& obj);
        std::shared_ptr<Object> obj;
        std::unordered_map<std::string, std::shared_ptr<AspectSlot>> slots;
        std::unordered_map<std::string, std::shared_ptr<Aspect>> aspects;
        virtual void load();
        virtual void loadFromDB(const std::shared_ptr<SQLite::Database> &db);
        virtual void saveToDB(const std::shared_ptr<SQLite::Database> &db);
    protected:
        bool loaded;
    };

}