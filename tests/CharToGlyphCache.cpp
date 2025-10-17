/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkTo.h"
#include "src/utils/SkCharToGlyphCache.h"
#include "tests/Test.h"

static SkGlyphID hash_to_glyph(uint32_t value) {
    return SkToU16(((value >> 16) ^ value) & 0xFFFF);
}

namespace {
    class UnicharGen {
        SkUnichar fU;
        const int fStep;
    public:
        UnicharGen(int step) : fU(0), fStep(step) {}

        SkUnichar next() {
            fU += fStep;
            return fU;
        }
    };
}  // namespace

DEF_TEST(chartoglyph_cache, reporter) {
    SkCharToGlyphCache cache;
    const int step = 3;

    UnicharGen gen(step);
    for (int i = 0; i < 500; ++i) {
        SkUnichar c = gen.next();
        SkGlyphID glyph = hash_to_glyph(c);

        int index = cache.findGlyphIndex(c);
        if (index >= 0) {
            index = cache.findGlyphIndex(c);
        }
        REPORTER_ASSERT(reporter, index < 0);
        cache.insertCharAndGlyph(~index, c, glyph);

        UnicharGen gen2(step);
        for (int j = 0; j <= i; ++j) {
            c = gen2.next();
            glyph = hash_to_glyph(c);
            index = cache.findGlyphIndex(c);
            if ((unsigned)index != glyph) {
                index = cache.findGlyphIndex(c);
            }
            REPORTER_ASSERT(reporter, (unsigned)index == glyph);
        }
    }
}
