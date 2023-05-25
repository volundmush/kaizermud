#pragma once

#include <optional>
#include "kaizermud/base.h"

namespace kaizermud::game {
    class AspectHandler;

    class AspectSlot;

    class Aspect {
    public:
        Aspect(AspectSlot *slot);
        virtual ~Aspect() = default;
        AspectSlot *slot;
        Object* obj();
        std::string_view getSlotType();
        virtual std::string_view getSaveKey() const = 0;
        virtual std::string_view getName() const = 0;
        virtual void onRemove();
        virtual void onAdd();
        virtual void onLoad();
    };

    struct AspectEntry {
        std::string objType, slotType, saveKey;
        std::function<std::unique_ptr<Aspect>(AspectSlot*)> ctor;
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, AspectEntry>>> aspectRegistry;

    OpResult registerAspect(AspectEntry entry);

    class AspectSlot {
    public:
        AspectSlot(AspectHandler *handler, const std::string& slotType);
        virtual ~AspectSlot() = default;
        AspectHandler *handler;
        Object *obj();
        std::unique_ptr<Aspect> aspect;
        std::string_view slotType;
        [[nodiscard]] virtual OpResult setAspect(const std::string& saveKey, bool isLoading);
    };

    struct AspectSlotEntry {
        std::string objType, slotType;
        std::function<std::unique_ptr<AspectSlot>(AspectHandler*)> ctor;
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, AspectSlotEntry>> aspectSlotRegistry;

    OpResult registerAspectSlot(AspectSlotEntry entry);

    class AspectHandler {
    public:
        explicit AspectHandler(const std::shared_ptr<Object>& obj);
        std::shared_ptr<Object> obj;
        std::unordered_map<std::string, std::unique_ptr<AspectSlot>> slots;
        virtual void load();
    protected:
        bool loaded;
    };

}