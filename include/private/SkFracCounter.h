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
        this->zeroFracCount();
        for(int i = 0; i < count; i += (FRAC_SAMPLE_MASK + 1)) {
            this->addFracCount(points[i].fX);
            this->addFracCount(points[i].fY);
        }
        return this->computeMax();
    }

private:
    void zeroFracCount() {
        sk_bzero(fFracCount, sizeof(fFracCount));
    }

    void addFracCount(SkScalar f) {
        int i = (int)f;
        if (f == i) {
            return;
        }
        fFracCount[i & FRAC_INT_MASK]++;
    }

    int computeMax() {
        fMaxFracCount = 0;
        for(int i = 0; i < FRAC_INT_MASK + 1; ++i) {
            fMaxFracCount = SkTMax(fMaxFracCount, fFracCount[i]);
        }
        return fMaxFracCount;
    }

    static constexpr int    FRAC_INT_MASK_BIT = 6;
    static constexpr int    FRAC_INT_MASK = (1 << FRAC_INT_MASK_BIT) - 1;

    // To save performance, only count points[i] where i & FRAC_SAMPLE_MASK == 0
    static constexpr int    FRAC_SAMPLE_MASK = (1 << 3) - 1;

    int                     fFracCount[FRAC_INT_MASK + 1];
    int                     fMaxFracCount;

    friend class SkPathRef;
};

#endif  // SkFracCounter_DEFINED
