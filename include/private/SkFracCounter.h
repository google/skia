/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFracCounter_DEFINED
#define SkFracCounter_DEFINED

#include "SkMalloc.h"

// Count the number of points (x, y) in the path with fractional values (either x or y) group by
// the last MASK_BIT bits of the integral part of that value (i.e., int(x) & FRAC_INT_MASK or
// int(y) & FRAC_INT_MASK). If the count is highly concentrated in some groups (i.e., pixel rows
// or columns), it's likely that the DAA/CCPR/SKC would generate some artifacts noticeable to
// human (skbug.com/6886). Hence we may fall back to supersampling or AAA. (See also comments
// for SkPath::isFracCountConcentrated.)
class SkFracCounter {
public:
    static constexpr int    FRAC_INT_MASK_BIT = 6;
    static constexpr int    FRAC_INT_MASK = (1 << FRAC_INT_MASK_BIT) - 1;

    // To save performance, only count points[i] if i % FRAC_SAMPLE_RATE == 0. FRAC_SAMPLE_RATE is a
    // prime number so the sampling could be more uniform when paths have repetitive patterns.
    static constexpr int    FRAC_SAMPLE_RATE = 7;

    SkFracCounter() {
        this->setDirty();
    }

    int maxFracCount() const {
        return fMaxFracCount;
    }

    SK_ALWAYS_INLINE void setDirty() {
        fMaxFracCount = -1;
    }

    int compute(const SkPoint* points, int count) {
        int fracCount[FRAC_INT_MASK + 1];
        sk_bzero(fracCount, sizeof(fracCount));
        for(int i = 0; i < count; i += FRAC_SAMPLE_RATE) {
            AddFracCount(points[i].fX, fracCount);
            AddFracCount(points[i].fY, fracCount);
        }
        return this->computeMax(fracCount);
    }

private:
    static void AddFracCount(SkScalar f, int fracCount[FRAC_INT_MASK + 1]) {
        int i = (int)f;
        if (f == i) {
            return;
        }
        fracCount[i & FRAC_INT_MASK]++;
    }

    int computeMax(int fracCount[FRAC_INT_MASK + 1]) {
        fMaxFracCount = 0;
        for(int i = 0; i < FRAC_INT_MASK + 1; ++i) {
            fMaxFracCount = SkTMax(fMaxFracCount, fracCount[i]);
        }
        return fMaxFracCount;
    }

    int fMaxFracCount;
};

#endif  // SkFracCounter_DEFINED
