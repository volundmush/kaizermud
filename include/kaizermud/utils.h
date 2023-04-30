#pragma once
#include "kaizermud/base.h"
#include <string_view>

constexpr unsigned int string_hash(const char *str, int h = 0) {
    return !str[h] ? 5381 : (string_hash(str, h + 1) * 33) ^ str[h];
}

constexpr unsigned int operator"" _hash(const char *str, size_t) {
    return string_hash(str);
}