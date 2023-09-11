/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkAlign.h"
#include "src/base/SkRandom.h"
#include "src/core/SkChecksum.h"
#include "tests/Test.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

DEF_TEST(Checksum, r) {
    // Put 128 random bytes into two identical buffers.  Any multiple of 4 will do.
    const size_t kBytes = SkAlign4(128);
    SkRandom rand;
    uint32_t data[kBytes/4], tweaked[kBytes/4];
    for (size_t i = 0; i < std::size(tweaked); ++i) {
        data[i] = tweaked[i] = rand.nextU();
    }

    const uint32_t hash = SkChecksum::Hash32(data, kBytes);
    // Should be deterministic.
    REPORTER_ASSERT(r, hash == SkChecksum::Hash32(data, kBytes));

    // Changing any single element should change the hash.
    for (size_t j = 0; j < std::size(tweaked); ++j) {
        const uint32_t saved = tweaked[j];
        tweaked[j] = rand.nextU();
        const uint32_t tweakedHash = SkChecksum::Hash32(tweaked, kBytes);
        REPORTER_ASSERT(r, tweakedHash != hash);
        REPORTER_ASSERT(r, tweakedHash == SkChecksum::Hash32(tweaked, kBytes));
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
    // our old optimized hashes split into three concurrent hashes and merged those hashes together.
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

        REPORTER_ASSERT(r, SkChecksum::Hash32(a, sizeof(a)) != SkChecksum::Hash32(b, sizeof(b)));
    }
    {
        double a[9] = { 0, 1, 2,
                        3, 4, 5,
                        6, 7, 8, };
        double b[9] = { 1, 2, 0,
                        4, 5, 3,
                        7, 8, 6, };

        REPORTER_ASSERT(r, SkChecksum::Hash32(a, sizeof(a)) != SkChecksum::Hash32(b, sizeof(b)));
    }
}

DEF_TEST(ChecksumConsistent, r) {
    // We don't guarantee that SkChecksum::Hash32 will return consistent results, but it does today.
    // Spot check a few:
    uint8_t bytes[256];
    for (int i = 0; i < 256; i++) {
        bytes[i] = i;
    }
    auto hash_bytes = [&](int n) { return SkChecksum::Hash32(bytes, n); };
    REPORTER_ASSERT(r, hash_bytes(  0) == 0xe2bde459, "%08x", hash_bytes(  0));
    REPORTER_ASSERT(r, hash_bytes(  1) == 0xe5f8bd85, "%08x", hash_bytes(  1));
    REPORTER_ASSERT(r, hash_bytes(  2) == 0x77acd42a, "%08x", hash_bytes(  2));
    REPORTER_ASSERT(r, hash_bytes(  7) == 0x78d0861f, "%08x", hash_bytes(  7));
    REPORTER_ASSERT(r, hash_bytes( 32) == 0x4e73df6d, "%08x", hash_bytes( 32));
    REPORTER_ASSERT(r, hash_bytes( 63) == 0x5e66a3f4, "%08x", hash_bytes( 63));
    REPORTER_ASSERT(r, hash_bytes( 64) == 0x962d6746, "%08x", hash_bytes( 64));
    REPORTER_ASSERT(r, hash_bytes( 99) == 0x79e09416, "%08x", hash_bytes( 99));
    REPORTER_ASSERT(r, hash_bytes(255) == 0x85f837f0, "%08x", hash_bytes(255));
}

DEF_TEST(ChecksumStrings, r) {
    constexpr char kMessage[] = "Checksums are supported for SkString, string, and string_view.";
    const uint32_t expectedHash = SkChecksum::Hash32(kMessage, strlen(kMessage));

    REPORTER_ASSERT(r, expectedHash == SkGoodHash()(SkString(kMessage)));
    REPORTER_ASSERT(r, expectedHash == SkGoodHash()(std::string(kMessage)));
    REPORTER_ASSERT(r, expectedHash == SkGoodHash()(std::string_view(kMessage)));
}
