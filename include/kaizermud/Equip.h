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

        virtual void equip(const std::shared_ptr<Object> object);

        virtual bool isAvailable() const;

        virtual OpResult canEquip(const std::shared_ptr<Object> object) const;

        virtual void setProperty(const std::string& name, std::string_view value);
        [[nodiscard]] virtual std::string_view getProperty() const;

        void setSortOrder(int order);
        int getSortOrder() const;

    protected:
        EquipHandler* handler;
        std::shared_ptr<Object> item;
        std::string_view slot, slotType;
        std::unordered_map<std::string, std::string_view> properties;
        int sortOrder{0};
    };

    struct EquipEntry {
        std::string objType, slot, slotType;
        std::optional<std::string> wearVerb, wearDisplay, removeVerb, removeDisplay, listDisplay;
        int sortOrder{0};
        std::function<std::unique_ptr<EquipSlot>(EquipHandler*, const std::string, const std::string)> ctor;
    };

    extern std::unordered_map<std::string, std::unordered_map<std::string, EquipEntry>> equipRegistry;
    OpResult registerEquip(EquipEntry entry);

    class EquipHandler {
    public:
        explicit EquipHandler(const std::shared_ptr<Object>& obj);
        std::shared_ptr<Object> obj;
        std::unordered_map<std::string, std::unique_ptr<EquipSlot>> slots;
        virtual void load();
    protected:
        bool loaded;
    };

}