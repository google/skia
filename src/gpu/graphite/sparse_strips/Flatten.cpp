/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/sparse_strips/Flatten.h"

#include "src/core/SkVx.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"

namespace skgpu::graphite {

namespace {

// An intermediate helper struct used to pass values between anonymous namespace functions and
// Flatten class functions. Used for flattenning both quads and cubics.
struct FlattenParams {
    // The starting value of the parabolic integral for each quadratic segment. Maps the start of
    // the curve into uniform arc-length space.
    double fA0;
    // The delta of the parabolic integral (a2 - a0) for each quadratic. Used to linearly
    // interpolate the integral 'a' across the segment.
    double fDa;
    // Inverse integral at fA0. Used to anchor the mapped parametric value back to 0.
    double fU0;
    // Normalization factor (1.0 / (u2 - u0)) to scale the final 't' into the [0, 1] range.
    double fUScale;
    // Total integrated curvature/error metric for the segment. Used to compute the required number
    // of subdivisions.
    double fCurvatureIntegral;
};

SK_ALWAYS_INLINE double approx_parabola_integral(double x) {
    constexpr double kD = 0.67;
    constexpr double kDPow4 = kD * kD * kD * kD;
    return x / ( 1.0 - kD + std::sqrt(std::sqrt(kDPow4 + 0.25 * x * x)));
}

SK_ALWAYS_INLINE double approx_parabola_inv_integral(double x) {
    constexpr double kB = 0.39;
    return x * (1.0 - kB + std::sqrt(kB * kB + 0.25 * x * x));
}

/*
 * Determing the number of quads from a cubic:
 *
 * 1. Error Budgeting (kCubicAccuracy):
 *    Flattening cubics happens in two steps: Cubic -> Quads, then Quads -> Lines. If the first step
 *    uses the full visual tolerance, the final lines will look jagged. kCubicAccuracy allocates
 *    only 10% (kCubicErrTolerance) of the total allowed pixel error to the Cubic -> Quads steps,
 *    saving 90% (kQuadToLineTolFromCubic) for the final lines.
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
 *    approximation error shrinks cubically (Base Error / n^3). Thus, to satisfy our desired
 *    tolerance, we must find the minimum n where:
 *
 *    ((||errV|| / (12*sqrt(3)) / n^3) <= kCubicErrTolerance.
 *
 *    To avoid calling sqrt, we square the equation: (12*sqrt(3))^2 = 432. Correspondingly,
 *    (n^3)^2 = n^6, so errDiv scales at n^6. To avoid calculating 6th roots, and because we know we
 *    can only have an integer number of subdivisions, we precompute a lookup table (kPowSixLut) of
 *    6th powers to find the required number of subdivisions.
 *
 * 4. Subdivision limit:
 *    As a guard against pathological inputs, and to avoid dynamic memory allocation, we bound the
 *    maximum number of quads that can be produced by a cubic. If errDiv exceeds the table, the
 *    function returns kMaxQuadsFromCubic.
 */
SK_ALWAYS_INLINE uint32_t estimate_num_quads_from_cubic(const SkPoint pts[4]) {
    // (12 * sqrt(3))^2 * kCubicErrTolerance^2, inverted to avoid division
    constexpr double kWangErrThreshold =
            1.0 / (432.0 * Flatten::kCubicErrTolerance * Flatten::kCubicErrTolerance);

    // A cubic is a perfect quadratic if (3*P1 - P0) == (3*P2 - P3)
    SkPoint leftQuadCtrlX2 = pts[1] * 3.0f - pts[0];
    SkPoint rightQuadCtrlX2 = pts[2] * 3.0f - pts[3];
    SkPoint errV = rightQuadCtrlX2 - leftQuadCtrlX2;

    // Squared error distance
    double errSq = SkPoint::DotProduct(errV, errV);

    // The relative error mapped into n^6
    double errDiv = errSq * kWangErrThreshold;

    constexpr double kPowSixLut[Flatten::kMaxQuadsFromCubic] = {
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

SK_ALWAYS_INLINE FlattenParams estimate_lines_from_quad(const SkPoint pts[3],
                                                        double kSqrtErrTolerance) {
    SkPoint d01 = pts[1] - pts[0];
    SkPoint d12 = pts[2] - pts[1];
    SkPoint dd = d01 - d12;

    double cross = SkPoint::CrossProduct(pts[2] - pts[0], dd);
    // If the quad's points are sufficiently collinear, we simplify it to a single line spanning
    // the endpoints to prevent division by zero.
    //
    // For perfectly collinear quads, this produces a correct image; in a filled path, the
    // collapsed control point adds no area. While this shortcut is technically incorrect for
    // "needle-thin" nearly collinear quads, it is extremely unlikely such a path would enclose
    // an MSAA subsample point, rendering extra handling unnecessary.
    if (std::abs(cross) < Flatten::kEpsilonD) {
        return {0.0, 0.0, 0.0, 1.0, 0.0};
    }

    double x0 = SkPoint::DotProduct(d01, dd) / cross;
    double x2 = SkPoint::DotProduct(d12, dd) / cross;
    double scale = std::abs(cross / (dd.length() * (x2 - x0)));

    double a0 = approx_parabola_integral(x0);
    double a2 = approx_parabola_integral(x2);
    double da = a2 - a0;
    double val = 0.0;

    if (std::isfinite(scale)) {
        double absDa = std::abs(da);
        double sqrtScale = std::sqrt(scale);

        if (std::signbit(x0) == std::signbit(x2)) {
            val = absDa * sqrtScale;
        } else {
            double xMin = kSqrtErrTolerance / sqrtScale;
            val = kSqrtErrTolerance * absDa / approx_parabola_integral(xMin);
        }
    }

    double u0 = approx_parabola_inv_integral(a0);
    double u2 = approx_parabola_inv_integral(a2);
    double uScale = 1.0 / (u2 - u0);

    return {a0, da, u0, uScale, val};
}

SK_ALWAYS_INLINE double determine_quad_subdiv_t(const FlattenParams& params, double x) {
    double a = params.fA0 + params.fDa * x;
    double u = approx_parabola_inv_integral(a);
    return (u - params.fU0) * params.fUScale;
}

SK_ALWAYS_INLINE SkPoint eval_quad_scalar(const SkPoint pts[3], float t) {
    float mt = 1.0f - t;
    return pts[0] * (mt * mt) + pts[1] * (2.0f * mt * t) + pts[2] * (t * t);
}

SK_ALWAYS_INLINE SkPoint eval_cubic_scalar(const SkPoint pts[4], float t) {
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float t2 = t * t;
    return pts[0] * (mt2 * mt) + pts[1] * (3.0f * mt2 * t) + pts[2] * (3.0f * mt * t2) +
           pts[3] * (t2 * t);
}

template <int N>
SK_ALWAYS_INLINE bool is_completely_culled(const SkPoint pts[N], float width, float height) {
    bool top = true;
    bool right = true;
    bool bottom = true;
    for (int i = 0; i < N; ++i) {
        top    &= (pts[i].fY < 0.0f);
        right  &= (pts[i].fX > width);
        bottom &= (pts[i].fY > height);
    }
    return top || right || bottom;
}

template <int N>
SK_ALWAYS_INLINE bool is_completely_left(const SkPoint pts[N]) {
    bool left = true;
    for (int i = 0; i < N; ++i) {
        left &= (pts[i].fX < 0.0f);
    }
    return left;
}

SK_ALWAYS_INLINE bool is_within_dist_sq(SkPoint p, SkPoint a, SkPoint b, float kErrTolerance) {
    SkPoint ab = b - a;
    SkPoint ap = p - a;
    float lenSq = SkPoint::DotProduct(ab, ab);
    float tNum = SkPoint::DotProduct(ap, ab);

    if (lenSq == 0.0f || tNum <= 0.0f) {
        return SkPoint::DotProduct(ap, ap) <= kErrTolerance;
    }
    if (tNum >= lenSq) {
        SkPoint bp = p - b;
        return SkPoint::DotProduct(bp, bp) <= kErrTolerance;
    }

    // Lagrange's identity: apSq*lenSq - tNum*tNum == cross^2
    float cross = SkPoint::CrossProduct(ap, ab);
    return (cross * cross) <= (kErrTolerance * lenSq);
}

// SIMD constants for convenience
static const skvx::float4 kIotaLo(0.0f, 0.0f, 2.0f, 2.0f);
static const skvx::float4 kIotaHi(1.0f, 1.0f, 3.0f, 3.0f);
static const skvx::float8 kIota = skvx::join(kIotaLo, kIotaHi);
static const skvx::float8 kIota2 =
        skvx::join(skvx::float4(0.0f, 0.0f, 1.0f, 1.0f), skvx::float4(2.0f, 2.0f, 3.0f, 3.0f));

SK_ALWAYS_INLINE skvx::float8 splat_pt_simd(SkPoint pt) {
    skvx::float4 xyxy(pt.fX, pt.fY, pt.fX, pt.fY);
    return skvx::join(xyxy, xyxy);
}

SK_ALWAYS_INLINE skvx::float4 approx_parabola_integral_simd(skvx::float4 x) {
    constexpr float kD = 0.67f;
    constexpr float kDPow4 = kD * kD * kD * kD;
    skvx::float4 x2 = x * x;
    skvx::float4 temp =
            skvx::sqrt(skvx::sqrt(skvx::fma(x2, skvx::float4(0.25f), skvx::float4(kDPow4))));
    skvx::float4 denom = temp + (1.0f - kD);

    return x / denom;
}

SK_ALWAYS_INLINE skvx::float8 approx_parabola_inv_integral_simd(skvx::float8 x) {
    constexpr float kB = 0.39f;
    constexpr float kOneMinusB = 1.0f - kB;
    constexpr float kB2 = kB * kB;

    skvx::float8 x2 = x * x;
    skvx::float8 temp = skvx::sqrt(skvx::fma(x2, skvx::float8(0.25f), skvx::float8(kB2)));
    skvx::float8 factor = temp + kOneMinusB;

    return x * factor;
}

SK_ALWAYS_INLINE skvx::int4 is_finite_simd(skvx::float4 x) {
    // x is guaranteed positive area/length, no abs() needed to clear sign bit
    return sk_bit_cast<skvx::int4>(x) < skvx::int4(0x7f800000);
}

template <bool kIsIdentity,
          typename ProcessQuadFn,
          typename ProcessConicFn,
          typename ProcessCubicFn>
SK_ALWAYS_INLINE void processPathsImpl(const SkPath& path,
                                       const SkMatrix& ctm,
                                       Polyline* polyline,
                                       ProcessQuadFn&& processQuad,
                                       ProcessConicFn&& processConic,
                                       ProcessCubicFn&& processCubic) {
    bool closed = true;
    SkPoint startPt = {0, 0};
    SkPoint lastPt = {0, 0};

    SkPath::Iter iter(path, false);
    SkPoint localPts[4];
    SkPoint mappedPts[4];
    SkPath::Verb verb;

    while ((verb = iter.next(localPts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb: {
                const SkPoint* pts = localPts;
                if constexpr (!kIsIdentity) {
                    ctm.mapPoints(SkSpan(mappedPts, 1), SkSpan(localPts, 1));
                    pts = mappedPts;
                }

                if (!closed && lastPt != startPt) {
                    polyline->appendPoint(startPt);
                }
                polyline->appendSentinel();

                closed = false;
                lastPt = pts[0];
                startPt = pts[0];
                polyline->appendPoint(pts[0]);
                break;
            }
            case SkPath::kLine_Verb: {
                SkASSERT(!closed);
                const SkPoint* pts = localPts;
                if constexpr (!kIsIdentity) {
                    ctm.mapPoints(SkSpan(mappedPts, 2), SkSpan(localPts, 2));
                    pts = mappedPts;
                }

                lastPt = pts[1];
                polyline->appendPoint(pts[1]);
                break;
            }
            case SkPath::kQuad_Verb: {
                SkASSERT(!closed);
                const SkPoint* pts = localPts;
                if constexpr (!kIsIdentity) {
                    ctm.mapPoints(SkSpan(mappedPts, 3), SkSpan(localPts, 3));
                    pts = mappedPts;
                }

                processQuad(pts);
                lastPt = pts[2];
                break;
            }
            case SkPath::kConic_Verb: {
                SkASSERT(!closed);
                const SkPoint* pts = localPts;
                if constexpr (!kIsIdentity) {
                    ctm.mapPoints(SkSpan(mappedPts, 3), SkSpan(localPts, 3));
                    pts = mappedPts;
                }

                processConic(pts, iter.conicWeight());
                lastPt = pts[2];
                break;
            }
            case SkPath::kCubic_Verb: {
                SkASSERT(!closed);
                const SkPoint* pts = localPts;
                if constexpr (!kIsIdentity) {
                    ctm.mapPoints(SkSpan(mappedPts, 4), SkSpan(localPts, 4));
                    pts = mappedPts;
                }

                processCubic(pts);
                lastPt = pts[3];
                break;
            }
            case SkPath::kClose_Verb: {
                closed = true;
                if (lastPt != startPt) {
                    polyline->appendPoint(startPt);
                }
                break;
            }
            default:
                break;
        }
    }

    if (!closed && lastPt != startPt) {
        polyline->appendPoint(startPt);
    }

    polyline->appendSentinel();
}

}  // anonymous namespace

SK_ALWAYS_INLINE void Flatten::flattenQuadScalar(const SkPoint pts[3], Polyline* polyline) {
    FlattenParams params = estimate_lines_from_quad(pts, kSqrtQuadTolerance);
    // Calculate the minimum number of linear subdivisions needed to stay within the visual
    // tolerance. The maximum geometric error of a straight line approximating a parabola is
    // (a * W^2) / 4. Taking the square root makes this easier: sqrt(Error) = (sqrt(a) * W) / 2
    // since fCurvatureIntegral = sqrt(a) * W_total. So the required number of segments 'n' to keep
    // the error per segment under `kSqrtQuadTolerance` is:
    // n = fCurvatureIntegral / (kSqrtQuadTolerance * 2).
    uint32_t numSegments = std::max(1u, static_cast<uint32_t>(
            std::ceil(0.5 / kSqrtQuadTolerance * params.fCurvatureIntegral)));
    double step = 1.0 / numSegments;

    for (uint32_t i = 1; i < numSegments; ++i) {
        double u = i * step;
        double t = determine_quad_subdiv_t(params, u);
        polyline->appendPoint(eval_quad_scalar(pts, static_cast<float>(t)));
    }
    polyline->appendPoint(pts[2]);
}

SK_ALWAYS_INLINE uint32_t Flatten::flattenCubicScalar(const SkPoint pts[4]) {
    uint32_t numQuads = estimate_num_quads_from_cubic(pts);
    fContext.fNumQuads = numQuads;
    static constexpr float kHalfOverN[kMaxQuadsFromCubic + 1] = {
        0.0f,
        0.5f/1.0f, 0.5f/2.0f, 0.5f/3.0f, 0.5f/4.0f,
        0.5f/5.0f, 0.5f/6.0f, 0.5f/7.0f, 0.5f/8.0f,
        0.5f/9.0f, 0.5f/10.f, 0.5f/11.f, 0.5f/12.f,
        0.5f/13.f, 0.5f/14.f, 0.5f/15.f, 0.5f/16.f
    };
    float dt = kHalfOverN[numQuads];

    float curvatureIntegralSum = 0.0f;

    for (uint32_t i = 0; i < numQuads; ++i) {
        float t0 = (2 * i) * dt;
        float t1 = (2 * i + 1) * dt;
        float t2 = (2 * i + 2) * dt;

        SkPoint p0 = eval_cubic_scalar(pts, t0);
        SkPoint pOneHalf = eval_cubic_scalar(pts, t1);
        SkPoint p2 = eval_cubic_scalar(pts, t2);
        SkPoint p1 = pOneHalf * 2.0f - (p0 + p2) * 0.5f;

        fContext.fEvenPts[i] = p0;
        fContext.fOddPts[i] = p1;
        fContext.fEvenPts[i + 1] = p2;

        SkPoint quad[3] = {p0, p1, p2};
        FlattenParams params = estimate_lines_from_quad(quad, kSqrtQuadFromCubicTol);

        fContext.fA0[i] = params.fA0;
        fContext.fDa[i] = params.fDa;
        fContext.fU0[i] = params.fU0;
        fContext.fUScale[i] = params.fUScale;

        fContext.fCurvatureIntegral[i] =
                std::max(static_cast<float>(params.fCurvatureIntegral), kEpsilonF);

        curvatureIntegralSum += params.fCurvatureIntegral;
    }

    uint32_t numSegments = std::max<uint32_t>(
        1u, static_cast<uint32_t>(std::ceil(0.5 * curvatureIntegralSum / kSqrtQuadFromCubicTol)));
    uint32_t targetLen = numSegments + 4;
    fContext.fFlattenedCubics.resize(targetLen);

    float step = curvatureIntegralSum / numSegments;
    float stepRecip = 1.0f / step;
    float cumulativeCurvature = 0.0f;
    uint32_t lastN = 0;
    float x0Base = 0.0f;

    // Flatten each quad produced by the cubic.
    for (uint32_t i = 0; i < numQuads; ++i) {
        cumulativeCurvature += fContext.fCurvatureIntegral[i];
        float thisN = cumulativeCurvature * stepRecip;
        float thisNNext = 1.0f + std::floor(thisN);
        uint32_t dn = static_cast<uint32_t>(thisNNext) - lastN; // thisNNext always > lastN

        if (dn > 0) {
            float dx = step / fContext.fCurvatureIntegral[i];
            float x0 = x0Base * dx;

            SkPoint p0 = fContext.fEvenPts[i];
            SkPoint p1 = fContext.fOddPts[i];
            SkPoint p2 = fContext.fEvenPts[i + 1];

            float a0 = fContext.fA0[i];
            float da = fContext.fDa[i];
            float u0 = fContext.fU0[i];
            float uScale = fContext.fUScale[i];

            for (uint32_t j = 0; j < dn; ++j) {
                float x = x0 + j * dx;
                float a = a0 + da * x;
                float u = approx_parabola_inv_integral(a);
                float t = (u - u0) * uScale;

                float mt = 1.0f - t;
                SkPoint p = p0 * (mt * mt) + p1 * (2.0f * t * mt) + p2 * (t * t);
                fContext.fFlattenedCubics[lastN + j] = p;
            }
        }
        x0Base = thisNNext - thisN;
        lastN = static_cast<uint32_t>(thisNNext);
    }

    fContext.fFlattenedCubics[numSegments] = fContext.fEvenPts[numQuads];
    return numSegments + 1;
}

SK_ALWAYS_INLINE void Flatten::flattenQuadSimd(const SkPoint pts[3], Polyline* polyline) {
    FlattenParams params = estimate_lines_from_quad(pts, kSqrtQuadTolerance);
    uint32_t numSegments = std::max(1u, static_cast<uint32_t>(
            std::ceil(0.5 / kSqrtQuadTolerance * params.fCurvatureIntegral)));

    float step = 1.0f / numSegments;

    skvx::float8 p0 = splat_pt_simd(pts[0]);
    skvx::float8 p1 = splat_pt_simd(pts[1]);
    skvx::float8 p2 = splat_pt_simd(pts[2]);

    skvx::float8 A = skvx::fma(p1, skvx::float8(-2.0f), p0) + p2;
    skvx::float8 B = (p1 - p0) * 2.0f;
    skvx::float8 C = p0;

    skvx::float8 vDa = skvx::float8(static_cast<float>(params.fDa));
    skvx::float8 vA0 = skvx::float8(static_cast<float>(params.fA0));
    skvx::float8 vU0 = skvx::float8(static_cast<float>(params.fU0));
    skvx::float8 vUScale = skvx::float8(static_cast<float>(params.fUScale));

    float out[8];
    for (uint32_t i = 1; i < numSegments; i += 4) {
        skvx::float8 x = (kIota2 + skvx::float8(static_cast<float>(i))) * step;

        skvx::float8 a = skvx::fma(vDa, x, vA0);
        skvx::float8 u = approx_parabola_inv_integral_simd(a);
        skvx::float8 t = (u - vU0) * vUScale;

        skvx::float8 p = skvx::fma(skvx::fma(A, t, B), t, C);
        p.store(out);

        // The first point is always valid if we entered the loop
        polyline->appendPoint({out[0], out[1]});
        // Conditionally append the tail results
        if (i + 1 < numSegments) polyline->appendPoint({out[2], out[3]});
        if (i + 2 < numSegments) polyline->appendPoint({out[4], out[5]});
        if (i + 3 < numSegments) polyline->appendPoint({out[6], out[7]});
    }

    // Always append the exact endpoint
    polyline->appendPoint(pts[2]);
}

SK_ALWAYS_INLINE void Flatten::evalCubicsSimd(const SkPoint pts[4], uint32_t numQuads) {
    fContext.fNumQuads = numQuads;
    float dt = 0.5f / numQuads;

    skvx::float8 p0 = splat_pt_simd(pts[0]);
    skvx::float8 p1 = splat_pt_simd(pts[1]);
    skvx::float8 p2 = splat_pt_simd(pts[2]);
    skvx::float8 p3 = splat_pt_simd(pts[3]);

    skvx::float8 A = skvx::fma(p1 - p2, skvx::float8(3.0f), p3 - p0);
    skvx::float8 B = skvx::fma(p1, skvx::float8(-2.0f), p0 + p2) * 3.0f;
    skvx::float8 C = (p1 - p0) * 3.0f;
    skvx::float8 D = p0;

    skvx::float8 step = kIota * dt;
    skvx::float8 t = step;
    skvx::float8 tInc(4.0f * dt);

    float* evenPts = reinterpret_cast<float*>(fContext.fEvenPts.data());
    float* oddPts = reinterpret_cast<float*>(fContext.fOddPts.data());

    uint32_t loopCount = (numQuads + 1) / 2;
    for (uint32_t i = 0; i < loopCount; ++i) {
        skvx::float8 evaluated = skvx::fma(skvx::fma(skvx::fma(A, t, B), t, C), t, D);
        evaluated.lo.store(evenPts + i * 4);
        evaluated.hi.store(oddPts + i * 4);
        t = t + tInc;
    }

    p3.store(evenPts + numQuads * 2);
}

SK_ALWAYS_INLINE void Flatten::estimateLinesFromQuadSimd() {
    uint32_t numQuads = fContext.fNumQuads;
    const float* evenPts = reinterpret_cast<const float*>(fContext.fEvenPts.data());
    float* oddPts = reinterpret_cast<float*>(fContext.fOddPts.data());

    uint32_t chunks = (numQuads + 3) / 4;
    for (uint32_t i = 0; i < chunks; ++i) {
        skvx::float8 p0 = skvx::float8::Load(evenPts + i * 8);
        skvx::float8 pOneHalf = skvx::float8::Load(oddPts + i * 8);
        skvx::float8 p2 = skvx::float8::Load(evenPts + i * 8 + 2);

        skvx::float8 p1 = skvx::fma(pOneHalf, skvx::float8(2.0f), (p0 + p2) * -0.5f);
        p1.store(oddPts + i * 8);

        skvx::float8 d01 = p1 - p0;
        skvx::float8 d12 = p2 - p1;

        skvx::float4 d01x = skvx::shuffle<0, 2, 4, 6>(d01);
        skvx::float4 d01y = skvx::shuffle<1, 3, 5, 7>(d01);
        skvx::float4 d12x = skvx::shuffle<0, 2, 4, 6>(d12);
        skvx::float4 d12y = skvx::shuffle<1, 3, 5, 7>(d12);

        skvx::float4 ddx = d01x - d12x;
        skvx::float4 ddy = d01y - d12y;

        skvx::float4 cross = (d01x + d12x) * ddy - (d01y + d12y) * ddx;
        skvx::float4 absCross = skvx::abs(cross);
        skvx::int4 collinearMask = skvx::cast<int32_t>(absCross < kEpsilonF);
        skvx::float4 invCross = 1.0f / cross;

        skvx::float4 x0 = skvx::fma(d01x, ddx, d01y * ddy) * invCross;
        skvx::float4 x2 = skvx::fma(d12x, ddx, d12y * ddy) * invCross;
        skvx::float4 ddSq = skvx::fma(ddx, ddx, ddy * ddy);
        skvx::float4 ddHypot = skvx::sqrt(ddSq);
        skvx::float4 scale = (cross * cross) / (ddHypot * ddSq);

        skvx::float4 a0 = approx_parabola_integral_simd(x0);
        skvx::float4 a2 = approx_parabola_integral_simd(x2);
        skvx::float4 da = a2 - a0;
        skvx::float4 absDa = skvx::abs(da);
        skvx::float4 sqrtScale = skvx::sqrt(scale);

        skvx::int4 signX0 = sk_bit_cast<skvx::int4>(x0) & 0x80000000;
        skvx::int4 signX2 = sk_bit_cast<skvx::int4>(x2) & 0x80000000;
        skvx::int4 mask = (signX0 == signX2);

        skvx::float4 nonCusp = absDa * sqrtScale;
        skvx::float4 xMin = static_cast<float>(kSqrtQuadFromCubicTol) / sqrtScale;
        skvx::float4 approxInt = approx_parabola_integral_simd(xMin);
        skvx::float4 cusp = (static_cast<float>(kSqrtQuadFromCubicTol) * absDa) / approxInt;

        skvx::float4 valRaw = skvx::if_then_else(mask, nonCusp, cusp);
        valRaw = skvx::if_then_else(collinearMask, skvx::float4(0.0f), valRaw);
        skvx::int4 finiteMask = is_finite_simd(valRaw);
        skvx::float4 val = skvx::if_then_else(finiteMask, valRaw, skvx::float4(0.0f));

        skvx::float8 u0U2 = approx_parabola_inv_integral_simd(skvx::join(a0, a2));
        skvx::float4 u0 = u0U2.lo;
        skvx::float4 u2 = u0U2.hi;
        skvx::float4 uScale = 1.0f / (u2 - u0);
        u0 = skvx::if_then_else(collinearMask, skvx::float4(0.0f), u0);
        uScale = skvx::if_then_else(collinearMask, skvx::float4(1.0f), uScale);

        a0.store(fContext.fA0.data() + i * 4);
        da.store(fContext.fDa.data() + i * 4);
        u0.store(fContext.fU0.data() + i * 4);
        uScale.store(fContext.fUScale.data() + i * 4);
        val.store(fContext.fCurvatureIntegral.data() + i * 4);
    }
}

SK_ALWAYS_INLINE void Flatten::outputLinesFromQuadSimd(
        uint32_t quadIdx, float x0, float dx, uint32_t numSegments, uint32_t startIdx) {
    skvx::float8 p0 = splat_pt_simd(fContext.fEvenPts[quadIdx]);
    skvx::float8 p1 = splat_pt_simd(fContext.fOddPts[quadIdx]);
    skvx::float8 p2 = splat_pt_simd(fContext.fEvenPts[quadIdx + 1]);

    skvx::float8 x = skvx::fma(kIota2, skvx::float8(dx), skvx::float8(x0));
    skvx::float8 da(fContext.fDa[quadIdx]);
    skvx::float8 a = skvx::fma(x, da, skvx::float8(fContext.fA0[quadIdx]));
    skvx::float8 aInc(4.0f * dx * fContext.fDa[quadIdx]);
    skvx::float8 uScale(fContext.fUScale[quadIdx]);
    skvx::float8 u0(fContext.fU0[quadIdx]);

    skvx::float8 A = skvx::fma(p1, skvx::float8(-2.0f), p0) + p2;
    skvx::float8 B = (p1 - p0) * 2.0f;
    skvx::float8 C = p0;

    uint32_t chunks = (numSegments + 3) / 4;
    for (uint32_t j = 0; j < chunks; ++j) {
        skvx::float8 u = approx_parabola_inv_integral_simd(a);
        skvx::float8 t = (u - u0) * uScale;
        skvx::float8 p = skvx::fma(skvx::fma(A, t, B), t, C);

        p.store(fContext.fFlattenedCubics.data() + startIdx + j * 4);
        a = a + aInc;
    }
}

uint32_t Flatten::flattenCubicSimd(const SkPoint pts[4]) {
    uint32_t numQuads = estimate_num_quads_from_cubic(pts);
    this->evalCubicsSimd(pts, numQuads);
    this->estimateLinesFromQuadSimd();

    float curvatureIntegralSum = 0.0f;
    for (uint32_t i = 0; i < numQuads; ++i) {
        float val = std::max(fContext.fCurvatureIntegral[i], static_cast<float>(kEpsilonF));
        fContext.fCurvatureIntegral[i] = val;
        curvatureIntegralSum += val;
    }

    uint32_t numSegments = std::max<uint32_t>(
            1,
            static_cast<uint32_t>(std::ceil(0.5f * curvatureIntegralSum / kSqrtQuadFromCubicTol)));
    uint32_t targetLen = numSegments + 4;
    fContext.fFlattenedCubics.resize(targetLen);

    float step = curvatureIntegralSum / numSegments;
    float stepRecip = 1.0f / step;
    float cumulativeCurvature = 0.0f;
    uint32_t lastN = 0;
    float x0Base = 0.0f;

    for (uint32_t i = 0; i < numQuads; ++i) {
        float val = fContext.fCurvatureIntegral[i];
        cumulativeCurvature += val;
        float thisN = cumulativeCurvature * stepRecip;
        float thisNNext = 1.0f + std::floor(thisN);
        uint32_t dn = static_cast<uint32_t>(thisNNext) - lastN;

        if (dn > 0) {
            float dx = step / val;
            float x0 = x0Base * dx;
            this->outputLinesFromQuadSimd(i, x0, dx, dn, lastN);
        }
        x0Base = thisNNext - thisN;
        lastN = static_cast<uint32_t>(thisNNext);
    }

    fContext.fFlattenedCubics[numSegments] = fContext.fEvenPts[numQuads];
    return numSegments + 1;
}

void Flatten::processPathsSimd(
        const SkPath& path, const SkMatrix& ctm, float width, float height, Polyline* polyline) {
    fContext.fFlattenedCubics.clear();

    auto processQuad = [this, width, height, polyline](const SkPoint pts[3]) {
        skvx::float4 X(pts[0].fX, pts[1].fX, pts[2].fX, pts[2].fX); // Duplicate last pt
        skvx::float4 Y(pts[0].fY, pts[1].fY, pts[2].fY, pts[2].fY);
        if (skvx::all(X > width) || skvx::all(Y < 0.0f) || skvx::all(Y > height)) {
            return;
        }
        if (skvx::all(X < 0.0f) ||
            is_within_dist_sq(pts[1], pts[0], pts[2], kQuadSubdivThreshold)) {
            polyline->appendPoint(pts[2]);
        } else {
            // Note: testing shows that the scalar function is *slightly* faster here, probably
            // because most quads don't produce enough segments to make simd worth it.
            this->flattenQuadScalar(pts, polyline);
        }
    };

    // TODO (thomsmit): this could probably simd-fied a little more.
    auto processConic = [this, width, height, polyline](const SkPoint pts[3], float weight) {
        skvx::float4 X(pts[0].fX, pts[1].fX, pts[2].fX, pts[2].fX); // Duplicate last pt
        skvx::float4 Y(pts[0].fY, pts[1].fY, pts[2].fY, pts[2].fY);
        if (skvx::all(X > width) || skvx::all(Y < 0.0f) || skvx::all(Y > height)) {
            return;
        }
        if (skvx::all(X < 0.0f) ||
            is_within_dist_sq(pts[1], pts[0], pts[2], kQuadSubdivThreshold)) {
            polyline->appendPoint(pts[2]);
        } else {
            const SkPoint* quadPts = fConicToQuad.computeQuads(pts, weight, kQuadErrTolerance);
            int quadCount = fConicToQuad.countQuads();
            for (int i = 0; i < quadCount; ++i) {
                this->flattenQuadSimd(&quadPts[i * 2], polyline);
            }
        }
    };

    auto processCubic = [this, width, height, polyline](const SkPoint pts[4]) {
        skvx::float4 X(pts[0].fX, pts[1].fX, pts[2].fX, pts[3].fX);
        skvx::float4 Y(pts[0].fY, pts[1].fY, pts[2].fY, pts[3].fY);
        if (skvx::all(X > width) || skvx::all(Y < 0.0f) || skvx::all(Y > height)) {
            return;
        }
        if (skvx::all(X < 0.0f) ||
            (is_within_dist_sq(pts[1], pts[0], pts[3], kCubicSubdivThreshold) &&
             is_within_dist_sq(pts[2], pts[0], pts[3], kCubicSubdivThreshold))) {
            polyline->appendPoint(pts[3]);
        } else {
            uint32_t numSegments = this->flattenCubicSimd(pts);
            polyline->appendPoints(SkSpan(fContext.fFlattenedCubics.data() + 1, numSegments - 1));
        }
    };

    if (ctm.isIdentity()) {
        processPathsImpl<true>(path, ctm, polyline, processQuad, processConic, processCubic);
    } else {
        processPathsImpl<false>(path, ctm, polyline, processQuad, processConic, processCubic);
    }
}

void Flatten::processPathsScalar(
        const SkPath& path, const SkMatrix& ctm, float width, float height, Polyline* polyline) {
    fContext.fFlattenedCubics.clear();

    auto processQuad = [this, width, height, polyline](const SkPoint pts[3]) {
        if (is_completely_culled<3>(pts, width, height)) {
            // If the quad is completely top, right, or bottom of the viewport, cull.
            return;
        }
        if (is_completely_left<3>(pts) ||
            is_within_dist_sq(pts[1], pts[0], pts[2], kQuadSubdivThreshold)) {
            // If the quad is visually a line or completely left of the viewport, simplify.
            polyline->appendPoint(pts[2]);
        } else {
            this->flattenQuadScalar(pts, polyline);
        }
    };

    auto processConic = [this, width, height, polyline](const SkPoint pts[3], float weight) {
        if (is_completely_culled<3>(pts, width, height)) {
            // If the conic is completely top, right, or bottom of the viewport, cull.
            return;
        }
        if (is_completely_left<3>(pts) ||
            is_within_dist_sq(pts[1], pts[0], pts[2], kQuadSubdivThreshold)) {
            // If the conic is visually a line or completely left of the viewport, simplify.
            // Note: A low weight can produce a visually flat conic even if the control point is far
            // away, causing a false negative. This is acceptable as we fall back to subdivision.
            polyline->appendPoint(pts[2]);
        } else {
            const SkPoint* quadPts = fConicToQuad.computeQuads(pts, weight, kQuadErrTolerance);
            int quadCount = fConicToQuad.countQuads();
            for (int i = 0; i < quadCount; ++i) {
                this->flattenQuadScalar(&quadPts[i * 2], polyline);
            }
        }
    };

    auto processCubic = [this, width, height, polyline](const SkPoint pts[4]) {
        if (is_completely_culled<4>(pts, width, height)) {
            // If the cubic is completely top, right, or bottom of the viewport, cull.
            return;
        }
        if (is_completely_left<4>(pts) ||
            (is_within_dist_sq(pts[1], pts[0], pts[3], kCubicSubdivThreshold) &&
             is_within_dist_sq(pts[2], pts[0], pts[3], kCubicSubdivThreshold))) {
            // If the cubic is visually a line or completely left of the viewport, simplify.
            polyline->appendPoint(pts[3]);
        } else {
            uint32_t numSegments = this->flattenCubicScalar(pts);
            polyline->appendPoints(SkSpan(fContext.fFlattenedCubics.data() + 1, numSegments - 1));
        }
    };

    if (ctm.isIdentity()) {
        processPathsImpl</*kIsIdentity=*/true>(
                path, ctm, polyline, processQuad, processConic, processCubic);
    } else {
        processPathsImpl</*kIsIdentity=*/false>(
                path, ctm, polyline, processQuad, processConic, processCubic);
    }
}

}  // namespace skgpu::graphite
