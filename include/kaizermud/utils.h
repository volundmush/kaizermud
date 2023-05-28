#pragma once
#include "kaizermud/base.h"
#include <unordered_set>

constexpr unsigned int string_hash(const char *str, int h = 0) {
    return !str[h] ? 5381 : (string_hash(str, h + 1) * 33) ^ str[h];
}

constexpr unsigned int operator"" _hash(const char *str, size_t) {
    return string_hash(str);
}

namespace kaizermud::utils {

    extern std::unordered_set<std::string> stringPool;

    std::string_view intern(const std::string& str);
    std::string_view intern(std::string_view str);


    template <typename Iterator, typename Key = std::function<std::string(typename Iterator::value_type)>>
    std::optional<typename Iterator::value_type> partialMatch(
            const std::string& match_text,
            Iterator begin, Iterator end,
            bool exact = false,
            Key key = [](const auto& val){ return std::to_string(val); }
            )
    {
        std::string mlow = match_text;
        std::transform(mlow.begin(), mlow.end(), mlow.begin(), ::tolower);
        std::vector<typename Iterator::value_type> candidates_sorted;

        // Use a multimap to automatically sort by the transformed key.
        std::multimap<std::string, typename Iterator::value_type> sorted_map;
        std::for_each(begin, end, [&](const auto& val) {
            std::string str = key(val);
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
            sorted_map.insert({str, val});
        });

        for (const auto& pair : sorted_map)
        {
            if (mlow == pair.first)
            {
                return pair.second;
            }
            else if (!exact && pair.first.starts_with(mlow))
            {
                return pair.second;
            }
        }
        return std::nullopt;
    }


}