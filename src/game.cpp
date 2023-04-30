#include "kaizermud/game.h"


namespace kaizermud::game {

    namespace state {
        std::vector<std::optional<Object>> objects;
        std::set<int64_t> free_ids;
        std::unordered_map<uint64_t, std::shared_ptr<kaizermud::net::ClientConnection>> connections;
    }

    void fill_free_ids() {
        // Scan the objects vector to find free IDs
        size_t count = 0;
        size_t index = 0;
        for (const auto& obj : state::objects) {
            if (count >= 50) {
                break;
            }
            if (!obj.has_value()) {
                state::free_ids.insert(static_cast<int64_t>(index));
                count++;
            }
            index++;
        }
    }

    std::optional<int64_t> pop_free_id() {
        if (!state::free_ids.empty()) {
            // Get the iterator pointing to the first free ID in the set
            auto it = state::free_ids.begin();
            int64_t id = *it;
            state::free_ids.erase(it);
            return id;
        }
        return {};
    }

    int64_t get_next_available_id() {
        auto maybe_id = pop_free_id();
        if(maybe_id.has_value()) {
            return maybe_id.value();
        }
        fill_free_ids();

        maybe_id = pop_free_id();
        if(maybe_id.has_value()) {
            return maybe_id.value();
        }

        // No free IDs found, use the next ID in the sequence
        auto id = static_cast<int64_t>(state::objects.size());
        state::objects.emplace_back(); // Add an empty optional to the vector
        return id;
    }


    ObjectReference Object::makeReference() const {
        return {id, timestamp};
    }

    std::optional<std::reference_wrapper<Object>> ObjectReference::getObject() const {
        // Check if the object_id_ is within the bounds of the vector
        if (object_id_ < 0 || object_id_ >= state::objects.size()) {
            return std::nullopt;
        }

        // Check if the object at the given index exists and the timestamp matches
        auto& obj_opt = state::objects[object_id_];
        if (obj_opt.has_value() && obj_opt->timestamp == timestamp_) {
            return std::ref(obj_opt.value());
        }

        // Object not found or timestamp mismatch
        return std::nullopt;
    }

}