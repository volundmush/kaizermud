#include "kaizermud/Stats.h"
#include "kaizermud/Object.h"

namespace kaizermud::game {

    std::unordered_map<std::string, std::unordered_map<std::string, StatEntry>> statRegistry{};

    Stat::Stat(StatHandler *handler) {
        this->handler = handler;
    }

    double Stat::get() {
        return getBase();
    }

    // RealStats
    void RealStat::set(double value) {
        this->base = value;
    }

    void RealStat::modify(double value) {
        this->base += value;
    }

    double RealStat::getBase() {
        return base;
    }

    bool RealStat::shouldSave() const {
        return true;
    }

    // VirtualStats
    void VirtualStat::set(double value) {
        // does nothing!
    }

    void VirtualStat::modify(double value) {
        // does nothing!
    }

    bool VirtualStat::shouldSave() const {
        return false;
    }

    OpResult registerStat(StatEntry entry) {
        if(entry.object_type.empty()) {
            return {false, "StatEntry object_type cannot be empty"};
        }
        if(entry.name.empty()) {
            return {false, "StatEntry name cannot be empty"};
        }

        if(entry.ctor == nullptr) {
            return {false, "StatEntry ctor cannot be null"};
        }

        auto &reg = statRegistry[entry.object_type];

        if(reg.find(entry.name) != reg.end()) {
            return {false, "StatEntry already registered"};
        }

        reg[entry.name] = entry;
        return {true, std::nullopt};
    }

    // StatHandler functions
    StatHandler::StatHandler(const std::shared_ptr<Object>& obj) : obj(obj) {

    }

    void StatHandler::load() {
        if(loaded) return;
        loaded = true;

        // This function could potentially be optimized.
        // Each unique sequence of getTypes() should have the same
        // stats to load, so we could cache the results of this function.
        std::unordered_map<std::string, StatEntry> statsToLoad;

        // Retrieve all stats to load.
        for(const auto &ctype : obj->getTypes()) {
            auto it = statRegistry.find(std::string(ctype));
            if(it != statRegistry.end()) {
                for(const auto &entry : it->second) {
                    statsToLoad[entry.first] = entry.second;
                }
            }
        }

        // Now let's instantiate them.
        for(const auto &[name, stat] : statsToLoad) {
            stats[name] = stat.ctor(this);
        }
    }
}