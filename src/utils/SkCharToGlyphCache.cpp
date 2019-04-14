/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCharToGlyphCache.h"
#include "../private/SkTFitsIn.h"

#define NEW_WAY

// TODO: consider faster ways to search big arrays
//       bsearch
//       slope-based (since we can compute a slope from first and last values)
//
//       Also, consider adding sentinels to key arrays so we don't have to check that
//       index is in rage.
//
template <typename T> int find_index(const T* base, int count, T value) {
    for (int index = 0; index < count; ++index) {
        if (value <= base[index]) {
            if (value < base[index]) {
                index = ~index; // not found
            }
            return index;
        }
    }
    return ~count;    // append at the end
}

SkCharToGlyphCache::SkCharToGlyphCache() {
    *fK32.append() = 0x80000000;    *fV16.append() = 0;
    *fK32.append() = 0x7FFFFFFF;    *fV16.append() = 0;
}

SkCharToGlyphCache::~SkCharToGlyphCache() {}

int SkCharToGlyphCache::findGlyphIndex(SkUnichar unichar) const {
#ifdef NEW_WAY
    return this->findGlyphIndex2(unichar);
#endif
    int index;
    if (SkTFitsIn<uint16_t>(unichar)) {
        int count = f1616.count() >> 1;
        index = find_index(f1616.begin(), count, SkToU16(unichar));
        if (index >= 0) {
            return f1616[count + index];
        }
    } else {
        int count = fKey32.count();
        index = find_index(fKey32.begin(), count, unichar);
        if (index >= 0) {
            return fValue16[index];
        }
    }
    SkASSERT(index < 0);
    return index;
}

void SkCharToGlyphCache::insertCharAndGlyph(int index, SkUnichar unichar, SkGlyphID glyph) {
#ifdef NEW_WAY
    this->insertCharAndGlyph2(index, unichar, glyph);
    return;
#endif

    if (SkTFitsIn<uint16_t>(unichar)) {
        // count is the logical count, since our array is really two arrays back-to-back
        size_t count = f1616.size() >> 1;
        SkASSERT((unsigned)index <= count);

        f1616.setCount(f1616.size() + 2); // make room for key and value
        uint16_t* base = f1616.begin();

        // now slide twice, to make room for the new key and value
        uint16_t* src = base + count + index;
        size_t    amt = count - index;
        sk_careful_memmove(src + 2, src, amt * sizeof(uint16_t));

        src = base + index;
        amt = count;
        sk_careful_memmove(src + 1, src, amt * sizeof(uint16_t));

        // now store the new values
        base[index] = SkToU16(unichar);
        base[count + 1 + index] = glyph;
    } else {
        SkASSERT(fKey32.size() == fValue16.size());
        SkASSERT((unsigned)index <= fKey32.size());

        *fKey32.insert(index) = unichar;
        *fValue16.insert(index) = glyph;
    }
    this->validate();
}

void SkCharToGlyphCache::validate() const {
#ifdef SK_DEBUG
    SkASSERT((f1616.count() & 1) == 0);
    for (int i = 1; i < f1616.count() >> 1; ++i) {
        SkASSERT(f1616[i - 1] < f1616[i]);
    }

    SkASSERT(fKey32.size() == fValue16.size());
    for (int i = 1; i < fKey32.count(); ++i) {
        SkASSERT(fKey32[i - 1] < fKey32[i]);
    }
#endif
}

//////////////////////////////////////////////////////////////////////////////////////////////////

constexpr int kSmallCountLimit = 16;

static int find_small(const SkUnichar base[], int count, SkUnichar value) {
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

static int find_big(const SkUnichar base[], int count, SkUnichar value) {
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
        index = (count - 2) * (value - base[1]) / (base[count - 2] - base[1]);
        SkASSERT(index >= 1 && index < count - 1);
        if (value > base[index]) {
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

int SkCharToGlyphCache::findGlyphIndex2(SkUnichar unichar) const {
    int count = fK32.count();
    int index;
    if (count <= kSmallCountLimit) {
        index = find_small(fK32.begin(), count, unichar);
    } else {
        index = find_big(fK32.begin(), count, unichar);
    }
    if (index >= 0) {
        return fV16[index];
    }
    return index;
}

void SkCharToGlyphCache::insertCharAndGlyph2(int index, SkUnichar unichar, SkGlyphID glyph) {
    SkASSERT(fK32.size() == fV16.size());
    SkASSERT((unsigned)index < fK32.size());
    SkASSERT(unichar < fK32[index]);

    *fK32.insert(index) = unichar;
    *fV16.insert(index) = glyph;

#ifdef SK_DEBUG
    for (int i = 1; i < fK32.count(); ++i) {
        SkASSERT(fK32[i-1] < fK32[i]);
    }
#endif
}
