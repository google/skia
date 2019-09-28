/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkChecksum_opts_DEFINED
#define SkChecksum_opts_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkChecksum.h"
#include "src/core/SkUtils.h"   // sk_unaligned_load

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE42
    #include <immintrin.h>
#elif defined(SK_ARM_HAS_CRC32)
    #include <arm_acle.h>
#endif

namespace SK_OPTS_NS {

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE42 && (defined(__x86_64__) || defined(_M_X64))
    // This is not a CRC32.  It's Just A Hash that uses those instructions because they're fast.
    /*not static*/ inline uint32_t hash_fn(const void* vdata, size_t bytes, uint32_t seed) {
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
                a = _mm_crc32_u64(a, sk_unaligned_load<uint64_t>(data+ 0));
                b = _mm_crc32_u64(b, sk_unaligned_load<uint64_t>(data+ 8));
                c = _mm_crc32_u64(c, sk_unaligned_load<uint64_t>(data+16));
                data += 24;
            }
            bytes %= 24;
            hash = _mm_crc32_u32(a, _mm_crc32_u32(b, c));
        }

        SkASSERT(bytes < 24);
        if (bytes >= 16) {
            hash = _mm_crc32_u64(hash, sk_unaligned_load<uint64_t>(data));
            bytes -= 8;
            data  += 8;
        }

        SkASSERT(bytes < 16);
        if (bytes & 8) {
            hash = _mm_crc32_u64(hash, sk_unaligned_load<uint64_t>(data));
            data  += 8;
        }

        // The remainder of these _mm_crc32_u*() operate on a 32-bit register.
        // We don't lose anything here: only the bottom 32-bits were populated.
        auto hash32 = (uint32_t)hash;

        if (bytes & 4) {
            hash32 = _mm_crc32_u32(hash32, sk_unaligned_load<uint32_t>(data));
            data += 4;
        }
        if (bytes & 2) {
            hash32 = _mm_crc32_u16(hash32, sk_unaligned_load<uint16_t>(data));
            data += 2;
        }
        if (bytes & 1) {
            hash32 = _mm_crc32_u8(hash32, sk_unaligned_load<uint8_t>(data));
        }
        return hash32;
    }

#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE42
    // 32-bit version of above, using _mm_crc32_u32() but not _mm_crc32_u64().
    /*not static*/ inline uint32_t hash_fn(const void* vdata, size_t bytes, uint32_t hash) {
        auto data = (const uint8_t*)vdata;

        if (bytes >= 12) {
            // We'll create 3 independent hashes, each using _mm_crc32_u32()
            // to hash 4 bytes per step.  Both 3 and independent are important:
            // we can execute 3 of these instructions in parallel on a single core.
            uint32_t a = hash,
                     b = hash,
                     c = hash;
            size_t steps = bytes/12;
            while (steps --> 0) {
                a = _mm_crc32_u32(a, sk_unaligned_load<uint32_t>(data+0));
                b = _mm_crc32_u32(b, sk_unaligned_load<uint32_t>(data+4));
                c = _mm_crc32_u32(c, sk_unaligned_load<uint32_t>(data+8));
                data += 12;
            }
            bytes %= 12;
            hash = _mm_crc32_u32(a, _mm_crc32_u32(b, c));
        }

        SkASSERT(bytes < 12);
        if (bytes >= 8) {
            hash = _mm_crc32_u32(hash, sk_unaligned_load<uint32_t>(data));
            bytes -= 4;
            data  += 4;
        }

        SkASSERT(bytes < 8);
        if (bytes & 4) {
            hash = _mm_crc32_u32(hash, sk_unaligned_load<uint32_t>(data));
            data += 4;
        }
        if (bytes & 2) {
            hash = _mm_crc32_u16(hash, sk_unaligned_load<uint16_t>(data));
            data += 2;
        }
        if (bytes & 1) {
            hash = _mm_crc32_u8(hash, sk_unaligned_load<uint8_t>(data));
        }
        return hash;
    }

#elif defined(SK_ARM_HAS_CRC32)
    /*not static*/ inline uint32_t hash_fn(const void* vdata, size_t bytes, uint32_t hash) {
        auto data = (const uint8_t*)vdata;
        if (bytes >= 24) {
            uint32_t a = hash,
                     b = hash,
                     c = hash;
            size_t steps = bytes/24;
            while (steps --> 0) {
                a = __crc32d(a, sk_unaligned_load<uint64_t>(data+ 0));
                b = __crc32d(b, sk_unaligned_load<uint64_t>(data+ 8));
                c = __crc32d(c, sk_unaligned_load<uint64_t>(data+16));
                data += 24;
            }
            bytes %= 24;
            hash = __crc32w(a, __crc32w(b, c));
        }

        SkASSERT(bytes < 24);
        if (bytes >= 16) {
            hash = __crc32d(hash, sk_unaligned_load<uint64_t>(data));
            bytes -= 8;
            data  += 8;
        }

        SkASSERT(bytes < 16);
        if (bytes & 8) {
            hash = __crc32d(hash, sk_unaligned_load<uint64_t>(data));
            data += 8;
        }
        if (bytes & 4) {
            hash = __crc32w(hash, sk_unaligned_load<uint32_t>(data));
            data += 4;
        }
        if (bytes & 2) {
            hash = __crc32h(hash, sk_unaligned_load<uint16_t>(data));
            data += 2;
        }
        if (bytes & 1) {
            hash = __crc32b(hash, sk_unaligned_load<uint8_t>(data));
        }
        return hash;
    }

#else
    // This is Murmur3.
    /*not static*/ inline uint32_t hash_fn(const void* vdata, size_t bytes, uint32_t hash) {
        auto data = (const uint8_t*)vdata;

        size_t original_bytes = bytes;

        // Handle 4 bytes at a time while possible.
        while (bytes >= 4) {
            uint32_t k = sk_unaligned_load<uint32_t>(data);
            k *= 0xcc9e2d51;
            k = (k << 15) | (k >> 17);
            k *= 0x1b873593;

            hash ^= k;
            hash = (hash << 13) | (hash >> 19);
            hash *= 5;
            hash += 0xe6546b64;

            bytes -= 4;
            data  += 4;
        }

        // Handle last 0-3 bytes.
        uint32_t k = 0;
        switch (bytes & 3) {
            case 3: k ^= data[2] << 16;
            case 2: k ^= data[1] <<  8;
            case 1: k ^= data[0] <<  0;
                    k *= 0xcc9e2d51;
                    k = (k << 15) | (k >> 17);
                    k *= 0x1b873593;
                    hash ^= k;
        }

        hash ^= original_bytes;
        return SkChecksum::Mix(hash);
    }
#endif

}  // namespace SK_OPTS_NS

#endif//SkChecksum_opts_DEFINED
