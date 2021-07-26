/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkChecksum.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkOpts.h"
#include "tests/Test.h"

DEF_TEST(Checksum, r) {
    // Put 128 random bytes into two identical buffers.  Any multiple of 4 will do.
    const size_t kBytes = SkAlign4(128);
    SkRandom rand;
    uint32_t data[kBytes/4], tweaked[kBytes/4];
    for (size_t i = 0; i < SK_ARRAY_COUNT(tweaked); ++i) {
        data[i] = tweaked[i] = rand.nextU();
    }

    // Hash of nullptr is always 0.
    REPORTER_ASSERT(r, SkOpts::hash(nullptr, 0) == 0);

    const uint32_t hash = SkOpts::hash(data, kBytes);
    // Should be deterministic.
    REPORTER_ASSERT(r, hash == SkOpts::hash(data, kBytes));

    // Changing any single element should change the hash.
    for (size_t j = 0; j < SK_ARRAY_COUNT(tweaked); ++j) {
        const uint32_t saved = tweaked[j];
        tweaked[j] = rand.nextU();
        const uint32_t tweakedHash = SkOpts::hash(tweaked, kBytes);
        REPORTER_ASSERT(r, tweakedHash != hash);
        REPORTER_ASSERT(r, tweakedHash == SkOpts::hash(tweaked, kBytes));
        tweaked[j] = saved;
    }
}

DEF_TEST(GoodHash, r) {
    // 4 bytes --> hits SkChecksum::Mix fast path.
    REPORTER_ASSERT(r, SkGoodHash()(( int32_t)4) ==  614249093);
    REPORTER_ASSERT(r, SkGoodHash()((uint32_t)4) ==  614249093);
}

DEF_TEST(ChecksumCollisions, r) {
    // We noticed a few workloads that would cause hash collisions due to the way
    // our optimized hashes split into three concurrent hashes and merge those hashes together.
    //
    // One of these two workloads ought to cause an unintentional hash collision on very similar
    // data in those old algorithms, the float version on 32-bit x86 and double elsewhere.
    {
        float a[9] = { 0, 1, 2,
                       3, 4, 5,
                       6, 7, 8, };
        float b[9] = { 1, 2, 0,
                       4, 5, 3,
                       7, 8, 6, };

        REPORTER_ASSERT(r, SkOpts::hash(a, sizeof(a)) != SkOpts::hash(b, sizeof(b)));
    }
    {
        double a[9] = { 0, 1, 2,
                        3, 4, 5,
                        6, 7, 8, };
        double b[9] = { 1, 2, 0,
                        4, 5, 3,
                        7, 8, 6, };

        REPORTER_ASSERT(r, SkOpts::hash(a, sizeof(a)) != SkOpts::hash(b, sizeof(b)));
    }
}

DEF_TEST(ChecksumConsistent, r) {
    // We've decided to make SkOpts::hash() always return consistent results, so spot check a few:
    uint8_t bytes[256];
    for (int i = 0; i < 256; i++) {
        bytes[i] = i;
    }
    REPORTER_ASSERT(r, SkOpts::hash(bytes,  0) == 0x00000000, "%08x", SkOpts::hash(bytes,  0));
    REPORTER_ASSERT(r, SkOpts::hash(bytes,  1) == 0x00000000, "%08x", SkOpts::hash(bytes,  1));
    REPORTER_ASSERT(r, SkOpts::hash(bytes,  2) == 0xf26b8303, "%08x", SkOpts::hash(bytes,  2));
    REPORTER_ASSERT(r, SkOpts::hash(bytes,  7) == 0x18678721, "%08x", SkOpts::hash(bytes,  7));
    REPORTER_ASSERT(r, SkOpts::hash(bytes, 32) == 0x9d1ef96b, "%08x", SkOpts::hash(bytes, 32));
    REPORTER_ASSERT(r, SkOpts::hash(bytes, 63) == 0xc4b07d3a, "%08x", SkOpts::hash(bytes, 63));
    REPORTER_ASSERT(r, SkOpts::hash(bytes, 64) == 0x3535a461, "%08x", SkOpts::hash(bytes, 64));
    REPORTER_ASSERT(r, SkOpts::hash(bytes, 99) == 0x3f98a130, "%08x", SkOpts::hash(bytes, 99));
    REPORTER_ASSERT(r, SkOpts::hash(bytes,255) == 0x3b9ceab2, "%08x", SkOpts::hash(bytes,255));
}
