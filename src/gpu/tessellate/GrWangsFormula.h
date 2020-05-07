/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrWangsFormula_DEFINED
#define GrWangsFormula_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/private/SkNx.h"
#include "src/gpu/tessellate/GrVectorXform.h"
#include <cmath>
#include <tuple>

// Wang's formulas for cubics and quadratics (1985) give us the minimum number of evenly spaced (in
// the parametric sense) line segments that a curve must be chopped into in order to guarantee all
// lines stay within a distance of "1/intolerance" pixels from the true curve.
namespace GrWangsFormula {

float length(const Sk2f&);
int nextlog2(float);

// Returns the minimum number of evenly spaced (in the parametric sense) line segments that the
// quadratic must be chopped into in order to guarantee all lines stay within a distance of
// "1/intolerance" pixels from the true curve.
inline float quadratic(float intolerance, const SkPoint pts[]) {
    auto [p0, p1, p2] = std::make_tuple(Sk2f::Load(pts), Sk2f::Load(pts+1), Sk2f::Load(pts+2));
    float k = intolerance * .25f;
    return std::sqrt(k * length(p0 - p1*2 + p2));
}

// Returns the minimum number of evenly spaced (in the parametric sense) line segments that the
// cubic must be chopped into in order to guarantee all lines stay within a distance of
// "1/intolerance" pixels from the true curve.
inline float cubic(float intolerance, const SkPoint pts[]) {
    auto [p0, p1, p2, p3] = std::make_tuple(
            Sk2f::Load(pts), Sk2f::Load(pts+1), Sk2f::Load(pts+2), Sk2f::Load(pts+3));
    float k = intolerance * .75f;
    return std::sqrt(k * length(Sk2f::Max((p0 - p1*2 + p2).abs(),
                                          (p1 - p2*2 + p3).abs())));
}

// Returns the minimum log2 number of evenly spaced (in the parametric sense) line segments that the
// transformed quadratic must be chopped into in order to guarantee all lines stay within a distance
// of "1/intolerance" pixels from the true curve.
inline int quadratic_log2(float intolerance, const SkPoint pts[],
                          const GrVectorXform& vectorXform = GrVectorXform()) {
    auto [p0, p1, p2] = std::make_tuple(Sk2f::Load(pts), Sk2f::Load(pts+1), Sk2f::Load(pts+2));
    Sk2f v = p0 + p1*-2 + p2;
    v = vectorXform(v);
    Sk2f vv = v*v;
    float k = intolerance * .25f;
    float f = k*k * (vv[0] + vv[1]);
    return (nextlog2(f) + 3) >> 2;  // ceil(log2(sqrt(sqrt(f))))
}

// Returns the minimum log2 number of evenly spaced (in the parametric sense) line segments that the
// transformed cubic must be chopped into in order to guarantee all lines stay within a distance of
// "1/intolerance" pixels from the true curve.
inline int cubic_log2(float intolerance, const SkPoint pts[],
                      const GrVectorXform& vectorXform = GrVectorXform()) {
    auto [p01, p12, p23] = std::make_tuple(Sk4f::Load(pts), Sk4f::Load(pts+1), Sk4f::Load(pts+2));
    Sk4f v = p01 + p12*-2 + p23;
    v = vectorXform(v);
    Sk4f vv = v*v;
    vv = Sk4f::Max(vv, SkNx_shuffle<2,3,0,1>(vv));
    float k = intolerance * .75f;
    float f = k*k * (vv[0] + vv[1]);
    return (nextlog2(f) + 3) >> 2;  // ceil(log2(sqrt(sqrt(f))))
}

inline float length(const Sk2f& n) {
    Sk2f nn = n*n;
    return std::sqrt(nn[0] + nn[1]);
}

#if defined(__GNUC__) || defined(__clang__)

constexpr static bool has_fast_builtin_clz64() {
    return sizeof(void*) >= 8 && (sizeof(long) == 8 || sizeof(long long) == 8);
}

inline int builtin_clz64(int64_t mask) {
    if constexpr (sizeof(long) == 8) {
        return __builtin_clzl(mask);
    } else if constexpr (sizeof(long long) == 8) {
        return __builtin_clzll(mask);
    }
    SkUNREACHABLE;
}

constexpr static bool has_fast_builtin_clz32() {
    return sizeof(void*) >= 4 && (sizeof(int) == 4 || sizeof(long) == 4);
}

inline int builtin_clz32(int32_t mask) {
    if constexpr (sizeof(int) == 4) {
        return __builtin_clz(mask);
    } else if constexpr (sizeof(long) == 4) {
        return __builtin_clzl(mask);
    }
    SkUNREACHABLE;
}

#else

constexpr static bool has_fast_builtin_clz64() { return false; }
constexpr static bool has_fast_builtin_clz32() { return false; }
inline int builtin_clz64(int64_t mask) { SkUNREACHABLE; }
inline int builtin_clz32(int64_t mask) { SkUNREACHABLE; }

#endif

// Returns approximately ceil(value) - 1.
// Might return ceil(value) - 2 if 0 < value - floor(value) < epsilon.
// Returns 0 if value == 0.
template<typename T> int64_t approx_ceil_minus_1(float value) {
    constexpr float _2invpow23 = 1.f / (8.f * 1024.f * 1024.f);
    // Take advantage of the property that ceil(value) - 1 ~= floor(value - epsilon)
    // This also works out to return 0 when value == 0.
    return static_cast<T>(value * -_2invpow23 + value);
}

// Returns the log2 of the provided value, were that value to be rounded up to the next power of 2.
// Returns 0 if value == 0:
//
//     nextlog2([0..1]) -> 0
//     nextlog2((1..2]) -> 1
//     nextlog2((2..4]) -> 2
//     nextlog2((4..8]) -> 3
//
// It is undefined to pass value < 0.
inline int nextlog2(float value) {
    SkASSERT(value >= 0);

    if constexpr (has_fast_builtin_clz64()) {
        constexpr float _2pow63 = 8.f * 1024.f * 1024.f * 1024.f * 1024.f * 1024.f * 1024.f;
        if (value < _2pow63) {
            int64_t mask = approx_ceil_minus_1<int64_t>(value);
            SkASSERT(mask >= 0); // NOTE: if value == 0 then approx_ceil_minus_1 == 0.
            return (mask) ? 64 - builtin_clz64(mask) : 0;
        }
    } else if constexpr (has_fast_builtin_clz32()) {
        constexpr float _2pow31 = 2.f * 1024.f * 1024.f * 1024.f;
        if (value < _2pow31) {
            int32_t mask = approx_ceil_minus_1<int32_t>(value);
            SkASSERT(mask >= 0); // NOTE: if value == 0 then approx_ceil_minus_1 == 0.
            return (mask) ? 32 - builtin_clz32(mask) : 0;
        }
    }

    int exp;
    if (std::frexp(value, &exp) == .5f) {
        --exp;  // This is an exact power of 2. Don't round up: "0.5 * 2^(exp)" == "2^(exp - 1)".
    }
    return std::max(exp, 0);
}

}  // namespace

#endif
