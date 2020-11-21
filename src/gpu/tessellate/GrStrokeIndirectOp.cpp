/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/tessellate/GrStrokeIndirectOp.h"

#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkUtils.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrVx.h"
#include "src/gpu/geometry/GrPathUtils.h"
#include "src/gpu/tessellate/GrStrokeIterator.h"
#include "src/gpu/tessellate/GrStrokeTessellateShader.h"
#include "src/gpu/tessellate/GrWangsFormula.h"

GrStrokeIndirectOp::GrStrokeIndirectOp(GrAAType aaType, const SkMatrix& viewMatrix,
                                       const SkStrokeRec& stroke, const SkPath& path,
                                       GrPaint&& paint)
        : GrStrokeOp(ClassID(), aaType, viewMatrix, stroke, path, std::move(paint))
        , fResolveLevelForCusps(sk_float_nextlog2(fNumRadialSegmentsPerRadian * SK_ScalarPI)) {
}

void GrStrokeIndirectOp::onPrePrepare(GrRecordingContext* context,
                                      const GrSurfaceProxyView& writeView, GrAppliedClip* clip,
                                      const GrXferProcessor::DstProxyView& dstProxyView,
                                      GrXferBarrierFlags renderPassXferBarriers,
                                      GrLoadOp colorLoadOp) {
    auto* arena = context->priv().recordTimeAllocator();
    this->prePrepareResolveLevels(context->priv().recordTimeAllocator());
    if (!fTotalInstanceCount) {
        return;
    }
    auto* strokeTessellateShader = arena->make<GrStrokeTessellateShader>(
            GrStrokeTessellateShader::Mode::kDrawIndirect, fStroke, fParametricIntolerance,
            fNumRadialSegmentsPerRadian, fViewMatrix, fColor);
    this->prePrepareColorProgram(context->priv().recordTimeAllocator(), strokeTessellateShader,
                                 writeView, std::move(*clip), dstProxyView, renderPassXferBarriers,
                                 colorLoadOp, *context->priv().caps());
    context->priv().recordProgramInfo(fColorProgram);
}

// Specialized for working with SIMD.
namespace {

// SIMD types from grvx.
using grvx::vec;
using grvx::ivec;
using grvx::uvec;
using grvx::float4;
using grvx::int4;
using grvx::uint4;

// New SIMD types to represent points. float4x2.lo corresponds to the x coordinates and float4x2.hi
// corresponds to the y.
using float4x2 = vec<8>;
using int4x2 = ivec<8>;

// For each lane, returns "(conds.lo && conds.hi) ? t : e";
static SK_ALWAYS_INLINE float4x2 if_both_then_else(int4x2 conds, float4x2 t, float4x2 e) {
    auto both = conds.lo & conds.hi;
    float4x2 ret;
    ret.lo = skvx::naive_if_then_else(both, t.lo, e.lo);
    ret.hi = skvx::naive_if_then_else(both, t.hi, e.hi);
    return ret;
}

// Returns v.hi^2 + v.lo^2
static SK_ALWAYS_INLINE float4 length_pow2(float4x2 v) {
    return grvx::fast_madd(v.lo, v.lo, v.hi * v.hi);
}

class CubicQueue {
public:
    // Blah.
    enum class DoChop : bool { kNo = false, kYes };

    CubicQueue(bool isRoundJoin, DoChop doChop, float parametricIntolerance,
               float numRadialSegmentsPerRadian, int* resolveLevelCounts)
            : fIsRoundJoin(isRoundJoin)
            , fDoChop((bool)doChop)
            , fWangsTerm(GrWangsFormula::length_term<3>(parametricIntolerance))
            , fNumRadialSegmentsPerRadian(numRadialSegmentsPerRadian)
            , fResolveLevelCounts(resolveLevelCounts) {
    }

    ~CubicQueue() {
        SkASSERT(fQueueCount == 0);  // Always call flush() when finished.
    }

    void setLastControlPointIfRoundJoin(SkPoint lastControlPoint) {
        SkASSERT(fQueueCount >= 0 && fQueueCount < 4);
        if (fIsRoundJoin) {
            fLastControlPointX[fQueueCount] = lastControlPoint.fX;
            fLastControlPointY[fQueueCount] = lastControlPoint.fY;
        }
    }

    void setChopT(float chopT) {
        SkASSERT(fDoChop);
        SkASSERT(fQueueCount >= 0 && fQueueCount < 4);
        SkASSERT(0 < chopT && chopT < 1);
        SkASSERT(SK_ScalarNearlyZero <= chopT && chopT <= 1 - SK_ScalarNearlyZero);
        fChopTs[fQueueCount] = chopT;
    }

    void push(const SkPoint pts[4], int8_t* resolveLevelPtrs) {
        SkASSERT(fQueueCount >= 0 && fQueueCount < 4);
        float4 x, y;
        grvx::strided_load2(&pts[0].fX, x, y);
        x.store(fXs[fQueueCount].data());
        y.store(fYs[fQueueCount].data());
        fResolveLevelPtrs[fQueueCount] = resolveLevelPtrs;
        if (++fQueueCount == 4) {
            this->flush();
        }
    }

    void flush() {
        if (!fQueueCount) {
            return;
        }
        float4x2 p0, p1, p2, p3;
        grvx::strided_load4(fXs[0].data(), p0.lo, p1.lo, p2.lo, p3.lo);
        grvx::strided_load4(fYs[0].data(), p0.hi, p1.hi, p2.hi, p3.hi);
        if (!fDoChop) {
            this->flush(p0, p1, p2, p3, 0, fIsRoundJoin);
        } else {
            float4x2 T;
            T.hi = T.lo = float4::Load(fChopTs);
            float4x2 ab = grvx::unchecked_mix(p0, p1, T);
            float4x2 bc = grvx::unchecked_mix(p1, p2, T);
            float4x2 cd = grvx::unchecked_mix(p2, p3, T);
            float4x2 abc = grvx::unchecked_mix(ab, bc, T);
            float4x2 bcd = grvx::unchecked_mix(bc, cd, T);
            float4x2 abcd = grvx::unchecked_mix(abc, bcd, T);
            this->flush(p0, ab, abc, abcd, 0, fIsRoundJoin);
            this->flush(abcd, bcd, cd, p3, 1, false);
        }
        fQueueCount = 0;
    }

private:
    SK_ALWAYS_INLINE void flush(float4x2 p0, float4x2 p1, float4x2 p2, float4x2 p3,
                                int resultIdx, bool isRoundJoin) const {
        // Execute Wang's formula to determine how many parametric segments the curve needs to be
        // divided into. (See GrWangsFormula::cubic().)
        float4 l0 = length_pow2(grvx::fast_madd<8>(-2, p1, p0) + p2);
        float4 l1 = length_pow2(grvx::fast_madd<8>(-2, p2, p1) + p3);
        float4 numParametricSegments = skvx::sqrt(fWangsTerm * skvx::sqrt(skvx::max(l0, l1)));

        // Find the starting tangent (or zero if p0==p1==p2).
        float4x2 tan0 = p1 - p0;
        tan0 = if_both_then_else((tan0 == 0), p2 - p0, tan0);

        // Find the ending tangent (or zero if p1==p2==p3).
        float4x2 tan1 = p3 - p2;
        tan1 = if_both_then_else((tan1 == 0), p3 - p1, tan1);

        // Find the curve's rotation. Since it cannot inflect or rotate more than 180 degrees at
        // this point, this is equal to the angle between the beginning and ending tangents.
        float4 rotation = grvx::approx_angle_between_vectors(tan0.lo, tan0.hi, tan1.lo, tan1.hi);
        if (isRoundJoin) {
            float4x2 lastControlPoint = skvx::join(float4::Load(fLastControlPointX),
                                                   float4::Load(fLastControlPointY));
            float4x2 lastTangent = p0 - lastControlPoint;
            tan0 = if_both_then_else((tan0 == 0), p3 - p0, tan0);  // TODO: RM if we detect earlier.
            rotation += grvx::approx_angle_between_vectors(lastTangent.lo, lastTangent.hi, tan0.lo,
                                                           tan0.hi);
        }
        float4 numRadialSegments = rotation * fNumRadialSegmentsPerRadian;

        // See GrStrokeOp::NumCombinedSegments. Don't sub1 cuz blah.
        float4 numCombinedSegments = numParametricSegments + numRadialSegments;

        // Find ceil(log2(numCombinedSegments)) by twiddling the exponents. See sk_float_nextlog2().
        uint4 bits = skvx::bit_pun<uint4>(numCombinedSegments);
        bits += (1u << 23) - 1u;  // Increment the exponent for non-powers-of-2.
        // This will make negative values, denorms, and negative exponents all < 0.
        int4 exp = (skvx::bit_pun<int4>(bits) >> 23) - 127;
        int4 resolveLevel = skvx::pin(exp, int4(0), int4(GrStrokeIndirectOp::kMaxResolveLevel));

        if (fQueueCount == 4) {
            ++fResolveLevelCounts[fResolveLevelPtrs[0][resultIdx] = resolveLevel[0]];
            ++fResolveLevelCounts[fResolveLevelPtrs[1][resultIdx] = resolveLevel[1]];
            ++fResolveLevelCounts[fResolveLevelPtrs[2][resultIdx] = resolveLevel[2]];
            ++fResolveLevelCounts[fResolveLevelPtrs[3][resultIdx] = resolveLevel[3]];
        } else for (int i = 0; i < fQueueCount; ++i) {
            ++fResolveLevelCounts[fResolveLevelPtrs[i][resultIdx] = resolveLevel[i]];
        }
    }

    const bool fIsRoundJoin;
    const bool fDoChop;
    const float fWangsTerm;
    const float fNumRadialSegmentsPerRadian;
    int* const fResolveLevelCounts;

    float fLastControlPointX[4];
    float fLastControlPointY[4];
    float fChopTs[4];
    std::array<float,4> fXs[4];
    std::array<float,4> fYs[4];
    int8_t* fResolveLevelPtrs[4];

    int fQueueCount = 0;
};

static bool is_quad_cusp(const SkPoint pts[3], SkPoint* cuspPt = nullptr) {
    SkVector a = pts[1] - pts[0];
    SkVector b = pts[2] - pts[1];
    if (a.cross(b) == 0 && a.dot(b) < 0) {
        if (cuspPt) {
            float T = sk_ieee_float_divide(a.dot(a), a.dot(a - b));
            T = SkTPin(T, 0.f, 1.f);
            *cuspPt = SkEvalQuadAt(pts, T);
        }
        return true;
    }
    return false;
}

}  // namespace

void GrStrokeIndirectOp::prePrepareResolveLevels(SkArenaAlloc* alloc) {
    SkASSERT(!fTotalInstanceCount);
    SkASSERT(!fResolveLevels);
    SkASSERT(!fNumResolveLevels);

    bool isRoundJoin = (fStroke.getJoin() == SkPaint::kRound_Join);
    CubicQueue cubicQueue(isRoundJoin, CubicQueue::DoChop::kNo, fParametricIntolerance,
                          fNumRadialSegmentsPerRadian, fResolveLevelCounts);
    CubicQueue cubicQueue_x2(isRoundJoin, CubicQueue::DoChop::kYes, fParametricIntolerance,
                             fNumRadialSegmentsPerRadian, fResolveLevelCounts);
    int resolveLevelAllocCount = fTotalCombinedVerbCnt*3 + 1;
    fResolveLevels = alloc->makeArray<int8_t>(resolveLevelAllocCount);
    int8_t* nextResolveLevel = fResolveLevels;

    for (const SkPath& path : fPathList) {
        GrStrokeIterator iter(path, fStroke);
        while (iter.advance()) {
            using Verb = GrStrokeIterator::Verb;
            Verb verb = iter.nextVerb();
            const SkPoint* pts = iter.nextPts();
            if (verb == Verb::kMoveWithinContour || verb == Verb::kContourFinished) {
                continue;
            }
            SkPoint lastControlPoint;
            if (isRoundJoin) {
                // Find the last control point so we can measure the angle of the previous join.
                // This doesn't have to be the exact control point we will send the GPU (after
                // chopping); we just need a direction.
                const SkPoint* prevPts = iter.prevPts();
                switch (iter.prevVerb()) {
                    case Verb::kCubic:
                        if (prevPts[2] != prevPts[3]) {
                            lastControlPoint = prevPts[2];
                            break;
                        }
                        [[fallthrough]];
                    case Verb::kQuad:
                        if (prevPts[1] != prevPts[2]) {
                            lastControlPoint = prevPts[1];
                            break;
                        }
                        [[fallthrough]];
                    case Verb::kLine:
                        lastControlPoint = prevPts[0];
                        break;
                    case Verb::kMoveWithinContour:
                    case Verb::kCusp:
                        lastControlPoint = pts[0];
                        break;
                    case Verb::kContourFinished:
                        SkUNREACHABLE;
                }
            }
            switch (verb) {
                case Verb::kMoveWithinContour:
                case Verb::kContourFinished:
                    SkUNREACHABLE;
                    break;
                case Verb::kLine: {
                    int8_t resolveLevel = 0;
                    if (isRoundJoin) {
                        SkVector lastTangent = pts[0] - lastControlPoint;
                        float rotation = SkMeasureAngleBetweenVectors(lastTangent, pts[1] - pts[0]);
                        resolveLevel = sk_float_nextlog2(rotation * fNumRadialSegmentsPerRadian);
                        resolveLevel = std::min(resolveLevel, GrStrokeIndirectOp::kMaxResolveLevel);
                        ++fResolveLevelCounts[(*nextResolveLevel++ = resolveLevel)];
                    } else {
                        ++fResolveLevelCounts[0];
                    }
                    ++fTotalInstanceCount;
                    break;
                }
                case Verb::kQuad: {
                    float numParametricSegments = std::ceil(GrWangsFormula::quadratic(
                            fParametricIntolerance, pts));
                    float rotation = SkMeasureQuadRotation(pts);
                    if (isRoundJoin) {
                        SkVector controlPoint = (pts[0] == pts[1]) ? pts[2] : pts[1];
                        rotation += SkMeasureAngleBetweenVectors(pts[0] - lastControlPoint,
                                                                 controlPoint - pts[0]);
                    }
                    float numRadialSegments = rotation * fNumRadialSegmentsPerRadian;
                    float numCombinedSegments = numParametricSegments + numRadialSegments;
                    int8_t resolveLevel = sk_float_nextlog2(numCombinedSegments);
                    resolveLevel = std::min(resolveLevel, GrStrokeIndirectOp::kMaxResolveLevel);
                    ++fResolveLevelCounts[(*nextResolveLevel++ = resolveLevel)];
                    ++fTotalInstanceCount;
                    if (is_quad_cusp(pts)) {
                        ++fResolveLevelCounts[fResolveLevelForCusps];
                        ++fTotalInstanceCount;
                    }
                    break;
                }
                case Verb::kCubic: {
                    bool areCusps = false;
                    float T[2];
                    int numChops = GrPathUtils::findCubicConvex180Chops(pts, T, &areCusps);
                    if (numChops == 0) {
                        cubicQueue.setLastControlPointIfRoundJoin(lastControlPoint);
                        cubicQueue.push(pts, nextResolveLevel);
                    } else if (numChops == 1) {
                        cubicQueue_x2.setLastControlPointIfRoundJoin(lastControlPoint);
                        cubicQueue_x2.setChopT(T[0]);
                        cubicQueue_x2.push(pts, nextResolveLevel);
                    } else {
                        SkASSERT(numChops == 2);
                        SkPoint pts_[10];
                        SkChopCubicAt(pts, pts_, T, 2);
                        for (int i = 0; i < 3; ++i) {
                            cubicQueue.setLastControlPointIfRoundJoin(
                                    (i == 0) ? lastControlPoint : pts_[i*3]);
                            cubicQueue.push(pts_ + i*3, nextResolveLevel + i);
                        }
                    }
                    nextResolveLevel += numChops + 1;
                    fTotalInstanceCount += numChops + 1;
                    if (areCusps) {
                        fResolveLevelCounts[fResolveLevelForCusps] += numChops;
                        fTotalInstanceCount += numChops;
                    }
                    break;
                }
                case Verb::kCusp:
                   // Round caps are implemented as two cusp points (i.e. circles).
                   ++fResolveLevelCounts[fResolveLevelForCusps];
                   ++fTotalInstanceCount;
                   break;
            }
        }
    }

    cubicQueue.flush();
    cubicQueue_x2.flush();

    SkDEBUGCODE(fNumResolveLevels = nextResolveLevel - fResolveLevels);
    SkASSERT(fNumResolveLevels < resolveLevelAllocCount);
}

void GrStrokeIndirectOp::onPrepare(GrOpFlushState* flushState) {
    if (!fColorProgram) {
        auto* arena = flushState->allocator();
        this->prePrepareResolveLevels(flushState->allocator());
        auto* strokeTessellateShader = arena->make<GrStrokeTessellateShader>(
                GrStrokeTessellateShader::Mode::kDrawIndirect, fStroke, fParametricIntolerance,
            fNumRadialSegmentsPerRadian, fViewMatrix, fColor);
        this->prePrepareColorProgram(arena, strokeTessellateShader, flushState->writeView(),
                                     flushState->detachAppliedClip(), flushState->dstProxyView(),
                                     flushState->renderPassBarriers(), flushState->colorLoadOp(),
                                     flushState->caps());
    }
    SkASSERT(fColorProgram);

    this->prepareBuffers(flushState);
}

namespace {

using IndirectInstance = GrStrokeTessellateShader::IndirectInstance;

constexpr static int num_edges_in_resolve_level(int resolveLevel) {
    int numSegments = 1 << resolveLevel;
    int numStrokeEdges = numSegments + 1;
    return numStrokeEdges;
}

constexpr static int num_extra_edges_in_join(SkPaint::Join joinType) {
    switch (joinType) {
        case SkPaint::kMiter_Join:
            return 4;
        case SkPaint::kRound_Join:  // The rest are counted in the resolve level.
        case SkPaint::kBevel_Join:
            return 3;
    }
}

static void emit_cusp(IndirectInstance* instance, SkPoint pt, float numEdgesForCusps) {
    // An empty stroke denotes a cusp.
    instance->fPts.fill(pt);
    instance->fLastControlPoint = pt;
    instance->fNumTotalEdges = numEdgesForCusps;
}

static SkPoint get_last_control_point_from_cubic_instance(const SkPoint p[4]) {
    SkPoint lastControlPoint = (p[2] == p[3]) ? (p[1] == p[3]) ? p[0] : p[1] : p[2];
    // Epsilons in GrPathUtils::findCubicConvex180Chops should guarantee we never chop
    // so close to a boundary that the curve becomes degenerate. This is an issue for
    // two reasons:
    //
    //   1. The following "p[0] - lastControlPoint" needs to make a valid vector.
    //   2. We reserve degenerate strokes in the GPU shader as a special case for cusps.
    //
    SkASSERT(lastControlPoint != p[3]);
    return lastControlPoint;
}

}  // namespace

void GrStrokeIndirectOp::prepareBuffers(GrMeshDrawOp::Target* target) {
    if (!fTotalInstanceCount) {
        return;
    }

    SkASSERT(!fInstanceBuffer);
    int baseInstance;
    IndirectInstance* instanceData = static_cast<IndirectInstance*>(target->makeVertexSpace(
            sizeof(IndirectInstance), fTotalInstanceCount, &fInstanceBuffer, &baseInstance));

    SkASSERT(!fDrawIndirectBuffer);
    GrDrawIndirectCommand* drawIndirectData = target->makeDrawIndirectSpace(
            kMaxResolveLevel + 1, &fDrawIndirectBuffer, &fDrawIndirectOffset);

    fDrawIndirectCount = 0;
    int numExtraEdgesInJoin = num_extra_edges_in_join(fStroke.getJoin());
    int currentInstanceIdx = 0;
    float numTotalEdges[kMaxResolveLevel];
    IndirectInstance* nextInstanceLocations[kMaxResolveLevel + 1];
    SkDEBUGCODE(IndirectInstance* endInstanceLocations[kMaxResolveLevel + 1];)
    for (int i = 0; i <= kMaxResolveLevel; ++i) {
        if (fResolveLevelCounts[i]) {
            int numEdges = numExtraEdgesInJoin + num_edges_in_resolve_level(i);
            auto& cmd = drawIndirectData[fDrawIndirectCount++];
            cmd.fVertexCount = numEdges * 2;
            cmd.fInstanceCount = fResolveLevelCounts[i];
            cmd.fBaseVertex = 0;
            cmd.fBaseInstance = baseInstance + currentInstanceIdx;
            numTotalEdges[i] = numEdges;
        }
        nextInstanceLocations[i] = instanceData + currentInstanceIdx;
#ifdef SK_DEBUG
        if (i > 0) {
            endInstanceLocations[i - 1] = nextInstanceLocations[i];
            SkASSERT(endInstanceLocations[i - 1] <= instanceData + fTotalInstanceCount);
        }
#endif
        currentInstanceIdx += fResolveLevelCounts[i];
    }
    SkASSERT(currentInstanceIdx == fTotalInstanceCount);
    SkASSERT(fDrawIndirectCount);
    SkDEBUGCODE(endInstanceLocations[kMaxResolveLevel] = instanceData + fTotalInstanceCount;)
    target->putBackIndirectDraws(kMaxResolveLevel + 1 - fDrawIndirectCount);

    int8_t* nextResolveLevel = fResolveLevels;
    bool isRoundJoin = (fStroke.getJoin() == SkPaint::kRound_Join);
    for (const SkPath& path : fPathList) {
        GrStrokeIterator iter(path, fStroke);
        SkPoint scratchPts[10*2];
        SkPoint* pts_ = scratchPts;
        SkPoint lastControlPoint;
        while (iter.advance()) {
            using Verb = GrStrokeIterator::Verb;
            const SkPoint* pts = iter.prevPts();
            int numFinalCubics = 1;
            Verb prevVerb = iter.prevVerb();
            if (iter.nextVerb() == Verb::kContourFinished) {
                // When we get kContourFinished, the prevVerb and prevPts should be the first ones.
                // This is the same stroke we have saved at the beginning of scratchPts.
                SkASSERT(prevVerb == iter.firstVerb());
                SkASSERT(iter.prevPts() == iter.firstPts());
                // This is equivalent to if we had subtracted 1 from the numerator.
                numFinalCubics = (pts_ - scratchPts) / 3;
                // Write out the first stroke that we deferred.
                pts_ = scratchPts;
            } else {
                // Iterate through the "previous" verbs this time. We will save the first one and
                // defer it until the very end. This will give the same ordering as if we had
                // iterated through the "next" points like we did in onPrePrepare.
                switch (prevVerb) {
                    case Verb::kCusp:
                        emit_cusp(nextInstanceLocations[fResolveLevelForCusps]++, pts[0],
                                  numTotalEdges[fResolveLevelForCusps]);
                        [[fallthrough]];
                    case Verb::kMoveWithinContour:
                        // The next verb won't be joined to anything.
                        lastControlPoint = pts[0];
                        continue;
                    case Verb::kLine:
                        pts_[0] = pts_[1] = pts[0];
                        pts_[2] = pts_[3] = pts[1];
                        break;
                    case Verb::kQuad: {
                        SkPoint cuspPt;
                        if (is_quad_cusp(pts, &cuspPt)) {
                            emit_cusp(nextInstanceLocations[fResolveLevelForCusps]++, cuspPt,
                                      numTotalEdges[fResolveLevelForCusps]);
                        }
                        GrPathUtils::convertQuadToCubic(pts, pts_);
                        break;
                    }
                    case Verb::kCubic: {
                        bool areCusps = false;
                        float T[2];
                        int numChops = GrPathUtils::findCubicConvex180Chops(pts, T, &areCusps);
                        if (areCusps) {
                            for (int i = 0; i < numChops; ++i) {
                                SkPoint cuspPt;
                                SkEvalCubicAt(pts, T[i], &cuspPt, nullptr, nullptr);
                                emit_cusp(nextInstanceLocations[fResolveLevelForCusps]++, cuspPt,
                                          numTotalEdges[fResolveLevelForCusps]);
                            }
                        }
                        SkChopCubicAt(pts, pts_, T, numChops);
                        numFinalCubics = numChops + 1;
                        break;
                    }
                    case Verb::kContourFinished:
                        SkUNREACHABLE;
                }
                if (pts_ == scratchPts) {
                    lastControlPoint = get_last_control_point_from_cubic_instance(
                            pts_ + (numFinalCubics - 1) * 3);
                    // Defer the first stroke until the contour is finished and we know its
                    // preceding control point.
                    pts_ += numFinalCubics*3 + 1;
                    continue;
                }
            }
            bool hasSavedResolveLevel = (isRoundJoin || prevVerb != Verb::kLine);
            float m = 1;
            for (const SkPoint* p = pts_, *end = pts_ + numFinalCubics*3; p != end; p += 3) {
                int8_t resolveLevel = (hasSavedResolveLevel) ? *nextResolveLevel++ : 0;
                IndirectInstance* instance = nextInstanceLocations[resolveLevel]++;
                memcpy(instance->fPts.data(), p, sizeof(instance->fPts));
                instance->fLastControlPoint = lastControlPoint;
                instance->fNumTotalEdges = numTotalEdges[resolveLevel] * m;
                lastControlPoint = get_last_control_point_from_cubic_instance(p);
                // Negative fNumTotalEdges will mean the next stroke is a chop, and chops always get
                // one segment in their join.
                m =-1;
            }
        }
    }

    SkASSERT(nextResolveLevel == fResolveLevels + fNumResolveLevels);
    for (int i = 0; i <= kMaxResolveLevel; ++i) {
        SkASSERT(nextInstanceLocations[i] == endInstanceLocations[i]);
    }
}

void GrStrokeIndirectOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    SkASSERT(chainBounds == this->bounds());

    if (!fDrawIndirectBuffer) {
        return;
    }

    flushState->bindPipelineAndScissorClip(*fColorProgram, this->bounds());
    flushState->bindTextures(fColorProgram->primProc(), nullptr, fColorProgram->pipeline());
    flushState->bindBuffers(nullptr, fInstanceBuffer, nullptr);
    flushState->drawIndirect(fDrawIndirectBuffer.get(), fDrawIndirectOffset, fDrawIndirectCount);
}
