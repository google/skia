/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCharToGlyphCache.h"
#include "../private/SkTFitsIn.h"

constexpr uint16_t k16BitSentinel = 0xFFFF;
constexpr uint32_t k32BitSentinel = 0xFFFFFFFF;

static bool use_16bit_keys(uint32_t uni) {
    return uni < k16BitSentinel;
}

template <typename T> int find_index(const T* base, int count, uint32_t value) {
    SkASSERT(count > 0 && value < base[count - 1]);
    int index;
    for (index = 0;; ++index) {
        SkASSERT(index <= count);
        if (value <= base[index]) {
            if (value < base[index]) {
                index = ~index; // not found
            }
            break;
        }
    }
    return index;
}

SkCharToGlyphCache::SkCharToGlyphCache() {
    *fKey16.append() = k16BitSentinel; *fValue16.append() = 0;
    *fKey32.append() = k32BitSentinel; *fValue32.append() = 0;
}

SkCharToGlyphCache::~SkCharToGlyphCache() {}

int SkCharToGlyphCache::findGlyphIndex(SkUnichar unichar) const {
    int index;
    if (use_16bit_keys(unichar)) {
        int count = fKey16.count();
        index = find_index(fKey16.begin(), count, unichar);
        if (index >= 0) {
            return fValue16[index];
        }
    } else {
        int count = fKey32.count();
        index = find_index(fKey32.begin(), count, unichar);
        if (index >= 0) {
            return fValue32[index];
        }
    }
    SkASSERT(index < 0);
    return index;
}

void SkCharToGlyphCache::insertCharAndGlyph(int index, SkUnichar unichar, SkGlyphID glyph) {
    if (use_16bit_keys(unichar)) {
        SkASSERT(fKey16.size() == fValue16.size());
        SkASSERT((unsigned)index <= fKey16.size());

        *fKey16.insert(index) = SkToU16(unichar);
        *fValue16.insert(index) = glyph;
    } else {
        SkASSERT(fKey32.size() == fValue32.size());
        SkASSERT((unsigned)index <= fKey32.size());

        *fKey32.insert(index) = unichar;
        *fValue32.insert(index) = glyph;
    }
    this->validate();
}

void SkCharToGlyphCache::validate() const {
#ifdef SK_DEBUG
    SkASSERT(fKey16.size() == fValue16.size());
    SkASSERT(fKey16.size() > 0 &&
             fKey16.end()[-1] == k16BitSentinel &&
             fValue16.end()[-1] == 0);
    for (int i = 1; i < fKey32.count(); ++i) {
        SkASSERT(fKey16[i - 1] < fKey16[i]);
    }

    SkASSERT(fKey32.size() == fValue32.size());
    SkASSERT(fKey32.size() > 0 &&
             fKey32.end()[-1] == k32BitSentinel &&
             fValue32.end()[-1] == 0);
    for (int i = 1; i < fKey32.count(); ++i) {
        SkASSERT(fKey32[i - 1] < fKey32[i]);
    }
#endif
}
