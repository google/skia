/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCharToGlyphCache.h"
#include "../private/SkTFitsIn.h"

SkCharToGlyphCache::SkCharToGlyphCache() {
    this->reset();
}

SkCharToGlyphCache::~SkCharToGlyphCache() {}

void SkCharToGlyphCache::reset() {
    fK32.reset();
    fV16.reset();

    // Add sentinels so we can always rely on these to stop linear searches (in either direction)
    // Neither is a legal unichar, so we don't care what glyphID we use.
    //
    *fK32.append() = 0x80000000;    *fV16.append() = 0;
    *fK32.append() = 0x7FFFFFFF;    *fV16.append() = 0;

    fDenom = 0;
}

// Determined experimentally. For N much larger, the slope technique is faster.
// For N much smaller, a simple search is faster.
//
constexpr int kSmallCountLimit = 16;

// To use slope technique we need at least 2 real entries (+2 sentinels) hence the min of 4
//
constexpr int kMinCountForSlope = 4;

static int find_simple(const SkUnichar base[], int count, SkUnichar value) {
    int index;
    for (index = 0;; ++index) {
        if (value <= base[index]) {
            if (value < base[index]) {
                index = ~index; // not found
            }
            break;
        }
    }
    return index;
}

static int find_with_slope(const SkUnichar base[], int count, SkUnichar value, double denom) {
    SkASSERT(count >= kMinCountForSlope);

    int index;
    if (value <= base[1]) {
        index = 1;
        if (value < base[index]) {
            index = ~index;
        }
    } else if (value >= base[count - 2]) {
        index = count - 2;
        if (value > base[index]) {
            index = ~(index + 1);
        }
    } else {
        // make our guess based on the "slope" of the current values
//        index = 1 + (int64_t)(count - 2) * (value - base[1]) / (base[count - 2] - base[1]);
        index = 1 + (int)(denom * (count - 2) * (value - base[1]));
        SkASSERT(index >= 1 && index <= count - 2);

        if (value >= base[index]) {
            for (;; ++index) {
                if (value <= base[index]) {
                    if (value < base[index]) {
                        index = ~index; // not found
                    }
                    break;
                }
            }
        } else {
            for (--index;; --index) {
                SkASSERT(index >= 0);
                if (value >= base[index]) {
                    if (value > base[index]) {
                        index = ~(index + 1);
                    }
                    break;
                }
            }
        }
    }
    return index;
}

int SkCharToGlyphCache::findGlyphIndex(SkUnichar unichar) const {
    const int count = fK32.count();
    int index;
    if (count <= kSmallCountLimit) {
        index = find_simple(fK32.begin(), count, unichar);
    } else {
        index = find_with_slope(fK32.begin(), count, unichar, fDenom);
    }
    if (index >= 0) {
        return fV16[index];
    }
    return index;
}

void SkCharToGlyphCache::insertCharAndGlyph(int index, SkUnichar unichar, SkGlyphID glyph) {
    SkASSERT(fK32.size() == fV16.size());
    SkASSERT((unsigned)index < fK32.size());
    SkASSERT(unichar < fK32[index]);

    *fK32.insert(index) = unichar;
    *fV16.insert(index) = glyph;

    // if we've changed the first [1] or last [count-2] entry, recompute our slope
    const int count = fK32.count();
    if (count >= kMinCountForSlope && (index == 1 || index == count - 2)) {
        SkASSERT(index >= 1 && index <= count - 2);
        fDenom = 1.0 / ((double)fK32[count - 2] - fK32[1]);
    }

#ifdef SK_DEBUG
    for (int i = 1; i < fK32.count(); ++i) {
        SkASSERT(fK32[i-1] < fK32[i]);
    }
#endif
}
