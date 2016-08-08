/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChecksum.h"
#include "SkOpts.h"
#include "SkRandom.h"
#include "Test.h"

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
