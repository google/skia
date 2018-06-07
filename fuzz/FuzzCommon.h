/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FuzzCommon_DEFINED
#define FuzzCommon_DEFINED

#include "Fuzz.h"
#include "SkPath.h"
#include "SkRegion.h"

// We don't always want to test NaNs and infinities.
static inline void fuzz_nice_float(Fuzz* fuzz, float* f) {
    float v;
    fuzz->next(&v);
    constexpr float kLimit = 1.0e35f;  // FLT_MAX?
    *f = (v == v && v <= kLimit && v >= -kLimit) ? v : 0.0f;
}

template <typename... Args>
inline void fuzz_nice_float(Fuzz* fuzz, float* f, Args... rest) {
    fuzz_nice_float(fuzz, f);
    fuzz_nice_float(fuzz, rest...);
}

template <typename T, typename Min, typename Max>
inline void fuzz_enum_range(Fuzz* fuzz, T* value, Min rmin, Max rmax) {
    using U = skstd::underlying_type_t<T>;
    fuzz->nextRange((U*)value, (U)rmin, (U)rmax);
}

inline void fuzz_region(Fuzz* fuzz, SkRegion* region, int maxN) {
    uint8_t N;
    fuzz->nextRange(&N, 0, maxN);
    for (uint8_t i = 0; i < N; ++i) {
        SkIRect r;
        SkRegion::Op op;
        // Avoid the sentinal value used by Region.
        fuzz->nextRange(&r.fLeft,   -2147483646, 2147483646);
        fuzz->nextRange(&r.fTop,    -2147483646, 2147483646);
        fuzz->nextRange(&r.fRight,  -2147483646, 2147483646);
        fuzz->nextRange(&r.fBottom, -2147483646, 2147483646);
        r.sort();
        fuzz_enum_range(fuzz, &op, (SkRegion::Op)0, SkRegion::kLastOp);
        if (!region->op(r, op)) {
            return;
        }
    }
}

template <>
inline void Fuzz::next(SkRegion* region) { fuzz_region(this, region, 10); }

// allows some float values for path points
void FuzzPath(Fuzz* fuzz, SkPath* path, int maxOps);
// allows all float values for path points
void BuildPath(Fuzz* fuzz, SkPath* path, int last_verb);

#endif

