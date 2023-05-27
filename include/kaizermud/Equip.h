#pragma once
#include "kaizermud/base.h"
#include <optional>
#include <string_view>
#include "SQLiteCpp/SQLiteCpp.h"


namespace kaizermud::game {

    class EquipHandler;

    class EquipSlot {
    public:
        EquipSlot(EquipHandler* handler, const std::string &slot, const std::string &slotType);
        virtual ~EquipSlot() = default;

        [[nodiscard]] virtual std::string_view getSlot() const;
        [[nodiscard]] virtual std::string_view getSlotType() const;

        virtual void equip(const std::shared_ptr<Object>& object);

        [[nodiscard]] virtual bool isAvailable() const;

        [[nodiscard]] virtual OpResult canEquip(const std::shared_ptr<Object>& object) const;

        virtual void setProperty(const std::string& name, std::string_view value);
        [[nodiscard]] virtual std::string_view getProperty(std::string_view name) const;

        void setSortOrder(int order);
        int getSortOrder() const;

        std::shared_ptr<Object> item;

    protected:
        EquipHandler* handler;

        std::string_view slot, slotType;
        std::unordered_map<std::string, std::string_view> properties;
        int sortOrder{0};
    };

    struct EquipEntry {
        std::string objType, slot, slotType;
        std::unordered_map<std::string, std::string> properties;
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
        virtual void loadFromDB(const std::shared_ptr<SQLite::Database> &db);
        virtual void saveToDB(const std::shared_ptr<SQLite::Database> &db);
    protected:
        bool loaded;
    };

}