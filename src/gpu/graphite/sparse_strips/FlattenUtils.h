/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_FlattenUtils_DEFINED
#define skgpu_graphite_sparse_strips_FlattenUtils_DEFINED

#include "include/core/SkPoint.h"
#include "include/private/SkAttributes.h"
#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/tessellate/WangsFormula.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <type_traits>

/*
 * Curve helpers shared by the flattening passes of sparse strips. They are inline and header-only,
 * so that each pass compiles them into its own loops, and they use Flatten's limits (see
 * Flatten.h). Error tolerances are parameters, so that each pass can budget its own.
 */
namespace skgpu::graphite::flatten_utils {

/*
 * Levien's approximations of the integral that maps a quadratic, projected onto the parabola
 * y = x^2, into uniform arc-length space, and of its inverse (see Flatten.h). The scalar versions
 * compute in the precision of their argument.
 */
template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
SK_ALWAYS_INLINE T approx_parabola_integral(T x) {
    constexpr T kD = static_cast<T>(0.67);
    constexpr T kDPow4 = kD * kD * kD * kD;
    return x /
           (static_cast<T>(1) - kD + std::sqrt(std::sqrt(kDPow4 + static_cast<T>(0.25) * x * x)));
}

template <typename T, std::enable_if_t<std::is_floating_point_v<T>, bool> = true>
SK_ALWAYS_INLINE T approx_parabola_inv_integral(T x) {
    constexpr T kB = static_cast<T>(0.39);
    return x * (static_cast<T>(1) - kB + std::sqrt(kB * kB + static_cast<T>(0.25) * x * x));
}

template <int N>
SK_ALWAYS_INLINE skvx::Vec<N, float> approx_parabola_integral(skvx::Vec<N, float> x) {
    constexpr float kD = 0.67f;
    constexpr float kDPow4 = kD * kD * kD * kD;
    skvx::Vec<N, float> x2 = x * x;
    skvx::Vec<N, float> temp = skvx::sqrt(
            skvx::sqrt(skvx::fma(x2, skvx::Vec<N, float>(0.25f), skvx::Vec<N, float>(kDPow4))));
    skvx::Vec<N, float> denom = temp + (1.0f - kD);

    return x / denom;
}

template <int N>
SK_ALWAYS_INLINE skvx::Vec<N, float> approx_parabola_inv_integral(skvx::Vec<N, float> x) {
    constexpr float kB = 0.39f;
    constexpr float kOneMinusB = 1.0f - kB;
    constexpr float kB2 = kB * kB;

    skvx::Vec<N, float> x2 = x * x;
    skvx::Vec<N, float> temp =
            skvx::sqrt(skvx::fma(x2, skvx::Vec<N, float>(0.25f), skvx::Vec<N, float>(kB2)));
    skvx::Vec<N, float> factor = temp + kOneMinusB;

    return x * factor;
}

/*
 * Determining the number of quads from a cubic:
 *
 * 1. Error Budgeting:
 *    Flattening cubics happens in two steps: Cubic -> Quads, then Quads -> Lines. If the first step
 *    uses the full visual tolerance, the final lines will look jagged (e.g., Flatten's
 *    kCubicAccuracy allocates only 10% (kCubicErrTolerance) of the total allowed pixel error to the
 *    Cubic -> Quads step, saving 90% (kQuadToLineTolFromCubic) for the final lines).
 *
 * 2. Wang's Formula and the Vector Error (errV):
 *    Wang's formula gives the minimum number of evenly spaced (in parametric space) line segments
 *    that a bezier curve of degree "n" must be chopped into to guarantee all lines stay within a
 *    distance of "1/precision" pixels from the true curve:
 *
 *    maxLength = max([length(p[i+2] - 2p[i+1] + p[i]) for (0 <= i <= n-2)])
 *    numParametricSegments = sqrt(maxLength * precision * n*(n - 1)/8)
 *
 *    (Goldman, Ron. (2003). 5.6.3 Wang's Formula. "Pyramid Algorithms: A Dynamic Programming
 *    Approach to Curves and Surfaces for Geometric Modeling". Morgan Kaufmann Publishers.)
 *
 *    Since we are flattening a cubic to a *quadratic*, we shift the math up one degree and evaluate
 *    the third derivative instead: (3*P2 - P3) - (3*P1 - P0). If this evaluates to zero, the second
 *    derivative is constant, meaning the curve is a perfect parabola. We use this expression 'errV'
 *    as a measure of how much the cubic "bulges" away from an ideal quadratic shape.
 *
 * 3. Error Quantization and Lookup Table
 *    From Wang's formula, the maximum geometric error of the un-chopped cubic is
 *    ||errV|| / (12*sqrt(3)). By Taylor's theorem, if we chop the curve into n segments, the
 *    approximation error shrinks cubically (Base Error / n^3). Thus, to satisfy the desired
 *    tolerance, we must find the minimum n where:
 *
 *    ((||errV|| / (12*sqrt(3)) / n^3) <= errTolerance.
 *
 *    To avoid calling sqrt, we square the equation: (12*sqrt(3))^2 = 432 (see
 *    inv_cubic_err_threshold()). Correspondingly, (n^3)^2 = n^6, so errDiv scales at n^6. To avoid
 *    calculating 6th roots, and because we know we can only have an integer number of
 *    subdivisions, we precompute a lookup table (kPowSixLut) of 6th powers to find the required
 *    number of subdivisions.
 *
 * 4. Subdivision limit:
 *    As a guard against pathological inputs, and to avoid dynamic memory allocation, we bound the
 *    maximum number of quads that can be produced by a cubic. If errDiv exceeds the table, the
 *    function returns Flatten::kMaxQuadsFromCubic.
 */

// The 'invErrThreshold' of estimate_num_quads_from_cubic() for an error tolerance in pixels:
// 1 / ((12 * sqrt(3))^2 * errTolerance^2), inverted to avoid division.
template <typename T> constexpr T inv_cubic_err_threshold(T errTolerance) {
    return static_cast<T>(1) / (static_cast<T>(432) * errTolerance * errTolerance);
}

template <typename T>
SK_ALWAYS_INLINE uint32_t estimate_num_quads_from_cubic(const SkPoint pts[4], T invErrThreshold) {
    // A cubic is a perfect quadratic if (3*P1 - P0) == (3*P2 - P3)
    SkPoint leftQuadCtrlX2 = pts[1] * 3.0f - pts[0];
    SkPoint rightQuadCtrlX2 = pts[2] * 3.0f - pts[3];
    SkPoint errV = rightQuadCtrlX2 - leftQuadCtrlX2;

    // Squared error distance
    T errSq = SkPoint::DotProduct(errV, errV);

    // The relative error mapped into n^6
    T errDiv = errSq * invErrThreshold;

    constexpr T kPowSixLut[Flatten::kMaxQuadsFromCubic] = {
            1.0,
            64.0,
            729.0,
            4096.0,
            15625.0,
            46656.0,
            117649.0,
            262144.0,
            531441.0,
            1000000.0,
            1771561.0,
            2985984.0,
            4826809.0,
            7529536.0,
            11390625.0,
            16777216.0,
    };

    for (uint32_t i = 0; i < Flatten::kMaxQuadsFromCubic; ++i) {
        if (errDiv <= kPowSixLut[i]) {
            return i + 1;
        }
    }
    return Flatten::kMaxQuadsFromCubic;
}

/*
 * 1. Error Proxy
 *    The vector `errV = p0 - 2*p1 + p2` is the second difference of the control points. If the
 *    curve were a standard polynomial quadratic (w = 1), this vector's length would directly bound
 *    the deviation between the curve and its chord. We use this as our baseline metric for the
 *    polygon's curvature. Because conics are rational, weights (0 < w < 1) cause the curve to
 *    parameterize non-uniformly. This significantly increases the worst-case error when
 *    approximating it with a cubic polynomial. We multiply by `algFactor` (1 / w^2) to
 *    heuristically overestimate the distortion and force enough subdivisions to hide visual errors.
 *
 * 2. Midpoint Approximation
 *    This implementation uses the G1 mid-point approximation from Proposition 5 of Floater, M. S.
 *    (1996). Floater demonstrates that the error of this approximation is O(h^6), where h is the
 *    parametric interval. Thus, bisecting the curve at t=0.5 reduces the worst-case error bound
 *    by a factor of 64 (2^6).
 *
 * 'invTol' is the inverse of the error tolerance in pixels. Returns the total number of generated
 * cubics (1 << bisections).
 */
SK_ALWAYS_INLINE uint32_t
split_conic_to_cubics(const SkPoint pts[3],
                      float w,
                      float invTol,
                      SkPoint outCubics[Flatten::kMaxCubicsFromConic][4]) {
    skvx::float2 p0 = skvx::float2::Load(&pts[0]);
    skvx::float2 p1 = skvx::float2::Load(&pts[1]);
    skvx::float2 p2 = skvx::float2::Load(&pts[2]);

    float algFactor = (w < 1.0f && w > 0.0f) ? 1.0f / (w * w) : 1.0f;
    skvx::float2 errV = p0 - p1 * 2.0f + p2;
    float err = algFactor * skvx::length(errV);

    int32_t bisections = std::min(skgpu::wangs_formula::nextlog64(err * invTol),
                                  static_cast<int32_t>(Flatten::kMaxBisections));

    struct ConicState {
        skvx::float2 p0, p1, p2;
        float w;
        int32_t level;
    };

    ConicState stack[Flatten::kMaxBisections + 1];
    int32_t stackIdx = 0;

    stack[stackIdx++] = {p0, p1, p2, w, bisections};
    int32_t cubicIdx = 0;

    // Use unrolled recursion with fixed-size array
    constexpr int32_t kMaxTreeNodes = (Flatten::kMaxCubicsFromConic * 2) - 1;
    for (int32_t i = 0; i < kMaxTreeNodes; ++i) {
        if (stackIdx == 0) {
            break;
        }

        ConicState c = stack[--stackIdx];

        if (c.level == 0) {
            // Approximate conic as cubic
            float k = (4.0f / 3.0f) * (c.w / (1.0f + c.w));
            skvx::float2 c1 = c.p0 + (c.p1 - c.p0) * k;
            skvx::float2 c2 = c.p2 + (c.p1 - c.p2) * k;

            c.p0.store(&outCubics[cubicIdx][0]);
            c1.store(&outCubics[cubicIdx][1]);
            c2.store(&outCubics[cubicIdx][2]);
            c.p2.store(&outCubics[cubicIdx][3]);

            cubicIdx++;
        } else {
            // Bisect conic
            float inv = 1.0f / (1.0f + c.w);
            skvx::float2 p01 = (c.p0 + c.p1 * c.w) * inv;
            skvx::float2 p12 = (c.p1 * c.w + c.p2) * inv;
            skvx::float2 M = (p01 + p12) * 0.5f;
            float newWeight = std::sqrt((1.0f + c.w) * 0.5f);

            stack[stackIdx++] = {M, p12, c.p2, newWeight, c.level - 1};
            stack[stackIdx++] = {c.p0, p01, M, newWeight, c.level - 1};
        }
    }

    return 1u << bisections;
}

// Whether the points, and so the curve they control, lie entirely above, right of, or below
// 'clip'. Such a curve contributes no winding inside the clip.
template <int N, typename Rect>
SK_ALWAYS_INLINE bool is_completely_culled(const SkPoint pts[N], const Rect& clip) {
    bool top = true;
    bool right = true;
    bool bottom = true;
    for (int i = 0; i < N; ++i) {
        top &= (pts[i].fY < clip.top());
        right &= (pts[i].fX > clip.right());
        bottom &= (pts[i].fY > clip.bottom());
    }
    return top || right || bottom;
}

// Whether the points lie entirely left of 'clip'. Such a curve still contributes winding to the
// rows it spans, so it is simplified rather than culled.
template <int N, typename Rect>
SK_ALWAYS_INLINE bool is_completely_left(const SkPoint pts[N], const Rect& clip) {
    bool left = true;
    for (int i = 0; i < N; ++i) {
        left &= (pts[i].fX < clip.left());
    }
    return left;
}

}  // namespace skgpu::graphite::flatten_utils

#endif  // skgpu_graphite_sparse_strips_FlattenUtils_DEFINED
