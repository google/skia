/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkChecksum.h"
#include "SkRandom.h"
#include "Test.h"


// Murmur3 has an optional third seed argument, so we wrap it to fit a uniform type.
static uint32_t murmur_noseed(const uint32_t* d, size_t l) { return SkChecksum::Murmur3(d, l); }

#define ASSERT(x) REPORTER_ASSERT(r, x)

DEF_TEST(Checksum, r) {
    // Algorithms to test.  They're currently all uint32_t(const uint32_t*, size_t).
    typedef uint32_t(*algorithmProc)(const uint32_t*, size_t);
    const algorithmProc kAlgorithms[] = { &SkChecksum::Compute, &murmur_noseed };

    // Put 128 random bytes into two identical buffers.  Any multiple of 4 will do.
    const size_t kBytes = SkAlign4(128);
    SkRandom rand;
    uint32_t data[kBytes/4], tweaked[kBytes/4];
    for (size_t i = 0; i < SK_ARRAY_COUNT(tweaked); ++i) {
        data[i] = tweaked[i] = rand.nextU();
    }

    // Test each algorithm.
    for (size_t i = 0; i < SK_ARRAY_COUNT(kAlgorithms); ++i) {
        const algorithmProc algorithm = kAlgorithms[i];

        // Hash of NULL is always 0.
        ASSERT(algorithm(NULL, 0) == 0);

        const uint32_t hash = algorithm(data, kBytes);
        // Should be deterministic.
        ASSERT(hash == algorithm(data, kBytes));

        // Changing any single element should change the hash.
        for (size_t j = 0; j < SK_ARRAY_COUNT(tweaked); ++j) {
            const uint32_t saved = tweaked[j];
            tweaked[j] = rand.nextU();
            const uint32_t tweakedHash = algorithm(tweaked, kBytes);
            ASSERT(tweakedHash != hash);
            ASSERT(tweakedHash == algorithm(tweaked, kBytes));
            tweaked[j] = saved;
        }
    }
}

DEF_TEST(GoodHash, r) {
    ASSERT(SkGoodHash(( int32_t)4) ==  614249093);  // 4 bytes.  Hits SkChecksum::Mix fast path.
    ASSERT(SkGoodHash((uint32_t)4) ==  614249093);  // (Ditto)

    // None of these are 4 byte sized, so they use SkChecksum::Murmur3, not SkChecksum::Mix.
    ASSERT(SkGoodHash((uint64_t)4) == 3491892518);
    ASSERT(SkGoodHash((uint16_t)4) ==  899251846);
    ASSERT(SkGoodHash( (uint8_t)4) ==  962700458);

    // Tests SkString is correctly specialized.
    ASSERT(SkGoodHash(SkString("Hi")) == 55667557);
}
