/*
 * Copyright 2018 Google, LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef FuzzCommon_DEFINED
#define FuzzCommon_DEFINED

#include "Fuzz.h"
#include "SkBlendMode.h"
#include "SkFilterQuality.h"
#include "SkImageFilter.h"
#include "SkMatrix.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkRegion.h"
#include "SkShader.h"

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

// Any overrides of the Fuzz::next behavior should go here.
// If they go in other files, they won't necessarily be shared with the
// oss-fuzz build, making things sometimes not reproduce reliably.

template <>
inline void Fuzz::next(SkRegion* region) { fuzz_region(this, region, 10); }

template <>
inline void Fuzz::next(SkImageFilter::CropRect* cropRect) {
    SkRect rect;
    uint8_t flags;
    this->next(&rect);
    this->nextRange(&flags, 0, 0xF);
    *cropRect = SkImageFilter::CropRect(rect, flags);
}

template <>
inline void Fuzz::next(SkShader::TileMode* m) {
    fuzz_enum_range(this, m, 0, SkShader::TileMode::kLast_TileMode);
}

template <>
inline void Fuzz::next(SkFilterQuality* q) {
    fuzz_enum_range(this, q, 0, SkFilterQuality::kLast_SkFilterQuality);
}

template <>
inline void Fuzz::next(SkBlendMode* mode) {
    fuzz_enum_range(this, mode, 0, SkBlendMode::kLastMode);
}

// allows some float values for path points
void FuzzPath(Fuzz* fuzz, SkPath* path, int maxOps);
// allows all float values for path points
void BuildPath(Fuzz* fuzz, SkPath* path, int last_verb);

void fuzz_rrect(Fuzz* fuzz, SkRRect* rr);

void fuzz_matrix(Fuzz* fuzz, SkMatrix* m);

#endif

