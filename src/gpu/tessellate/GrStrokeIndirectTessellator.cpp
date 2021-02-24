/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokeIndirectTessellator.h"

#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrVertexWriter.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/tessellate/GrStrokeIterator.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

namespace {

// Only use SIMD if SkVx will use a built-in compiler extensions for vectors.
#if !defined(SKNX_NO_SIMD) && (defined(__clang__) || defined(__GNUC__))
#define USE_SIMD 1
#else
#define USE_SIMD 0
#endif

#if USE_SIMD
using grvx::vec;
using grvx::ivec;
using grvx::uvec;

// Muxes between "N" (Nx2/2) 2d vectors in SIMD based on the provided conditions. This is equivalent
// to returning the following at each point:
//
//   (conds.lo[i] & conds.hi[i])) ? {t[i].lo, t[i].hi} : {e[i].lo, e[i].hi}.
//
template<int Nx2>
static SK_ALWAYS_INLINE vec<Nx2> if_both_then_else(ivec<Nx2> conds, vec<Nx2> t, vec<Nx2> e) {
    auto both = conds.lo & conds.hi;
    return skvx::if_then_else(skvx::join(both, both), t, e);
}

// Returns the lengths squared of "N" (Nx2/2) 2d vectors in SIMD. The x values are in "v.lo" and
// the y values are in "v.hi".
template<int Nx2> static SK_ALWAYS_INLINE vec<Nx2/2> length_pow2(vec<Nx2> v) {
    auto vv = v*v;
    return vv.lo + vv.hi;
}

// Interpolates between "a" and "b" by a factor of T. T must be <1 and >= 0.
//
// NOTE: This does not return b when T==1. It's implemented as-is because it otherwise seems to get
// better precision than "a*(1 - T) + b*T" for things like chopping cubics on exact cusp points.
// The responsibility falls on the caller to check that T != 1 before calling.
template<int N> SK_ALWAYS_INLINE vec<N> unchecked_mix(vec<N> a, vec<N> b, vec<N> T) {
    return grvx::fast_madd(b - a, T, a);
}
#endif

// Computes and writes out the resolveLevels for individual strokes. Maintains a counter of the
// number of instances at each resolveLevel. If SIMD is available, then these calculations are done
// in batches.
class ResolveLevelCounter {
public:
    constexpr static int8_t kMaxResolveLevel = GrStrokeIndirectTessellator::kMaxResolveLevel;

    ResolveLevelCounter(const SkMatrix& viewMatrix, int* resolveLevelCounts)
            : fResolveLevelCounts(resolveLevelCounts) {
        if (!viewMatrix.getMinMaxScales(fMatrixMinMaxScales.data())) {
            fMatrixMinMaxScales.fill(1);
        }
    }

    void updateTolerances(float strokeWidth, bool isRoundJoin) {
        this->flush();
        fTolerances = GrStrokeTessellateShader::Tolerances::Make(fMatrixMinMaxScales.data(),
                                                                 strokeWidth);
        fResolveLevelForCircles = SkTPin<float>(
                sk_float_nextlog2(fTolerances.fNumRadialSegmentsPerRadian * SK_ScalarPI),
                1, kMaxResolveLevel);
        fIsRoundJoin = isRoundJoin;
#if USE_SIMD
        fWangsTermQuadratic = GrWangsFormula::length_term<2>(fTolerances.fParametricIntolerance);
        fWangsTermCubic = GrWangsFormula::length_term<3>(fTolerances.fParametricIntolerance);
#endif
    }

    bool isRoundJoin() const { return fIsRoundJoin; }

    // Accounts for 180-degree point strokes, which render as circles with diameters equal to the
    // stroke width. We draw circles at cusp points on curves and for round caps.
    //
    // Returns the resolveLevel to use when drawing these circles.
    int8_t countCircles(int numCircles) {
        fResolveLevelCounts[fResolveLevelForCircles] += numCircles;
        return fResolveLevelForCircles;
    }

#if !USE_SIMD
    bool SK_WARN_UNUSED_RESULT countLine(const SkPoint pts[2], SkPoint lastControlPoint,
                                         int8_t* resolveLevelPtr) {
        if (!fIsRoundJoin) {
            // There is no resolve level to track. It's always zero.
            ++fResolveLevelCounts[0];
            return false;
        }
        float rotation = SkMeasureAngleBetweenVectors(pts[0] - lastControlPoint, pts[1] - pts[0]);
        this->writeResolveLevel(0, rotation, resolveLevelPtr);
        return true;
    }

    void countQuad(const SkPoint pts[3], SkPoint lastControlPoint, int8_t* resolveLevelPtr) {
        float numParametricSegments =
                GrWangsFormula::quadratic(fTolerances.fParametricIntolerance, pts);
        float rotation = SkMeasureQuadRotation(pts);
        if (fIsRoundJoin) {
            SkVector nextTan = ((pts[0] == pts[1]) ? pts[2] : pts[1]) - pts[0];
            rotation += SkMeasureAngleBetweenVectors(pts[0] - lastControlPoint, nextTan);
        }
        this->writeResolveLevel(numParametricSegments, rotation, resolveLevelPtr);
    }

    void countCubic(const SkPoint pts[4], SkPoint lastControlPoint, int8_t* resolveLevelPtr) {
        float numParametricSegments =
                GrWangsFormula::cubic(fTolerances.fParametricIntolerance, pts);
        SkVector tan0 = ((pts[0] == pts[1]) ? pts[2] : pts[1]) - pts[0];
        SkVector tan1 = pts[3] - ((pts[3] == pts[2]) ? pts[1] : pts[2]);
        float rotation = SkMeasureAngleBetweenVectors(tan0, tan1);
        if (fIsRoundJoin && pts[0] != lastControlPoint) {
            SkVector nextTan = (tan0.isZero()) ? tan1 : tan0;
            rotation += SkMeasureAngleBetweenVectors(pts[0] - lastControlPoint, nextTan);
        }
        this->writeResolveLevel(numParametricSegments, rotation, resolveLevelPtr);
    }

    void countChoppedCubic(const SkPoint pts[4], const float chopT, SkPoint lastControlPoint,
                           int8_t* resolveLevelPtr) {
        SkPoint chops[7];
        SkChopCubicAt(pts, chops, chopT);
        this->countCubic(chops, lastControlPoint, resolveLevelPtr);
        this->countCubic(chops + 3, chops[3], resolveLevelPtr + 1);
    }

    void flush() {}

private:
    void writeResolveLevel(float numParametricSegments, float rotation,
                           int8_t* resolveLevelPtr) const {
        float numCombinedSegments =
                fTolerances.fNumRadialSegmentsPerRadian * rotation + numParametricSegments;
        int8_t resolveLevel = sk_float_nextlog2(numCombinedSegments);
        resolveLevel = std::min(resolveLevel, kMaxResolveLevel);
        ++fResolveLevelCounts[(*resolveLevelPtr = resolveLevel)];
    }

#else  // USE_SIMD
    ~ResolveLevelCounter() {
        // Always call flush() when finished.
        SkASSERT(fLineQueue.fCount == 0);
        SkASSERT(fQuadQueue.fCount == 0);
        SkASSERT(fCubicQueue.fCount == 0);
        SkASSERT(fChoppedCubicQueue.fCount == 0);
    }

    bool SK_WARN_UNUSED_RESULT countLine(const SkPoint pts[2], SkPoint lastControlPoint,
                                         int8_t* resolveLevelPtr) {
        if (!fIsRoundJoin) {
            // There is no resolve level to track. It's always zero.
            ++fResolveLevelCounts[0];
            return false;
        }
        if (fLineQueue.push(pts, fIsRoundJoin, lastControlPoint, resolveLevelPtr) == 3) {
            this->flushLines<4>();
        }
        return true;
    }

    void countQuad(const SkPoint pts[3], SkPoint lastControlPoint, int8_t* resolveLevelPtr) {
        if (fQuadQueue.push(pts, fIsRoundJoin, lastControlPoint, resolveLevelPtr) == 3) {
            this->flushQuads<4>();
        }
    }

    void countCubic(const SkPoint pts[4], SkPoint lastControlPoint, int8_t* resolveLevelPtr) {
        if (fCubicQueue.push(pts, fIsRoundJoin, lastControlPoint, resolveLevelPtr) == 3) {
            this->flushCubics<4>();
        }
    }

    void countChoppedCubic(const SkPoint pts[4], const float chopT, SkPoint lastControlPoint,
                           int8_t* resolveLevelPtr) {
        int i = fChoppedCubicQueue.push(pts, fIsRoundJoin, lastControlPoint, resolveLevelPtr);
        fCubicChopTs[i] = fCubicChopTs[4 + i] = chopT;
        if (i == 3) {
            this->flushChoppedCubics<4>();
        }
    }

    void flush() {
        // Flush each queue, crunching either 2 curves in SIMD or 4. We do 2 when the queue is low
        // because it allows us to expand two points into a single float4: [x0,x1,y0,y1].
        if (fLineQueue.fCount) {
            SkASSERT(fIsRoundJoin);
            if (fLineQueue.fCount <= 2) {
                this->flushLines<2>();
            } else {
                this->flushLines<4>();
            }
        }
        if (fQuadQueue.fCount) {
            if (fQuadQueue.fCount <= 2) {
                this->flushQuads<2>();
            } else {
                this->flushQuads<4>();
            }
        }
        if (fCubicQueue.fCount) {
            if (fCubicQueue.fCount <= 2) {
                this->flushCubics<2>();
            } else {
                this->flushCubics<4>();
            }
        }
        if (fChoppedCubicQueue.fCount) {
            if (fChoppedCubicQueue.fCount <= 2) {
                this->flushChoppedCubics<2>();
            } else {
                this->flushChoppedCubics<4>();
            }
        }
    }

private:
    // This struct stores deferred resolveLevel calculations for performing in SIMD batches.
    template<int NumPts> struct SIMDQueue {
        // Enqueues a stroke.
        SK_ALWAYS_INLINE int push(const SkPoint pts[NumPts], bool pushRoundJoin,
                                  SkPoint lastControlPoint, int8_t* resolveLevelPtr) {
            SkASSERT(0 <= fCount && fCount < 4);
            for (int i = 0; i < NumPts; ++i) {
                fPts[i][fCount] = pts[i].fX;
                fPts[i][4 + fCount] = pts[i].fY;
            }
            if (pushRoundJoin) {
                fLastControlPoints[fCount] = lastControlPoint.fX;
                fLastControlPoints[4 + fCount] = lastControlPoint.fY;
            }
            fResolveLevelPtrs[fCount] = resolveLevelPtr;
            return fCount++;
        }

        // Loads pts[idx] in SIMD for fCount strokes, with the x values in the "vec.lo" and the y
        // values in "vec.hi".
        template<int N> vec<N*2> loadPoint(int idx) const {
            SkASSERT(0 <= idx && idx < NumPts);
            return this->loadPointFromArray<N>(fPts[idx]);
        }

        // Loads fCount lastControlPoints in SIMD, with the x values in "vec.lo" and the y values in
        // "vec.hi".
        template<int N> vec<N*2> loadLastControlPoint() const {
            return this->loadPointFromArray<N>(fLastControlPoints);
        }

        // Loads fCount points from the given array in SIMD, with the x values in "vec.lo" and the y
        // values in "vec.hi". The array must be ordered as: [x0,x1,x2,x3,y0,y1,y2,y3].
        template<int N> vec<N*2> loadPointFromArray(const float array[8]) const {
            if constexpr (N == 4) {
                if (fCount == 4) {
                    return vec<8>::Load(array);
                } else {
                    SkASSERT(fCount == 3);
                    return {array[0], array[1], array[2], 0, array[4], array[5], array[6], 0};
                }
            } else {
                if (fCount == 2) {
                    return {array[0], array[1], array[4], array[5]};
                } else {
                    SkASSERT(fCount == 1);
                    return {array[0], 0, array[4], 0};
                }
            }
        }

        struct alignas(sizeof(float) * 8) {
            float fPts[NumPts][8];
            float fLastControlPoints[8];
        };
        int8_t* fResolveLevelPtrs[4];
        int fCount = 0;
    };

    template<int N> void flushLines() {
        SkASSERT(fLineQueue.fCount > 0);

        // Find the angle of rotation in the preceding round join.
        auto a = fLineQueue.loadLastControlPoint<N>();
        auto b = fLineQueue.loadPoint<N>(0);
        auto c = fLineQueue.loadPoint<N>(1);
        auto rotation = grvx::approx_angle_between_vectors(b - a, c - b);

        this->writeResolveLevels<N>(0, rotation, fLineQueue.fCount, fLineQueue.fResolveLevelPtrs);
        fLineQueue.fCount = 0;
    }

    template<int N> void flushQuads() {
        SkASSERT(fQuadQueue.fCount > 0);
        auto p0 = fQuadQueue.loadPoint<N>(0);
        auto p1 = fQuadQueue.loadPoint<N>(1);
        auto p2 = fQuadQueue.loadPoint<N>(2);

        // Execute Wang's formula to determine how many parametric segments the curve needs to be
        // divided into. (See GrWangsFormula::quadratic().)
        auto l = length_pow2(grvx::fast_madd<N*2>(-2, p1, p2) + p0);
        auto numParametricSegments = skvx::sqrt(fWangsTermQuadratic * skvx::sqrt(l));

        // Find the curve's rotation. Since quads cannot inflect or rotate more than 180 degrees,
        // this is equal to the angle between the beginning and ending tangents.
        // NOTE: If p0==p1 or p1==p2, this will give rotation=0.
        auto tan0 = p1 - p0;
        auto tan1 = p2 - p1;
        auto rotation = grvx::approx_angle_between_vectors(tan0, tan1);
        if (fIsRoundJoin) {
            // Add rotation for the preceding round join.
            auto lastControlPoint = fQuadQueue.loadLastControlPoint<N>();
            auto nextTan = if_both_then_else((tan0 == 0), tan1, tan0);
            rotation += grvx::approx_angle_between_vectors(p0 - lastControlPoint, nextTan);
        }

        this->writeResolveLevels<N>(numParametricSegments, rotation, fQuadQueue.fCount,
                                    fQuadQueue.fResolveLevelPtrs);
        fQuadQueue.fCount = 0;
    }

    template<int N> void flushCubics() {
        SkASSERT(fCubicQueue.fCount > 0);
        auto p0 = fCubicQueue.loadPoint<N>(0);
        auto p1 = fCubicQueue.loadPoint<N>(1);
        auto p2 = fCubicQueue.loadPoint<N>(2);
        auto p3 = fCubicQueue.loadPoint<N>(3);
        this->flushCubics<N>(fCubicQueue, p0, p1, p2, p3, fIsRoundJoin, 0);
        fCubicQueue.fCount = 0;
    }

    template<int N> void flushChoppedCubics() {
        SkASSERT(fChoppedCubicQueue.fCount > 0);
        auto p0 = fChoppedCubicQueue.loadPoint<N>(0);
        auto p1 = fChoppedCubicQueue.loadPoint<N>(1);
        auto p2 = fChoppedCubicQueue.loadPoint<N>(2);
        auto p3 = fChoppedCubicQueue.loadPoint<N>(3);
        auto T = fChoppedCubicQueue.loadPointFromArray<N>(fCubicChopTs);

        // Chop the cubic at its chopT and find the resolve level for each half.
        auto ab = unchecked_mix(p0, p1, T);
        auto bc = unchecked_mix(p1, p2, T);
        auto cd = unchecked_mix(p2, p3, T);
        auto abc = unchecked_mix(ab, bc, T);
        auto bcd = unchecked_mix(bc, cd, T);
        auto abcd = unchecked_mix(abc, bcd, T);
        this->flushCubics<N>(fChoppedCubicQueue, p0, ab, abc, abcd, fIsRoundJoin, 0);
        this->flushCubics<N>(fChoppedCubicQueue, abcd, bcd, cd, p3, false/*countRoundJoin*/, 1);

        fChoppedCubicQueue.fCount = 0;
    }

    template<int N> SK_ALWAYS_INLINE void flushCubics(const SIMDQueue<4>& queue, vec<N*2> p0,
                                                      vec<N*2> p1, vec<N*2> p2, vec<N*2> p3,
                                                      bool countRoundJoin, int resultOffset) const {
        // Execute Wang's formula to determine how many parametric segments the curve needs to be
        // divided into. (See GrWangsFormula::cubic().)
        auto l0 = length_pow2(grvx::fast_madd<N*2>(-2, p1, p2) + p0);
        auto l1 = length_pow2(grvx::fast_madd<N*2>(-2, p2, p3) + p1);
        auto numParametricSegments = skvx::sqrt(fWangsTermCubic * skvx::sqrt(skvx::max(l0, l1)));

        // Find the starting tangent (or zero if p0==p1==p2).
        auto tan0 = p1 - p0;
        tan0 = if_both_then_else((tan0 == 0), p2 - p0, tan0);

        // Find the ending tangent (or zero if p1==p2==p3).
        auto tan1 = p3 - p2;
        tan1 = if_both_then_else((tan1 == 0), p3 - p1, tan1);

        // Find the curve's rotation. Since it cannot inflect or rotate more than 180 degrees at
        // this point, this is equal to the angle between the beginning and ending tangents.
        auto rotation = grvx::approx_angle_between_vectors(tan0, tan1);
        if (countRoundJoin) {
            // Add rotation for the preceding round join.
            auto lastControlPoint = queue.loadLastControlPoint<N>();
            auto nextTan = if_both_then_else((tan0 == 0), tan1, tan0);
            rotation += grvx::approx_angle_between_vectors(p0 - lastControlPoint, nextTan);
        }

        this->writeResolveLevels<N>(numParametricSegments, rotation, queue.fCount,
                                    queue.fResolveLevelPtrs, resultOffset);
    }

    template<int N> SK_ALWAYS_INLINE void writeResolveLevels(
            vec<N> numParametricSegments, vec<N> rotation, int count,
            int8_t* const* resolveLevelPtrs, int offset = 0) const {
        auto numCombinedSegments = grvx::fast_madd<N>(
                fTolerances.fNumRadialSegmentsPerRadian, rotation, numParametricSegments);

        // Find ceil(log2(numCombinedSegments)) by twiddling the exponents. See sk_float_nextlog2().
        auto bits = skvx::bit_pun<uvec<N>>(numCombinedSegments);
        bits += (1u << 23) - 1u;  // Increment the exponent for non-powers-of-2.
        // This will make negative values, denorms, and negative exponents all < 0.
        auto exp = (skvx::bit_pun<ivec<N>>(bits) >> 23) - 127;
        auto level = skvx::pin<N,int>(exp, 0, kMaxResolveLevel);

        switch (count) {
            default: SkUNREACHABLE;
            case 4: ++fResolveLevelCounts[resolveLevelPtrs[3][offset] = level[3]]; [[fallthrough]];
            case 3: ++fResolveLevelCounts[resolveLevelPtrs[2][offset] = level[2]]; [[fallthrough]];
            case 2: ++fResolveLevelCounts[resolveLevelPtrs[1][offset] = level[1]]; [[fallthrough]];
            case 1: ++fResolveLevelCounts[resolveLevelPtrs[0][offset] = level[0]]; break;
        }
    }

    SIMDQueue<2> fLineQueue;
    SIMDQueue<3> fQuadQueue;
    SIMDQueue<4> fCubicQueue;
    SIMDQueue<4> fChoppedCubicQueue;
    struct alignas(sizeof(float) * 8) {
        float fCubicChopTs[8];
    };

    float fWangsTermQuadratic;
    float fWangsTermCubic;

#endif
    int* const fResolveLevelCounts;
    std::array<float, 2> fMatrixMinMaxScales;
    GrStrokeTessellateShader::Tolerances fTolerances;
    int fResolveLevelForCircles;
    bool fIsRoundJoin;
};

}  // namespace

GrStrokeIndirectTessellator::GrStrokeIndirectTessellator(ShaderFlags shaderFlags,
                                                         const SkMatrix& viewMatrix,
                                                         PathStrokeList* pathStrokeList,
                                                         int totalCombinedVerbCnt,
                                                         SkArenaAlloc* alloc)
        : GrStrokeTessellator(shaderFlags, std::move(pathStrokeList)) {
    // The maximum potential number of values we will need in fResolveLevels is:
    //
    //   * 3 segments per verb (from two chops)
    //   * Plus 1 extra resolveLevel per verb that says how many chops it needs
    //   * Plus 2 final resolveLevels for square or round caps at the very end not initiated by a
    //     "kMoveTo".
    int resolveLevelAllocCount = totalCombinedVerbCnt * (3 + 1) + 2;
    fResolveLevels = alloc->makeArrayDefault<int8_t>(resolveLevelAllocCount);
    int8_t* nextResolveLevel = fResolveLevels;

    // The maximum potential number of chopT values we will need is 2 per verb.
    int chopTAllocCount = totalCombinedVerbCnt * 2;
    fChopTs = alloc->makeArrayDefault<float>(chopTAllocCount);
    float* nextChopTs = fChopTs;

    ResolveLevelCounter counter(viewMatrix, fResolveLevelCounts);

    float lastStrokeWidth = -1;
    SkPoint lastControlPoint = {0,0};
    for (PathStrokeList* pathStroke = fPathStrokeList; pathStroke; pathStroke = pathStroke->fNext) {
        const SkStrokeRec& stroke = pathStroke->fStroke;
        SkASSERT(stroke.getWidth() >= 0);  // Otherwise we can't initialize lastStrokeWidth=-1.
        if (stroke.getWidth() != lastStrokeWidth ||
            (stroke.getJoin() == SkPaint::kRound_Join) != counter.isRoundJoin()) {
            counter.updateTolerances(stroke.getWidth(), (stroke.getJoin() == SkPaint::kRound_Join));
            lastStrokeWidth = stroke.getWidth();
        }
        fMaxNumExtraEdgesInJoin = std::max(fMaxNumExtraEdgesInJoin,
                GrStrokeTessellateShader::NumExtraEdgesInIndirectJoin(stroke.getJoin()));
        // Iterate through each verb in the stroke, counting its resolveLevel(s).
        GrStrokeIterator iter(pathStroke->fPath, &stroke, &viewMatrix);
        while (iter.next()) {
            using Verb = GrStrokeIterator::Verb;
            Verb verb = iter.verb();
            if (!GrStrokeIterator::IsVerbGeometric(verb)) {
                // We don't need to handle non-geomtric verbs.
                continue;
            }
            const SkPoint* pts = iter.pts();
            if (counter.isRoundJoin()) {
                // Round joins need a "lastControlPoint" so we can measure the angle of the previous
                // join. This doesn't have to be the exact control point we will send the GPU after
                // any chopping; we just need a direction.
                const SkPoint* prevPts = iter.prevPts();
                switch (iter.prevVerb()) {
                    case Verb::kCubic:
                        if (prevPts[2] != prevPts[3]) {
                            lastControlPoint = prevPts[2];
                            break;
                        }
                        [[fallthrough]];
                    case Verb::kQuad:
                    case Verb::kConic:
                        if (prevPts[1] != prevPts[2]) {
                            lastControlPoint = prevPts[1];
                            break;
                        }
                        [[fallthrough]];
                    case Verb::kLine:
                        lastControlPoint = prevPts[0];
                        break;
                    case Verb::kMoveWithinContour:
                    case Verb::kCircle:
                        // There is no previous stroke to join to. Set lastControlPoint equal to the
                        // current point, which makes the direction 0 and the number of radial
                        // segments in the join 0.
                        lastControlPoint = pts[0];
                        break;
                    case Verb::kContourFinished:
                        SkUNREACHABLE;
                }
            }
            switch (verb) {
                case Verb::kLine:
                    if (counter.countLine(pts, lastControlPoint, nextResolveLevel)) {
                        ++nextResolveLevel;
                    }
                    break;
                case Verb::kConic:
                    // We use the same quadratic formula for conics, ignoring w. This is pretty
                    // close to what the actual number of subdivisions would have been.
                    [[fallthrough]];
                case Verb::kQuad: {
                    // Check for a cusp. A conic of any class can only have a cusp if it is a
                    // degenerate flat line with a 180 degree turnarund. To detect this, the
                    // beginning and ending tangents must be parallel (a.cross(b) == 0) and pointing
                    // in opposite directions (a.dot(b) < 0).
                    SkVector a = pts[1] - pts[0];
                    SkVector b = pts[2] - pts[1];
                    if (a.cross(b) == 0 && a.dot(b) < 0) {
                        // The curve has a cusp. Draw two lines and a circle instead of a quad.
                        int8_t cuspResolveLevel = counter.countCircles(1);
                        *nextResolveLevel++ = -cuspResolveLevel;  // Negative signals a cusp.
                        if (counter.countLine(pts, lastControlPoint, nextResolveLevel)) {
                            ++nextResolveLevel;
                        }
                        ++fResolveLevelCounts[0];  // Second line instance.
                    } else {
                        counter.countQuad(pts, lastControlPoint, nextResolveLevel++);
                    }
                    break;
                }
                case Verb::kCubic: {
                    int8_t cuspResolveLevel = 0;
                    bool areCusps;
                    int numChops = GrPathUtils::findCubicConvex180Chops(pts, nextChopTs, &areCusps);
                    if (areCusps && numChops > 0) {
                        cuspResolveLevel = counter.countCircles(numChops);
                    }
                    if (numChops == 0) {
                        counter.countCubic(pts, lastControlPoint, nextResolveLevel);
                    } else if (numChops == 1) {
                        // A negative resolveLevel indicates how many chops the curve needs, and
                        // whether they are cusps.
                        static_assert(kMaxResolveLevel <= 0xf);
                        SkASSERT(cuspResolveLevel <= 0xf);
                        *nextResolveLevel++ = -((1 << 4) | cuspResolveLevel);
                        counter.countChoppedCubic(pts, nextChopTs[0], lastControlPoint,
                                                  nextResolveLevel);
                    } else {
                        SkASSERT(numChops == 2);
                        // A negative resolveLevel indicates how many chops the curve needs, and
                        // whether they are cusps.
                        static_assert(kMaxResolveLevel <= 0xf);
                        SkASSERT(cuspResolveLevel <= 0xf);
                        *nextResolveLevel++ = -((2 << 4) | cuspResolveLevel);
                        SkPoint pts_[10];
                        SkChopCubicAt(pts, pts_, nextChopTs, 2);
                        counter.countCubic(pts_, lastControlPoint, nextResolveLevel);
                        counter.countCubic(pts_ + 3, pts_[3], nextResolveLevel + 1);
                        counter.countCubic(pts_ + 6, pts_[6], nextResolveLevel + 2);
                    }
                    nextResolveLevel += numChops + 1;
                    nextChopTs += numChops;
                    break;
                }
                case Verb::kCircle:
                    // The iterator implements round caps as circles.
                    *nextResolveLevel++ = counter.countCircles(1);
                    break;
                case Verb::kMoveWithinContour:
                case Verb::kContourFinished:
                    // We should have continued early for non-geometric verbs.
                    SkUNREACHABLE;
                    break;
            }
        }
    }
    counter.flush();

    for (int resolveLevelInstanceCount : fResolveLevelCounts) {
        fTotalInstanceCount += resolveLevelInstanceCount;
        if (resolveLevelInstanceCount) {
            ++fChainedDrawIndirectCount;
        }
    }
    fChainedInstanceCount = fTotalInstanceCount;

#ifdef SK_DEBUG
    SkASSERT(nextResolveLevel <= fResolveLevels + resolveLevelAllocCount);
    fResolveLevelArrayCount = nextResolveLevel - fResolveLevels;
    SkASSERT(nextChopTs <= fChopTs + chopTAllocCount);
    fChopTsArrayCount = nextChopTs - fChopTs;
    fChopTsArrayCount = nextChopTs - fChopTs;
#endif
}

void GrStrokeIndirectTessellator::addToChain(GrStrokeIndirectTessellator* tessellator) {
    SkASSERT(tessellator->fShaderFlags == fShaderFlags);

    fChainedInstanceCount += tessellator->fChainedInstanceCount;
    tessellator->fChainedInstanceCount = 0;

    fChainedDrawIndirectCount += tessellator->fChainedDrawIndirectCount;
    tessellator->fChainedDrawIndirectCount = 0;

    fMaxNumExtraEdgesInJoin = std::max(tessellator->fMaxNumExtraEdgesInJoin,
                                       fMaxNumExtraEdgesInJoin);
    tessellator->fMaxNumExtraEdgesInJoin = 0;

    *fChainTail = tessellator;
    fChainTail = tessellator->fChainTail;
    tessellator->fChainTail = nullptr;
}

namespace {

constexpr static int num_edges_in_resolve_level(int resolveLevel) {
    // A "resolveLevel" means the stroke is composed of 2^resolveLevel line segments.
    int numSegments = 1 << resolveLevel;
    // There are edges both at the beginning and end of a stroke, so there is always one more edge
    // than there are segments.
    int numStrokeEdges = numSegments + 1;
    return numStrokeEdges;
}

// Partitions the instance buffer into bins for each resolveLevel. Writes out indirect draw commands
// per bin. Provides methods to write strokes to their respective bins.
class BinningInstanceWriter {
public:
    using ShaderFlags = GrStrokeTessellateShader::ShaderFlags;
    using DynamicStroke = GrStrokeTessellateShader::DynamicStroke;
    constexpr static int kNumBins = GrStrokeIndirectTessellator::kMaxResolveLevel + 1;

    BinningInstanceWriter(GrDrawIndirectWriter* indirectWriter, GrVertexWriter* instanceWriter,
                          ShaderFlags shaderFlags, size_t instanceStride, int baseInstance,
                          int numExtraEdgesInJoin, const int resolveLevelCounts[kNumBins])
            : fShaderFlags(shaderFlags) {
        SkASSERT(numExtraEdgesInJoin == 3 || numExtraEdgesInJoin == 4);
        // Partition the instance buffer into bins and write out indirect draw commands per bin.
        int runningInstanceCount = 0;
        for (int i = 0; i < kNumBins; ++i) {
            if (resolveLevelCounts[i]) {
                int numEdges = numExtraEdgesInJoin + num_edges_in_resolve_level(i);
                indirectWriter->write(resolveLevelCounts[i], baseInstance + runningInstanceCount,
                                      numEdges * 2, 0);
                fInstanceWriters[i] = instanceWriter->makeOffset(instanceStride *
                                                                 runningInstanceCount);
                fNumEdgesPerResolveLevel[i] = numEdges;
#ifdef SK_DEBUG
            } else {
                fInstanceWriters[i] = {nullptr};
            }
            if (i > 0) {
                fEndWriters[i - 1] = instanceWriter->makeOffset(instanceStride *
                                                                runningInstanceCount);
#endif
            }
            runningInstanceCount += resolveLevelCounts[i];
        }
        SkDEBUGCODE(fEndWriters[kNumBins - 1] =
                            instanceWriter->makeOffset(instanceStride * runningInstanceCount));
        *instanceWriter = instanceWriter->makeOffset(instanceStride * runningInstanceCount);
    }

    void updateDynamicStroke(const SkStrokeRec& stroke) {
        SkASSERT(fShaderFlags & ShaderFlags::kDynamicStroke);
        fDynamicStroke.set(stroke);
    }

    void updateDynamicColor(const SkPMColor4f& color) {
        SkASSERT(fShaderFlags & ShaderFlags::kDynamicColor);
        bool wideColor = fShaderFlags & ShaderFlags::kWideColor;
        SkASSERT(wideColor || color.fitsInBytes());
        fDynamicColor.set(color, wideColor);
    }

    void writeStroke(int8_t resolveLevel, const SkPoint pts[4], SkPoint prevControlPoint,
                     bool isInternalChop = false) {
        SkASSERT(0 <= resolveLevel && resolveLevel < kNumBins);
        float numEdges = fNumEdgesPerResolveLevel[resolveLevel];
        fInstanceWriters[resolveLevel].writeArray(pts, 4);
        fInstanceWriters[resolveLevel].write(prevControlPoint,
                                             // Negative numEdges will tell the GPU that this stroke
                                             // instance follows a chop, and round joins from
                                             // chopping always get exactly one segment.
                                             (isInternalChop) ? -numEdges : +numEdges);
        this->writeDynamicAttribs(resolveLevel);
    }

    // Writes out a 180-degree point stroke, which renders as a circle with a diameter equal to the
    // stroke width. These should be drawn at at cusp points on curves and for round caps.
    void writeCircle(int8_t resolveLevel, SkPoint center) {
        SkASSERT(0 <= resolveLevel && resolveLevel < kNumBins);
        // An empty stroke is a special case that denotes a circle, or 180-degree point stroke.
        fInstanceWriters[resolveLevel].fill(center, 5);
        // Mark numTotalEdges negative so the shader assigns the least possible number of edges to
        // its (empty) preceding join.
        fInstanceWriters[resolveLevel].write(-fNumEdgesPerResolveLevel[resolveLevel]);
        this->writeDynamicAttribs(resolveLevel);
    }

#ifdef SK_DEBUG
    ~BinningInstanceWriter() {
        for (int i = 0; i < kNumBins; ++i) {
            if (fInstanceWriters[i].isValid()) {
                SkASSERT(fInstanceWriters[i] == fEndWriters[i]);
            }
        }
    }
#endif

private:
    void writeDynamicAttribs(int8_t resolveLevel) {
        if (fShaderFlags & ShaderFlags::kDynamicStroke) {
            fInstanceWriters[resolveLevel].write(fDynamicStroke);
        }
        if (fShaderFlags & ShaderFlags::kDynamicColor) {
            fInstanceWriters[resolveLevel].write(fDynamicColor);
        }
    }

    const ShaderFlags fShaderFlags;
    GrVertexWriter fInstanceWriters[kNumBins];
    float fNumEdgesPerResolveLevel[kNumBins];
    SkDEBUGCODE(GrVertexWriter fEndWriters[kNumBins];)

    // Stateful values for the dynamic state (if any) that will get written out with each instance.
    DynamicStroke fDynamicStroke;
    GrVertexColor fDynamicColor;
};

}  // namespace

void GrStrokeIndirectTessellator::prepare(GrMeshDrawOp::Target* target,
                                          const SkMatrix& viewMatrix) {
    SkASSERT(fResolveLevels);
    SkASSERT(!fDrawIndirectBuffer);
    SkASSERT(!fInstanceBuffer);

    if (!fChainedDrawIndirectCount) {
        return;
    }
    SkASSERT(fChainedDrawIndirectCount > 0);
    SkASSERT(fChainedInstanceCount > 0);

    // Allocate indirect draw commands.
    GrDrawIndirectWriter indirectWriter = target->makeDrawIndirectSpace(fChainedDrawIndirectCount,
                                                                        &fDrawIndirectBuffer,
                                                                        &fDrawIndirectOffset);
    if (!indirectWriter.isValid()) {
        SkASSERT(!fDrawIndirectBuffer);
        return;
    }
    SkDEBUGCODE(auto endIndirectWriter = indirectWriter.makeOffset(fChainedDrawIndirectCount));

    // We already know the instance count. Allocate an instance for each.
    int baseInstance;
    size_t instanceStride = GrStrokeTessellateShader::IndirectInstanceStride(fShaderFlags);
    GrVertexWriter instanceWriter = {target->makeVertexSpace(instanceStride, fChainedInstanceCount,
                                                             &fInstanceBuffer, &baseInstance)};
    if (!instanceWriter.isValid()) {
        SkASSERT(!fInstanceBuffer);
        fDrawIndirectBuffer.reset();
        return;
    }
    SkDEBUGCODE(auto endInstanceWriter = instanceWriter.makeOffset(instanceStride *
                                                                   fChainedInstanceCount);)

    // Fill in the indirect-draw and instance buffers.
    for (auto* tess = this; tess; tess = tess->fNextInChain) {
        tess->writeBuffers(&indirectWriter, &instanceWriter, viewMatrix, instanceStride,
                           baseInstance, fMaxNumExtraEdgesInJoin);
        baseInstance += tess->fTotalInstanceCount;
    }

    SkASSERT(indirectWriter == endIndirectWriter);
    SkASSERT(instanceWriter == endInstanceWriter);
}

void GrStrokeIndirectTessellator::writeBuffers(GrDrawIndirectWriter* indirectWriter,
                                               GrVertexWriter* instanceWriter,
                                               const SkMatrix& viewMatrix,
                                               size_t instanceStride, int baseInstance,
                                               int numExtraEdgesInJoin) {
    BinningInstanceWriter binningWriter(indirectWriter, instanceWriter, fShaderFlags,
                                        instanceStride, baseInstance, numExtraEdgesInJoin,
                                        fResolveLevelCounts);

    SkPoint scratchBuffer[4 + 10];
    SkPoint* scratch = scratchBuffer;

    int8_t* nextResolveLevel = fResolveLevels;
    float* nextChopTs = fChopTs;

    SkPoint lastControlPoint = {0,0};
    const SkPoint* firstCubic = nullptr;
    int8_t firstResolveLevel = -1;
    int8_t resolveLevel;

    // Now write out each instance to its resolveLevel's designated location in the instance buffer.
    for (PathStrokeList* pathStroke = fPathStrokeList; pathStroke; pathStroke = pathStroke->fNext) {
        const SkStrokeRec& stroke = pathStroke->fStroke;
        SkASSERT(stroke.getJoin() != SkPaint::kMiter_Join || numExtraEdgesInJoin == 4);
        bool isRoundJoin = (stroke.getJoin() == SkPaint::kRound_Join);
        if (fShaderFlags & ShaderFlags::kDynamicStroke) {
            binningWriter.updateDynamicStroke(stroke);
        }
        if (fShaderFlags & ShaderFlags::kDynamicColor) {
            binningWriter.updateDynamicColor(pathStroke->fColor);
        }
        GrStrokeIterator iter(pathStroke->fPath, &stroke, &viewMatrix);
        bool hasLastControlPoint = false;
        while (iter.next()) {
            using Verb = GrStrokeIterator::Verb;
            int numChops = 0;
            const SkPoint* pts=iter.pts(), *pts_=pts;
            Verb verb = iter.verb();
            switch (verb) {
                case Verb::kCircle:
                    binningWriter.writeCircle(*nextResolveLevel++, pts[0]);
                    [[fallthrough]];
                case Verb::kMoveWithinContour:
                    // The next verb won't be joined to anything.
                    lastControlPoint = pts[0];
                    hasLastControlPoint = true;
                    continue;
                case Verb::kContourFinished:
                    SkASSERT(hasLastControlPoint);
                    if (firstCubic) {
                        // Emit the initial cubic that we deferred at the beginning.
                        binningWriter.writeStroke(firstResolveLevel, firstCubic, lastControlPoint);
                        firstCubic = nullptr;
                    }
                    hasLastControlPoint = false;
                    // Restore "scratch" to the original scratchBuffer.
                    scratch = scratchBuffer;
                    continue;
                case Verb::kLine:
                    resolveLevel = (isRoundJoin) ? *nextResolveLevel++ : 0;
                    scratch[0] = scratch[1] = pts[0];
                    scratch[2] = scratch[3] = pts[1];
                    pts_ = scratch;
                    break;
                case Verb::kQuad:
                    resolveLevel = *nextResolveLevel++;
                    if (resolveLevel < 0) {
                        // The curve has a cusp. Draw two lines and a circle instead of a quad.
                        int8_t cuspResolveLevel = -resolveLevel;
                        float cuspT = SkFindQuadMidTangent(pts);
                        SkPoint cusp = SkEvalQuadAt(pts, cuspT);
                        numChops = 1;
                        scratch[0] = scratch[1] = pts[0];
                        scratch[2] = scratch[3] = scratch[4] = cusp;
                        scratch[5] = scratch[6] = pts[2];
                        binningWriter.writeCircle(cuspResolveLevel, cusp);
                        resolveLevel = (isRoundJoin) ? *nextResolveLevel++ : 0;
                    } else {
                        GrPathUtils::convertQuadToCubic(pts, scratch);
                    }
                    pts_ = scratch;
                    break;
                case Verb::kConic:
                    resolveLevel = *nextResolveLevel++;
                    if (resolveLevel < 0) {
                        // The curve has a cusp. Draw two lines and a cusp instead of a conic.
                        int8_t cuspResolveLevel = -resolveLevel;
                        SkPoint cusp;
                        SkConic conic(pts, iter.w());
                        float cuspT = conic.findMidTangent();
                        conic.evalAt(cuspT, &cusp);
                        numChops = 1;
                        scratch[0] = scratch[1] = pts[0];
                        scratch[2] = scratch[3] = scratch[4] = cusp;
                        scratch[5] = scratch[6] = pts[2];
                        binningWriter.writeCircle(cuspResolveLevel, cusp);
                        resolveLevel = (isRoundJoin) ? *nextResolveLevel++ : 0;
                    } else {
                        GrPathShader::WriteConicPatch(pts, iter.w(), scratch);
                    }
                    pts_ = scratch;
                    break;
                case Verb::kCubic:
                    resolveLevel = *nextResolveLevel++;
                    if (resolveLevel < 0) {
                        // A negative resolveLevel indicates how many chops the curve needs, and
                        // whether they are cusps.
                        numChops = -resolveLevel >> 4;
                        SkChopCubicAt(pts, scratch, nextChopTs, numChops);
                        nextChopTs += numChops;
                        pts_ = scratch;
                        // Are the chop points cusps?
                        if (int8_t cuspResolveLevel = (-resolveLevel & 0xf)) {
                            for (int i = 1; i <= numChops; ++i) {
                                binningWriter.writeCircle(cuspResolveLevel, pts_[i*3]);
                            }
                        }
                        resolveLevel = *nextResolveLevel++;
                    }
                    break;
            }
            for (int i = 0;;) {
                if (!hasLastControlPoint) {
                    SkASSERT(!firstCubic);
                    // Defer the initial cubic until we know its previous control point.
                    firstCubic = pts_;
                    firstResolveLevel = resolveLevel;
                    // Increment the scratch pts in case that's where our first cubic is stored.
                    scratch += 4;
                } else {
                    binningWriter.writeStroke(resolveLevel, pts_, lastControlPoint, (i != 0));
                }
                // Determine the last control point.
                if (pts_[2] != pts_[3] && verb != Verb::kConic) {  // Conics use pts_[3] for w.
                    lastControlPoint = pts_[2];
                } else if (pts_[1] != pts_[2]) {
                    lastControlPoint = pts_[1];
                } else if (pts_[0] != pts_[1]) {
                    lastControlPoint = pts_[0];
                } else {
                    // This is very unusual, but all chops became degenerate. Don't update the
                    // lastControlPoint.
                }
                hasLastControlPoint = true;
                if (i++ == numChops) {
                    break;
                }
                pts_ += 3;
                // If a non-cubic got chopped, it means it was chopped into lines and a circle.
                resolveLevel = (verb == Verb::kCubic) ? *nextResolveLevel++ : 0;
                SkASSERT(verb == Verb::kQuad || verb == Verb::kConic || verb == Verb::kCubic);
            }
        }
    }

    SkASSERT(nextResolveLevel == fResolveLevels + fResolveLevelArrayCount);
    SkASSERT(nextChopTs == fChopTs + fChopTsArrayCount);
}

void GrStrokeIndirectTessellator::draw(GrOpFlushState* flushState) const {
    if (!fDrawIndirectBuffer) {
        return;
    }

    SkASSERT(fChainedDrawIndirectCount > 0);
    SkASSERT(fChainedInstanceCount > 0);

    flushState->bindBuffers(nullptr, fInstanceBuffer, nullptr);
    flushState->drawIndirect(fDrawIndirectBuffer.get(), fDrawIndirectOffset,
                             fChainedDrawIndirectCount);
}
