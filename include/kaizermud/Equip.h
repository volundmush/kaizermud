#pragma once
#include "kaizermud/base.h"
#include <optional>
#include <string_view>


namespace kaizermud::game {

    class EquipHandler;

    class EquipSlot {
    public:
        EquipSlot(EquipHandler* handler, const std::string &slot, const std::string &slotType);
        virtual ~EquipSlot();

        std::string_view getSlot() const;
        std::string_view getSlotType() const;

        virtual void equip(Object* object) = 0;

        virtual bool isAvailable() const;

        virtual OpResult canEquip(Object* object) const;

        virtual void setWearVerb(const std::string& verb);
        virtual std::string_view getWearVerb() const;

        virtual void setWearDisplay(const std::string& display);
        virtual std::string_view getWearDisplay() const;

        virtual void setRemoveVerb(const std::string& verb);
        virtual std::string_view getRemoveVerb() const;

        virtual void setRemoveDisplay(const std::string& display);
        virtual std::string_view getRemoveDisplay() const;

        virtual void setListDisplay(const std::string& display);
        virtual std::string_view getListDisplay() const;

        void setSortOrder(int order);
        int getSortOrder() const;

    protected:
        EquipHandler* handler;
        std::optional<ObjectID> item;
        std::string_view slot, slotType;
        std::optional<std::string_view> wearVerb, wearDisplay, removeVerb, removeDisplay, listDisplay;
        int sortOrder{0};
    };

    struct EquipEntry {
        std::string objType, slot, slotType;
        std::optional<std::string> wearVerb, wearDisplay, removeVerb, removeDisplay, listDisplay;
        int sortOrder{0};
        std::function<std::unique_ptr<EquipSlot>(EquipHandler*, const std::string, const std::string)> ctor;
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, EquipEntry>> equipRegistry;
    void registerEquip(EquipEntry entry);

    class EquipHandler {
    public:
        explicit EquipHandler(Object *obj);
        Object *obj;
        std::unordered_map<std::string, std::unique_ptr<EquipSlot>> slots;
        void load();
    };

}