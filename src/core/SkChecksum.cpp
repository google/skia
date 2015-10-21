/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChecksum.h"

uint32_t SkChecksum::Murmur3(const void* data, size_t bytes, uint32_t seed) {
    // Use may_alias to remind the compiler we're intentionally violating strict aliasing,
    // and so not to apply strict-aliasing-based optimizations.
    typedef uint32_t SK_ATTRIBUTE(may_alias) aliased_uint32_t;
    typedef uint8_t SK_ATTRIBUTE(may_alias) aliased_uint8_t;

    // Handle 4 bytes at a time while possible.
    const aliased_uint32_t* safe_data = (const aliased_uint32_t*)data;
    const size_t words = bytes/4;
    uint32_t hash = seed;
    for (size_t i = 0; i < words; i++) {
        uint32_t k = safe_data[i];
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;

        hash ^= k;
        hash = (hash << 13) | (hash >> 19);
        hash *= 5;
        hash += 0xe6546b64;
    }

    // Handle last 0-3 bytes.
    const aliased_uint8_t* safe_tail = (const uint8_t*)(safe_data + words);
    uint32_t k = 0;
    switch (bytes & 3) {
        case 3: k ^= safe_tail[2] << 16;
        case 2: k ^= safe_tail[1] <<  8;
        case 1: k ^= safe_tail[0] <<  0;
                k *= 0xcc9e2d51;
                k = (k << 15) | (k >> 17);
                k *= 0x1b873593;
                hash ^= k;
    }

    hash ^= bytes;
    return SkChecksum::Mix(hash);
}
