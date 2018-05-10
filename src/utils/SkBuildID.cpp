/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBuildID.h"

constexpr uint8_t convert(char value) {
    return value >= 0 ? value : (uint8_t)((int)value + 256);
}

constexpr uint8_t deref(const char* p) { return convert(*p); }

constexpr uint32_t load(const char* data) {
    return (uint32_t)deref(data) |
           ((uint32_t)deref(data + 1) << 8) |
           ((uint32_t)deref(data + 2) << 16) |
           ((uint32_t)deref(data + 3) << 24);
}

constexpr uint32_t shiftmix(uint32_t hash, int shift) { return hash ^ (hash >> shift); }

constexpr uint32_t mix(uint32_t hash) {
    return shiftmix(shiftmix(shiftmix(hash, 16) * 0x85EBCA6B, 13) * 0xC2B2AE35, 16);
}

constexpr uint32_t rotate(uint32_t k, int shift) {
    return (k << (32 - shift)) | (k >> shift);
}
constexpr uint32_t hash_4_bytes(uint32_t k, uint32_t hash) {
        return (rotate(hash ^ rotate(k * 0xCC9E2D51, 17) * 0x1B873593, 19) * 5) + 0xE6546B64;
}

constexpr uint32_t constexpr_strlen(const char* s) {
    return *s ? 1 + constexpr_strlen(s + 1) : 0;
}

constexpr uint32_t headhash(const char* string, uint32_t n) {
    return n > 0 ? hash_4_bytes(load(string + 4 * (n - 1)), headhash(string, n - 1)) : 0;
}

constexpr uint32_t subload(const char* data, uint32_t len, uint32_t accumulator) {
    return len == 3 ? subload(data, 2, deref(data + 2) << 16)
         : len == 2 ? subload(data, 1, accumulator ^ (deref(data + 1) << 8))
         :            accumulator ^ deref(data);
}

constexpr uint32_t tailmix(uint32_t k) {
    return rotate(k * 0xCC9E2D51, 17) * 0x1b873593;
}

constexpr uint32_t tailhash(const char* string, uint32_t len, uint32_t hash) {
    return len == 0 ? hash : hash ^ tailmix(subload(string, len, 0));
}

constexpr uint32_t hasher2(const char* string, uint32_t len) {
    return mix(tailhash(string + 4 * (len / 4), len % 4, headhash(string, len / 4)) ^ len);
}

constexpr uint32_t hasher(const char* string) {
    return hasher2((const char*)string, constexpr_strlen(string));
}

uint32_t SkBuildID() {
    constexpr uint32_t value = hasher(__DATE__ " - " __TIME__);
    return value;
}

