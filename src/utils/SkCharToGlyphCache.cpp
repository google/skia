/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkCharToGlyphCache.h"

SkCharToGlyphCache::SkCharToGlyphCache() {
    this->reset();
}

SkCharToGlyphCache::~SkCharToGlyphCache() {}

void SkCharToGlyphCache::reset() {
    fKUnichar.reset();
    fVGlyph.reset();

    // Add sentinels so we can always rely on these to stop linear searches (in either direction)
    // Neither is a legal unichar, so we don't care what glyphID we use.
    //
    *fKUnichar.append() = 0x80000000;    *fVGlyph.append() = 0;
    *fKUnichar.append() = 0x7FFFFFFF;    *fVGlyph.append() = 0;

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
    const int count = fKUnichar.size();
    int index;
    if (count <= kSmallCountLimit) {
        index = find_simple(fKUnichar.begin(), count, unichar);
    } else {
        index = find_with_slope(fKUnichar.begin(), count, unichar, fDenom);
    }
    if (index >= 0) {
        return fVGlyph[index];
    }
    return index;
}

void SkCharToGlyphCache::insertCharAndGlyph(int index, SkUnichar unichar, SkGlyphID glyph) {
    SkASSERT(fKUnichar.size() == fVGlyph.size());
    SkASSERT(index < fKUnichar.size());
    SkASSERT(unichar < fKUnichar[index]);

    *fKUnichar.insert(index) = unichar;
    *fVGlyph.insert(index) = glyph;

    // if we've changed the first [1] or last [count-2] entry, recompute our slope
    const int count = fKUnichar.size();
    if (count >= kMinCountForSlope && (index == 1 || index == count - 2)) {
        SkASSERT(index >= 1 && index <= count - 2);
        fDenom = 1.0 / ((double)fKUnichar[count - 2] - fKUnichar[1]);
    }

#ifdef SK_DEBUG
    for (int i = 1; i < fKUnichar.size(); ++i) {
        SkASSERT(fKUnichar[i-1] < fKUnichar[i]);
    }
#endif
}
