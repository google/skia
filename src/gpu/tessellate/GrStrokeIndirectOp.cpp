/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokeIndirectOp.h"

#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/tessellate/GrStrokeIterator.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

void GrStrokeIndirectOp::onPrePrepare(GrRecordingContext* context,
                                      const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                      const GrXferProcessor::DstProxyView& dstProxyView,
                                      GrXferBarrierFlags renderPassXferBarriers,
                                      GrLoadOp colorLoadOp) {
    auto* arena = context->priv().recordTimeAllocator();
    this->prePrepareResolveLevels(arena);
    SkASSERT(fResolveLevels);
    if (!fTotalInstanceCount) {
        return;
    }
    this->prePreparePrograms(GrStrokeTessellateShader::Mode::kIndirect, arena, writeView,
                             (clip) ? std::move(*clip) : GrAppliedClip::Disabled(), dstProxyView,
                             renderPassXferBarriers, colorLoadOp, *context->priv().caps());
    if (fFillProgram) {
        context->priv().recordProgramInfo(fFillProgram);
    }
    if (fStencilProgram) {
        context->priv().recordProgramInfo(fStencilProgram);
    }
}

// Helpers for GrStrokeIndirectOp::prePrepareResolveLevels.
namespace {

#ifndef SKNX_NO_SIMD
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
    ResolveLevelCounter(const SkStrokeRec& stroke, GrStrokeTessellateShader::Tolerances tolerances,
                        int* resolveLevelCounts)
            : fIsRoundJoin(stroke.getJoin() == SkPaint::kRound_Join)
            , fTolerances(tolerances)
#ifndef SKNX_NO_SIMD
            , fWangsTermQuadratic(
                    GrWangsFormula::length_term<2>(fTolerances.fParametricIntolerance))
            , fWangsTermCubic(GrWangsFormula::length_term<3>(fTolerances.fParametricIntolerance))
#endif
            , fResolveLevelCounts(resolveLevelCounts) {
    }

    bool isRoundJoin() const { return fIsRoundJoin; }

#ifdef SKNX_NO_SIMD
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
        resolveLevel = std::min(resolveLevel, GrStrokeIndirectOp::kMaxResolveLevel);
        ++fResolveLevelCounts[(*resolveLevelPtr = resolveLevel)];
    }

#else // !defined(SKNX_NO_SIMD)
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
        fCubicChopTs[i] = chopT;
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
            if constexpr (NumPts == 4) {
                // Store the points transposed in 4-point queues. The caller can use a strided load.
                vec<4> x, y;
                grvx::strided_load2(&pts[0].fX, x, y);
                x.store(fXs[fCount].data());
                y.store(fYs[fCount].data());
            } else {
                for (int i = 0; i < NumPts; ++i) {
                    fXs[i][fCount] = pts[i].fX;
                    fYs[i][fCount] = pts[i].fY;
                }
            }
            if (pushRoundJoin) {
                fLastControlPointsX[fCount] = lastControlPoint.fX;
                fLastControlPointsY[fCount] = lastControlPoint.fY;
            }
            fResolveLevelPtrs[fCount] = resolveLevelPtr;
            return fCount++;
        }

        // Loads pts[idx] in SIMD for all 4 strokes, with the x values in the "vec.lo" and the y
        // values in "vec.hi".
        template<int N> SK_ALWAYS_INLINE vec<N*2> loadPoint(int idx) const {
            SkASSERT(0 <= idx && idx < NumPts);
            static_assert(NumPts != 4, "4-point queues store transposed. Use grvx::strided_load4.");
            return skvx::join(vec<N>::Load(fXs[idx].data()), vec<N>::Load(fYs[idx].data()));
        }

        // Loads all 4 lastControlPoints in SIMD, with the x values in "vec.lo" and the y values in
        // "vec.hi".
        template<int N> SK_ALWAYS_INLINE vec<N*2> loadLastControlPoint() const {
            return skvx::join(vec<N>::Load(fLastControlPointsX), vec<N>::Load(fLastControlPointsY));
        }

        std::array<float,4> fXs[NumPts];
        std::array<float,4> fYs[NumPts];
        float fLastControlPointsX[4];
        float fLastControlPointsY[4];
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
        vec<N*2> p0, p1, p2, p3;
        grvx::strided_load4(fCubicQueue.fXs[0].data(), p0.lo, p1.lo, p2.lo, p3.lo);
        grvx::strided_load4(fCubicQueue.fYs[0].data(), p0.hi, p1.hi, p2.hi, p3.hi);
        this->flushCubics<N>(fCubicQueue, p0, p1, p2, p3, fIsRoundJoin, 0);
        fCubicQueue.fCount = 0;
    }

    template<int N> void flushChoppedCubics() {
        SkASSERT(fChoppedCubicQueue.fCount > 0);
        vec<N*2> p0, p1, p2, p3;
        grvx::strided_load4(fChoppedCubicQueue.fXs[0].data(), p0.lo, p1.lo, p2.lo, p3.lo);
        grvx::strided_load4(fChoppedCubicQueue.fYs[0].data(), p0.hi, p1.hi, p2.hi, p3.hi);
        // Chop the cubic at its chopT and find the resolve level for each half.
        auto T = skvx::join(vec<N>::Load(fCubicChopTs), vec<N>::Load(fCubicChopTs));
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
        auto level = skvx::pin<N,int>(exp, 0, GrStrokeIndirectOp::kMaxResolveLevel);

        switch (count) {
            default: SkUNREACHABLE;
            case 4: ++fResolveLevelCounts[resolveLevelPtrs[3][offset] = level[3]]; [[fallthrough]];
            case 3: ++fResolveLevelCounts[resolveLevelPtrs[2][offset] = level[2]]; [[fallthrough]];
            case 2: ++fResolveLevelCounts[resolveLevelPtrs[1][offset] = level[1]]; [[fallthrough]];
            case 1: ++fResolveLevelCounts[resolveLevelPtrs[0][offset] = level[0]]; break;
        }
    }

#endif
    const bool fIsRoundJoin;
    GrStrokeTessellateShader::Tolerances fTolerances;

#ifndef SKNX_NO_SIMD
    const float fWangsTermQuadratic;
    const float fWangsTermCubic;

    SIMDQueue<2> fLineQueue;
    SIMDQueue<3> fQuadQueue;
    SIMDQueue<4> fCubicQueue;
    SIMDQueue<4> fChoppedCubicQueue;
    float fCubicChopTs[4];

#endif
    int* const fResolveLevelCounts;
};

}  // namespace

void GrStrokeIndirectOp::prePrepareResolveLevels(SkArenaAlloc* alloc) {
    SkASSERT(!fTotalInstanceCount);
    SkASSERT(!fResolveLevels);
    SkASSERT(!fResolveLevelArrayCount);

    // The maximum potential number of values we will need in fResolveLevels is:
    //
    //   * 3 segments per verb (from two chops)
    //   * Plus 1 extra resolveLevel per verb that says how many chops it needs
    //   * Plus 2 final resolveLevels for square caps at the very end not initiated by a "kMoveTo".
    //
    int resolveLevelAllocCount = fTotalCombinedVerbCnt * (3 + 1) + 2;
    fResolveLevels = alloc->makeArrayDefault<int8_t>(resolveLevelAllocCount);
    int8_t* nextResolveLevel = fResolveLevels;

    // The maximum potential number of chopT values we will need is 2 per verb.
    int chopTAllocCount = fTotalCombinedVerbCnt * 2;
    fChopTs = alloc->makeArrayDefault<float>(chopTAllocCount);
    float* nextChopTs = fChopTs;

    auto tolerances = this->preTransformTolerances();
    fResolveLevelForCircles =
            sk_float_nextlog2(tolerances.fNumRadialSegmentsPerRadian * SK_ScalarPI);
    ResolveLevelCounter counter(fStroke, tolerances, fResolveLevelCounts);

    SkPoint lastControlPoint = {0,0};
    for (const SkPath& path : fPathList) {
        // Iterate through each verb in the stroke, counting its resolveLevel(s).
        GrStrokeIterator iter(path, &fStroke, &fViewMatrix);
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
                    ++fTotalInstanceCount;
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
                        *nextResolveLevel++ = -1;  // -1 signals a cusp.
                        if (counter.countLine(pts, lastControlPoint, nextResolveLevel)) {
                            ++nextResolveLevel;
                        }
                        ++fResolveLevelCounts[fResolveLevelForCircles];  // Circle instance.
                        ++fResolveLevelCounts[0];  // Second line instance.
                        fTotalInstanceCount += 3;
                    } else {
                        counter.countQuad(pts, lastControlPoint, nextResolveLevel++);
                        ++fTotalInstanceCount;
                    }
                    break;
                }
                case Verb::kCubic: {
                    bool areCusps = false;
                    int numChops = GrPathUtils::findCubicConvex180Chops(pts, nextChopTs, &areCusps);
                    if (areCusps && numChops > 0) {
                        fResolveLevelCounts[fResolveLevelForCircles] += numChops;
                        fTotalInstanceCount += numChops;
                    }
                    if (numChops == 0) {
                        counter.countCubic(pts, lastControlPoint, nextResolveLevel);
                    } else if (numChops == 1) {
                        // A negative resolveLevel indicates how many chops the curve needs, and
                        // whether they are cusps.
                        *nextResolveLevel++ = -((1 << 1) | (int)areCusps);
                        counter.countChoppedCubic(pts, nextChopTs[0], lastControlPoint,
                                                  nextResolveLevel);
                    } else {
                        SkASSERT(numChops == 2);
                        // A negative resolveLevel indicates how many chops the curve needs, and
                        // whether they are cusps.
                        *nextResolveLevel++ = -((2 << 1) | (int)areCusps);
                        SkPoint pts_[10];
                        SkChopCubicAt(pts, pts_, nextChopTs, 2);
                        counter.countCubic(pts_, lastControlPoint, nextResolveLevel);
                        counter.countCubic(pts_ + 3, pts_[3], nextResolveLevel + 1);
                        counter.countCubic(pts_ + 6, pts_[6], nextResolveLevel + 2);
                    }
                    nextResolveLevel += numChops + 1;
                    nextChopTs += numChops;
                    fTotalInstanceCount += numChops + 1;
                    break;
                }
                case Verb::kCircle:
                    // The iterator implements round caps as circles.
                    ++fResolveLevelCounts[fResolveLevelForCircles];
                    ++fTotalInstanceCount;
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

#ifdef SK_DEBUG
    SkASSERT(nextResolveLevel <= fResolveLevels + resolveLevelAllocCount);
    fResolveLevelArrayCount = nextResolveLevel - fResolveLevels;
    SkASSERT(nextChopTs <= fChopTs + chopTAllocCount);
    fChopTsArrayCount = nextChopTs - fChopTs;
    fChopTsArrayCount = nextChopTs - fChopTs;
#endif
}

void GrStrokeIndirectOp::onPrepare(GrOpFlushState* flushState) {
    if (!fResolveLevels) {
        auto* arena = flushState->allocator();
        this->prePrepareResolveLevels(arena);
        if (!fTotalInstanceCount) {
            return;
        }
        this->prePreparePrograms(GrStrokeTessellateShader::Mode::kIndirect, arena,
                                 flushState->writeView(), flushState->detachAppliedClip(),
                                 flushState->dstProxyView(), flushState->renderPassBarriers(),
                                 flushState->colorLoadOp(), flushState->caps());
    }
    SkASSERT(fResolveLevels);

    this->prepareBuffers(flushState);
}

constexpr static int num_edges_in_resolve_level(int resolveLevel) {
    // A resolveLevel means the instance is composed of 2^resolveLevel line segments.
    int numSegments = 1 << resolveLevel;
    // There are edges at the beginning and end both, so there is always one more edge than there
    // are segments.
    int numStrokeEdges = numSegments + 1;
    return numStrokeEdges;
}

void GrStrokeIndirectOp::prepareBuffers(GrMeshDrawOp::Target* target) {
    using IndirectInstance = GrStrokeTessellateShader::IndirectInstance;

    SkASSERT(fResolveLevels);
    SkASSERT(!fDrawIndirectBuffer);
    SkASSERT(!fInstanceBuffer);

    if (!fTotalInstanceCount) {
        return;
    }

    // Allocate enough indirect commands for every resolve level. We will putBack the unused ones
    // at the end.
    GrDrawIndirectCommand* drawIndirectData = target->makeDrawIndirectSpace(
            kMaxResolveLevel + 1, &fDrawIndirectBuffer, &fDrawIndirectOffset);
    if (!drawIndirectData) {
        SkASSERT(!fDrawIndirectBuffer);
        return;
    }

    // We already know the instance count. Allocate an instance for each.
    int baseInstance;
    IndirectInstance* instanceData = static_cast<IndirectInstance*>(target->makeVertexSpace(
            sizeof(IndirectInstance), fTotalInstanceCount, &fInstanceBuffer, &baseInstance));
    if (!instanceData) {
        SkASSERT(!fInstanceBuffer);
        fDrawIndirectBuffer.reset();
        return;
    }

    // Fill out our drawIndirect commands and determine the layout of the instance buffer.
    fDrawIndirectCount = 0;
    int numExtraEdgesInJoin = IndirectInstance::NumExtraEdgesInJoin(fStroke.getJoin());
    int currentInstanceIdx = 0;
    float numEdgesPerResolveLevel[kMaxResolveLevel];
    IndirectInstance* nextInstanceLocations[kMaxResolveLevel + 1];
    SkDEBUGCODE(IndirectInstance* endInstanceLocations[kMaxResolveLevel];)
    for (int i = 0; i <= kMaxResolveLevel; ++i) {
        if (fResolveLevelCounts[i]) {
            int numEdges = numExtraEdgesInJoin + num_edges_in_resolve_level(i);
            auto& cmd = drawIndirectData[fDrawIndirectCount++];
            cmd.fVertexCount = numEdges * 2;
            cmd.fInstanceCount = fResolveLevelCounts[i];
            cmd.fBaseVertex = 0;
            cmd.fBaseInstance = baseInstance + currentInstanceIdx;
            numEdgesPerResolveLevel[i] = numEdges;
            nextInstanceLocations[i] = instanceData + currentInstanceIdx;
#ifdef SK_DEBUG
        } else {
            nextInstanceLocations[i] = nullptr;
        }
        if (i > 0) {
            endInstanceLocations[i - 1] = instanceData + currentInstanceIdx;
            SkASSERT(endInstanceLocations[i - 1] <= instanceData + fTotalInstanceCount);
#endif
        }
        currentInstanceIdx += fResolveLevelCounts[i];
    }
    SkASSERT(currentInstanceIdx == fTotalInstanceCount);
    SkASSERT(fDrawIndirectCount);
    target->putBackIndirectDraws(kMaxResolveLevel + 1 - fDrawIndirectCount);

    SkPoint scratchBuffer[4 + 10];
    SkPoint* scratch = scratchBuffer;

    bool isRoundJoin = (fStroke.getJoin() == SkPaint::kRound_Join);
    int8_t* nextResolveLevel = fResolveLevels;
    float* nextChopTs = fChopTs;

    SkPoint lastControlPoint = {0,0};
    const SkPoint* firstCubic = nullptr;
    int8_t firstResolveLevel = -1;
    int8_t resolveLevel;

    // Now write out each instance to its resolveLevel's designated location in the instance buffer.
    for (const SkPath& path : fPathList) {
        GrStrokeIterator iter(path, &fStroke, &fViewMatrix);
        bool hasLastControlPoint = false;
        while (iter.next()) {
            using Verb = GrStrokeIterator::Verb;
            int numChops = 0;
            const SkPoint* pts=iter.pts(), *pts_=pts;
            Verb verb = iter.verb();
            switch (verb) {
                case Verb::kCircle:
                    nextInstanceLocations[fResolveLevelForCircles]++->setCircle(
                            pts[0], numEdgesPerResolveLevel[fResolveLevelForCircles]);
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
                        nextInstanceLocations[firstResolveLevel]++->set(firstCubic,
                                lastControlPoint, numEdgesPerResolveLevel[firstResolveLevel]);
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
                        SkASSERT(resolveLevel == -1);
                        float cuspT = SkFindQuadMidTangent(pts);
                        SkPoint cusp = SkEvalQuadAt(pts, cuspT);
                        resolveLevel = (isRoundJoin) ? *nextResolveLevel++ : 0;
                        numChops = 1;
                        scratch[0] = scratch[1] = pts[0];
                        scratch[2] = scratch[3] = scratch[4] = cusp;
                        scratch[5] = scratch[6] = pts[2];
                        nextInstanceLocations[fResolveLevelForCircles]++->setCircle(
                                cusp, numEdgesPerResolveLevel[fResolveLevelForCircles]);
                    } else {
                        GrPathUtils::convertQuadToCubic(pts, scratch);
                    }
                    pts_ = scratch;
                    break;
                case Verb::kConic:
                    resolveLevel = *nextResolveLevel++;
                    if (resolveLevel < 0) {
                        // The curve has a cusp. Draw two lines and a cusp instead of a conic.
                        SkASSERT(resolveLevel == -1);
                        SkPoint cusp;
                        SkConic conic(pts, iter.w());
                        float cuspT = conic.findMidTangent();
                        conic.evalAt(cuspT, &cusp);
                        resolveLevel = (isRoundJoin) ? *nextResolveLevel++ : 0;
                        numChops = 1;
                        scratch[0] = scratch[1] = pts[0];
                        scratch[2] = scratch[3] = scratch[4] = cusp;
                        scratch[5] = scratch[6] = pts[2];
                        nextInstanceLocations[fResolveLevelForCircles]++->setCircle(
                                cusp, numEdgesPerResolveLevel[fResolveLevelForCircles]);
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
                        numChops = -resolveLevel >> 1;
                        SkChopCubicAt(pts, scratch, nextChopTs, numChops);
                        nextChopTs += numChops;
                        pts_ = scratch;
                        if (-resolveLevel & 1) {  // Are the chop points cusps?
                            for (int i = 1; i <= numChops; ++i) {
                                nextInstanceLocations[fResolveLevelForCircles]++->setCircle(
                                        pts_[i*3],numEdgesPerResolveLevel[fResolveLevelForCircles]);
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
                    int numEdges = numEdgesPerResolveLevel[resolveLevel];
                    nextInstanceLocations[resolveLevel]++->set(pts_, lastControlPoint,
                            // Negative numEdges will tell the GPU that this stroke instance follows
                            // a chop, and round joins from chopping always get exactly one segment.
                            (i == 0) ? numEdges : -numEdges);
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

#ifdef SK_DEBUG
    SkASSERT(nextResolveLevel == fResolveLevels + fResolveLevelArrayCount);
    SkASSERT(nextChopTs == fChopTs + fChopTsArrayCount);
    auto lastInstanceLocation = nextInstanceLocations[kMaxResolveLevel];
    for (int i = kMaxResolveLevel - 1; i >= 0; --i) {
        if (nextInstanceLocations[i]) {
            SkASSERT(nextInstanceLocations[i] == endInstanceLocations[i]);
        }
        if (!lastInstanceLocation) {
            lastInstanceLocation = nextInstanceLocations[i];
        }
    }
    SkDEBUGCODE(lastInstanceLocation = instanceData + fTotalInstanceCount;)
#endif
}

void GrStrokeIndirectOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    if (!fInstanceBuffer) {
        return;
    }

    SkASSERT(fDrawIndirectCount);
    SkASSERT(fTotalInstanceCount > 0);
    SkASSERT(chainBounds == this->bounds());

    if (fStencilProgram) {
        flushState->bindPipelineAndScissorClip(*fStencilProgram, this->bounds());
        flushState->bindTextures(fStencilProgram->primProc(), nullptr, fStencilProgram->pipeline());
        flushState->bindBuffers(nullptr, fInstanceBuffer, nullptr);
        flushState->drawIndirect(fDrawIndirectBuffer.get(), fDrawIndirectOffset,
                                 fDrawIndirectCount);
    }
    if (fFillProgram) {
        flushState->bindPipelineAndScissorClip(*fFillProgram, this->bounds());
        flushState->bindTextures(fFillProgram->primProc(), nullptr, fFillProgram->pipeline());
        flushState->bindBuffers(nullptr, fInstanceBuffer, nullptr);
        flushState->drawIndirect(fDrawIndirectBuffer.get(), fDrawIndirectOffset,
                                 fDrawIndirectCount);
    }
}
