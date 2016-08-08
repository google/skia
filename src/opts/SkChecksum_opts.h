/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkChecksum_opts_DEFINED
#define SkChecksum_opts_DEFINED

#include "SkChecksum.h"
#include "SkTypes.h"

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE42
    #include <immintrin.h>
#endif

// TODO: ARMv8 has optional CRC instructions similar to SSE 4.2
// TODO: 32-bit x86 version: same sort of idea using only _mm_crc32_u32() and smaller

namespace SK_OPTS_NS {

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE42 && (defined(__x86_64__) || defined(_M_X64))
    template <typename T>
    static inline T unaligned_load(const uint8_t* src) {
        T val;
        memcpy(&val, src, sizeof(val));
        return val;
    }

    static uint32_t hash_fn(const void* vdata, size_t bytes, uint32_t seed) {
        auto data = (const uint8_t*)vdata;

        // _mm_crc32_u64() operates on 64-bit registers, so we use uint64_t for a while.
        uint64_t hash = seed;
        if (bytes >= 24) {
            // We'll create 3 independent hashes, each using _mm_crc32_u64()
            // to hash 8 bytes per step.  Both 3 and independent are important:
            // we can execute 3 of these instructions in parallel on a single core.
            uint64_t a = hash,
                     b = hash,
                     c = hash;
            size_t steps = bytes/24;
            while (steps --> 0) {
                a = _mm_crc32_u64(a, unaligned_load<uint64_t>(data+ 0));
                b = _mm_crc32_u64(b, unaligned_load<uint64_t>(data+ 8));
                c = _mm_crc32_u64(c, unaligned_load<uint64_t>(data+16));
                data += 24;
            }
            bytes %= 24;
            hash = a^b^c;
        }

        SkASSERT(bytes < 24);
        if (bytes >= 16) {
            hash = _mm_crc32_u64(hash, unaligned_load<uint64_t>(data));
            bytes -= 8;
            data  += 8;
        }

        SkASSERT(bytes < 16);
        if (bytes & 8) {
            hash = _mm_crc32_u64(hash, unaligned_load<uint64_t>(data));
            data  += 8;
        }

        // The remainder of these _mm_crc32_u*() operate on a 32-bit register.
        // We don't lose anything here: only the bottom 32-bits were populated.
        auto hash32 = (uint32_t)hash;

        if (bytes & 4) {
            hash32 = _mm_crc32_u32(hash32, unaligned_load<uint32_t>(data));
            data += 4;
        }
        if (bytes & 2) {
            hash32 = _mm_crc32_u16(hash32, unaligned_load<uint16_t>(data));
            data += 2;
        }
        if (bytes & 1) {
            hash32 = _mm_crc32_u8(hash32, unaligned_load<uint8_t>(data));
        }
        return hash32;
    }

#else
    static uint32_t hash_fn(const void* data, size_t bytes, uint32_t seed) {
        // This is Murmur3.

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
#endif

}  // namespace SK_OPTS_NS

#endif//SkChecksum_opts_DEFINED
