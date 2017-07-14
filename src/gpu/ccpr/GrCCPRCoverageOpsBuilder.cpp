/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCCPRCoverageOpsBuilder.h"

#include "GrBuffer.h"
#include "GrGpuCommandBuffer.h"
#include "GrOnFlushResourceProvider.h"
#include "GrOpFlushState.h"
#include "SkGeometry.h"
#include "SkMakeUnique.h"
#include "SkMathPriv.h"
#include "SkPath.h"
#include "SkPathPriv.h"
#include "SkPoint.h"
#include "SkNx.h"
#include "ops/GrDrawOp.h"
#include "../pathops/SkPathOpsCubic.h"
#include <numeric>

class GrCCPRCoverageOpsBuilder::CoverageOp : public GrDrawOp {
public:
    using PrimitiveTallies = GrCCPRCoverageOpsBuilder::PrimitiveTallies;

    DEFINE_OP_CLASS_ID

    CoverageOp(const SkISize& drawBounds, sk_sp<GrBuffer> pointsBuffer,
               sk_sp<GrBuffer> trianglesBuffer,
               const PrimitiveTallies baseInstances[kNumScissorModes],
               const PrimitiveTallies endInstances[kNumScissorModes], SkTArray<ScissorBatch>&&);

    // GrDrawOp interface.
    const char* name() const override { return "GrCCPRCoverageOpsBuilder::CoverageOp"; }
    FixedFunctionFlags fixedFunctionFlags() const override { return FixedFunctionFlags::kNone; }
    RequiresDstTexture finalize(const GrCaps&, const GrAppliedClip*) override {
        return RequiresDstTexture::kNo;
    }
    bool onCombineIfPossible(GrOp* other, const GrCaps& caps) override { return false; }
    void onPrepare(GrOpFlushState*) override {}
    void onExecute(GrOpFlushState*) override;

private:
    void drawMaskPrimitives(GrOpFlushState*, const GrPipeline&, const GrCCPRCoverageProcessor::Mode,
                            GrPrimitiveType, int vertexCount,
                            int PrimitiveTallies::* instanceType) const;

    const SkISize                  fDrawBounds;
    const sk_sp<GrBuffer>          fPointsBuffer;
    const sk_sp<GrBuffer>          fTrianglesBuffer;
    const PrimitiveTallies         fBaseInstances[GrCCPRCoverageOpsBuilder::kNumScissorModes];
    const PrimitiveTallies         fInstanceCounts[GrCCPRCoverageOpsBuilder::kNumScissorModes];
    const SkTArray<ScissorBatch>   fScissorBatches;

    mutable SkTArray<GrMesh>                     fMeshesScratchBuffer;
    mutable SkTArray<GrPipeline::DynamicState>   fDynamicStatesScratchBuffer;

    typedef GrDrawOp INHERITED;
};

/**
 * This is a view matrix that accumulates two bounding boxes as it maps points: device-space bounds
 * and "45 degree" device-space bounds (| 1 -1 | * devCoords).
 *                                      | 1  1 |
 */
class GrCCPRCoverageOpsBuilder::AccumulatingViewMatrix {
public:
    AccumulatingViewMatrix(const SkMatrix& m, const SkPoint& initialPoint);

    SkPoint transform(const SkPoint& pt);
    void getAccumulatedBounds(SkRect* devBounds, SkRect* devBounds45) const;

private:
    Sk4f fX;
    Sk4f fY;
    Sk4f fT;

    Sk4f fTopLeft;
    Sk4f fBottomRight;
};

static int num_pts(uint8_t verb) {
    switch (verb) {
        case SkPath::kClose_Verb:
        case SkPath::kDone_Verb:
        default:
            SkFAIL("Path verb does not have an endpoint.");
            return 0;
        case SkPath::kMove_Verb:
        case SkPath::kLine_Verb:
            return 1;
        case SkPath::kQuad_Verb:
            return 2;
        case SkPath::kConic_Verb:
            return 2;
        case SkPath::kCubic_Verb:
            return 3;
    }
}

static SkPoint to_skpoint(double x, double y) {
    return {static_cast<SkScalar>(x), static_cast<SkScalar>(y)};
}

static SkPoint to_skpoint(const SkDPoint& dpoint) {
    return to_skpoint(dpoint.fX, dpoint.fY);
}

bool GrCCPRCoverageOpsBuilder::init(GrOnFlushResourceProvider* onFlushRP,
                                    const MaxBufferItems& maxBufferItems) {
    const int maxPoints = maxBufferItems.fMaxFanPoints + maxBufferItems.fMaxControlPoints;
    fPointsBuffer = onFlushRP->makeBuffer(kTexel_GrBufferType, maxPoints * 2 * sizeof(float));
    if (!fPointsBuffer) {
        return false;
    }

    const MaxPrimitives* const maxPrimitives = maxBufferItems.fMaxPrimitives;
    const int maxInstances = (maxPrimitives[0].sum() + maxPrimitives[1].sum());
    fInstanceBuffer = onFlushRP->makeBuffer(kVertex_GrBufferType, maxInstances * 4 * sizeof(int));
    if (!fInstanceBuffer) {
        fPointsBuffer.reset();
        return false;
    }

    fFanPtsIdx = 0;
    fControlPtsIdx = maxBufferItems.fMaxFanPoints;
    SkDEBUGCODE(fMaxFanPoints = maxBufferItems.fMaxFanPoints);
    SkDEBUGCODE(fMaxControlPoints = maxBufferItems.fMaxControlPoints);

    int baseInstance = 0;
    for (int i = 0; i < kNumScissorModes; ++i) {
        fBaseInstances[i].fTriangles = baseInstance;
        baseInstance += maxPrimitives[i].fMaxTriangles;

        fBaseInstances[i].fQuadratics = baseInstance;
        baseInstance += maxPrimitives[i].fMaxQuadratics;

        fBaseInstances[i].fSerpentines = baseInstance;
        baseInstance += maxPrimitives[i].fMaxCubics;

        // Loops grow backwards.
        fBaseInstances[i].fLoops = baseInstance;

        fInstanceIndices[i] = fBaseInstances[i];
    }

    fPointsData = static_cast<SkPoint*>(fPointsBuffer->map());
    SkASSERT(fPointsData);
    GR_STATIC_ASSERT(SK_SCALAR_IS_FLOAT);
    GR_STATIC_ASSERT(8 == sizeof(SkPoint));

    fInstanceData = static_cast<PrimitiveInstance*>(fInstanceBuffer->map());
    SkASSERT(fInstanceData);

    return true;
}

void GrCCPRCoverageOpsBuilder::parsePath(ScissorMode scissorMode, const SkMatrix& viewMatrix,
                                         const SkPath& path, SkRect* devBounds,
                                         SkRect* devBounds45) {
    // Make sure they haven't called finalize yet (or not called init).
    SkASSERT(fPointsData);
    SkASSERT(fInstanceData);

    fCurrScissorMode = scissorMode;
    fCurrPathIndices = fInstanceIndices[(int)fCurrScissorMode];
    fCurrContourStartIdx = fFanPtsIdx;

    const SkPoint* const pts = SkPathPriv::PointData(path);
    int ptsIdx = 0;

    SkASSERT(!path.isEmpty());
    SkASSERT(path.countPoints() > 0);
    AccumulatingViewMatrix m(viewMatrix, pts[0]);

    for (SkPath::Verb verb : SkPathPriv::Verbs(path)) {
        switch (verb) {
            case SkPath::kMove_Verb:
                this->startContour(m, pts[ptsIdx++]);
                continue;
            case SkPath::kClose_Verb:
                this->closeContour();
                continue;
            case SkPath::kLine_Verb:
                this->fanTo(m, pts[ptsIdx]);
                break;
            case SkPath::kQuad_Verb:
                SkASSERT(ptsIdx >= 1); // SkPath should have inserted an implicit moveTo if needed.
                this->quadraticTo(m, &pts[ptsIdx - 1]);
                break;
            case SkPath::kCubic_Verb:
                SkASSERT(ptsIdx >= 1); // SkPath should have inserted an implicit moveTo if needed.
                this->cubicTo(m, &pts[ptsIdx - 1]);
                break;
            case SkPath::kConic_Verb:
                SkFAIL("Conics are not supported.");
            default:
                SkFAIL("Unexpected path verb.");
        }

        ptsIdx += num_pts(verb);
    }

    this->closeContour();

    m.getAccumulatedBounds(devBounds, devBounds45);
    SkDEBUGCODE(this->validate();)
}

void GrCCPRCoverageOpsBuilder::saveParsedPath(const SkIRect& clippedDevIBounds,
                                              int16_t atlasOffsetX, int16_t atlasOffsetY) {
    const PrimitiveTallies& baseIndices = fInstanceIndices[(int)fCurrScissorMode];
    const int32_t packedAtlasOffset = (atlasOffsetY << 16) | (atlasOffsetX & 0xffff);
    for (int i = baseIndices.fTriangles; i < fCurrPathIndices.fTriangles; ++i) {
        fInstanceData[i].fPackedAtlasOffset = packedAtlasOffset;
    }
    for (int i = baseIndices.fQuadratics; i < fCurrPathIndices.fQuadratics; ++i) {
        fInstanceData[i].fPackedAtlasOffset = packedAtlasOffset;
    }
    for (int i = baseIndices.fSerpentines; i < fCurrPathIndices.fSerpentines; ++i) {
        fInstanceData[i].fPackedAtlasOffset = packedAtlasOffset;
    }
    for (int i = baseIndices.fLoops - 1; i >= fCurrPathIndices.fLoops; --i) {
        fInstanceData[i].fPackedAtlasOffset = packedAtlasOffset;
    }
    if (ScissorMode::kScissored == fCurrScissorMode) {
        fScissorBatches.push_back() = {
            fCurrPathIndices - fInstanceIndices[(int)fCurrScissorMode],
            clippedDevIBounds.makeOffset(atlasOffsetX, atlasOffsetY)
        };
    }
    fInstanceIndices[(int)fCurrScissorMode] = fCurrPathIndices;
}

void GrCCPRCoverageOpsBuilder::startContour(AccumulatingViewMatrix& m, const SkPoint& anchorPoint) {
    this->closeContour();
    fCurrPathSpaceAnchorPoint = anchorPoint;
    fPointsData[fFanPtsIdx++] = m.transform(anchorPoint);
    SkASSERT(fCurrContourStartIdx == fFanPtsIdx - 1);
}

void GrCCPRCoverageOpsBuilder::fanTo(AccumulatingViewMatrix& m, const SkPoint& pt) {
    SkASSERT(fCurrContourStartIdx < fFanPtsIdx);
    if (pt == fCurrPathSpaceAnchorPoint) {
        this->startContour(m, pt);
        return;
    }
    fPointsData[fFanPtsIdx++] = m.transform(pt);
}

void GrCCPRCoverageOpsBuilder::quadraticTo(AccumulatingViewMatrix& m, const SkPoint P[3]) {
    SkASSERT(fCurrPathIndices.fQuadratics < fBaseInstances[(int)fCurrScissorMode].fSerpentines);

    this->fanTo(m, P[2]);
    fPointsData[fControlPtsIdx++] = m.transform(P[1]);

    fInstanceData[fCurrPathIndices.fQuadratics++].fQuadraticData = {
        fControlPtsIdx - 1,
        fFanPtsIdx - 2
    };
}

void GrCCPRCoverageOpsBuilder::cubicTo(AccumulatingViewMatrix& m, const SkPoint P[4]) {
    double t[2], s[2];
    SkCubicType type = SkClassifyCubic(P, t, s);

    if (SkCubicType::kLineOrPoint == type) {
        this->fanTo(m, P[3]);
        return;
    }

    if (SkCubicType::kQuadratic == type) {
        SkScalar x1 = P[1].y() - P[0].y(),  y1 = P[0].x() - P[1].x(),
                 k1 = x1 * P[0].x() + y1 * P[0].y();
        SkScalar x2 = P[2].y() - P[3].y(),  y2 = P[3].x() - P[2].x(),
                 k2 = x2 * P[3].x() + y2 * P[3].y();
        SkScalar rdet = 1 / (x1*y2 - y1*x2);
        SkPoint Q[3] = {P[0], {(y2*k1 - y1*k2) * rdet, (x1*k2 - x2*k1) * rdet}, P[3]};
        this->quadraticTo(m, Q);
        return;
    }

    SkDCubic C;
    C.set(P);

    for (int x = 0; x <= 1; ++x) {
        if (t[x] * s[x] <= 0) { // This is equivalent to tx/sx <= 0.
            // This technically also gets taken if tx/sx = infinity, but the code still does
            // the right thing in that edge case.
            continue; // Don't increment x0.
        }
        if (fabs(t[x]) >= fabs(s[x])) { // tx/sx >= 1.
            break;
        }

        const double chopT = double(t[x]) / double(s[x]);
        SkASSERT(chopT >= 0 && chopT <= 1);
        if (chopT <= 0 || chopT >= 1) { // floating-point error.
            continue;
        }

        SkDCubicPair chopped = C.chopAt(chopT);

        // Ensure the double points are identical if this is a loop (more workarounds for FP error).
        if (SkCubicType::kLoop == type && 0 == t[0]) {
            chopped.pts[3] = chopped.pts[0];
        }

        // (This might put ts0/ts1 out of order, but it doesn't matter anymore at this point.)
        this->emitCubicSegment(m, type, chopped.first(),
                               to_skpoint(t[1 - x], s[1 - x] * chopT), to_skpoint(1, 1));
        t[x] = 0;
        s[x] = 1;

        const double r = s[1 - x] * chopT;
        t[1 - x] -= r;
        s[1 - x] -= r;

        C = chopped.second();
    }

    this->emitCubicSegment(m, type, C, to_skpoint(t[0], s[0]), to_skpoint(t[1], s[1]));
}

void GrCCPRCoverageOpsBuilder::emitCubicSegment(AccumulatingViewMatrix& m,
                                                SkCubicType type, const SkDCubic& C,
                                                const SkPoint& ts0, const SkPoint& ts1) {
    SkASSERT(fCurrPathIndices.fSerpentines < fCurrPathIndices.fLoops);

    fPointsData[fControlPtsIdx++] = m.transform(to_skpoint(C[1]));
    fPointsData[fControlPtsIdx++] = m.transform(to_skpoint(C[2]));
    this->fanTo(m, to_skpoint(C[3]));

    // Also emit the cubic's root t,s values as "control points".
    fPointsData[fControlPtsIdx++] = ts0;
    fPointsData[fControlPtsIdx++] = ts1;

    // Serpentines grow up from the front, and loops grow down from the back.
    fInstanceData[SkCubicType::kLoop != type ?
                  fCurrPathIndices.fSerpentines++ : --fCurrPathIndices.fLoops].fCubicData = {
        fControlPtsIdx - 4,
        fFanPtsIdx - 2
    };
}

void GrCCPRCoverageOpsBuilder::closeContour() {
    int fanSize = fFanPtsIdx - fCurrContourStartIdx;
    if (fanSize >= 3) {
         // Technically this can grow to fanSize + log3(fanSize), but we approximate with log2.
        SkAutoSTMalloc<300, int32_t> indices(fanSize + SkNextLog2(fanSize));
        std::iota(indices.get(), indices.get() + fanSize, fCurrContourStartIdx);
        this->emitHierarchicalFan(indices, fanSize);
    }

    // Reset the current contour.
    fCurrContourStartIdx = fFanPtsIdx;
}

void GrCCPRCoverageOpsBuilder::emitHierarchicalFan(int32_t indices[], int count) {
    if (count < 3) {
        return;
    }

    const int32_t oneThirdPt = count / 3;
    const int32_t twoThirdsPt = (2 * count) / 3;
    SkASSERT(fCurrPathIndices.fTriangles < fBaseInstances[(int)fCurrScissorMode].fQuadratics);

    fInstanceData[fCurrPathIndices.fTriangles++].fTriangleData = {
        indices[0],
        indices[oneThirdPt],
        indices[twoThirdsPt]
    };

    this->emitHierarchicalFan(indices, oneThirdPt + 1);
    this->emitHierarchicalFan(&indices[oneThirdPt], twoThirdsPt - oneThirdPt + 1);

    int32_t oldIndex = indices[count];
    indices[count] = indices[0];
    this->emitHierarchicalFan(&indices[twoThirdsPt], count - twoThirdsPt + 1);
    indices[count] = oldIndex;
}

std::unique_ptr<GrDrawOp> GrCCPRCoverageOpsBuilder::createIntermediateOp(SkISize drawBounds) {
    auto op = skstd::make_unique<CoverageOp>(drawBounds, fPointsBuffer, fInstanceBuffer,
                                             fBaseInstances, fInstanceIndices,
                                             std::move(fScissorBatches));
    SkASSERT(fScissorBatches.empty());

    fBaseInstances[0] = fInstanceIndices[0];
    fBaseInstances[1] = fInstanceIndices[1];
    return std::move(op);
}

std::unique_ptr<GrDrawOp> GrCCPRCoverageOpsBuilder::finalize(SkISize drawBounds) {
    fPointsBuffer->unmap();
    SkDEBUGCODE(fPointsData = nullptr);

    fInstanceBuffer->unmap();
    SkDEBUGCODE(fInstanceData = nullptr);

    return skstd::make_unique<CoverageOp>(drawBounds, std::move(fPointsBuffer),
                                          std::move(fInstanceBuffer), fBaseInstances,
                                          fInstanceIndices, std::move(fScissorBatches));
}

#ifdef SK_DEBUG

void GrCCPRCoverageOpsBuilder::validate() {
    SkASSERT(fFanPtsIdx <= fMaxFanPoints);
    SkASSERT(fControlPtsIdx <= fMaxFanPoints + fMaxControlPoints);
    for (int i = 0; i < kNumScissorModes; ++i) {
        SkASSERT(fInstanceIndices[i].fTriangles <= fBaseInstances[i].fQuadratics);
        SkASSERT(fInstanceIndices[i].fQuadratics <= fBaseInstances[i].fSerpentines);
        SkASSERT(fInstanceIndices[i].fSerpentines <= fInstanceIndices[i].fLoops);
    }
}

#endif

using MaxBufferItems = GrCCPRCoverageOpsBuilder::MaxBufferItems;

void MaxBufferItems::countPathItems(GrCCPRCoverageOpsBuilder::ScissorMode scissorMode,
                                    const SkPath& path) {
    MaxPrimitives& maxPrimitives = fMaxPrimitives[(int)scissorMode];
    int currFanPts = 0;

    for (SkPath::Verb verb : SkPathPriv::Verbs(path)) {
        switch (verb) {
        case SkPath::kMove_Verb:
        case SkPath::kClose_Verb:
            fMaxFanPoints += currFanPts;
            maxPrimitives.fMaxTriangles += SkTMax(0, currFanPts - 2);
            currFanPts = SkPath::kMove_Verb == verb ? 1 : 0;
            continue;
        case SkPath::kLine_Verb:
            SkASSERT(currFanPts > 0);
            ++currFanPts;
            continue;
        case SkPath::kQuad_Verb:
            SkASSERT(currFanPts > 0);
            ++currFanPts;
            ++fMaxControlPoints;
            ++maxPrimitives.fMaxQuadratics;
            continue;
        case SkPath::kCubic_Verb: {
            SkASSERT(currFanPts > 0);
            // Over-allocate for the worst case when the cubic is chopped into 3 segments.
            static constexpr int kMaxSegments = 3;
            currFanPts += kMaxSegments;
            // Each cubic segment has two control points.
            fMaxControlPoints += kMaxSegments * 2;
            // Each cubic segment also emits two root t,s values as "control points".
            fMaxControlPoints += kMaxSegments * 2;
            maxPrimitives.fMaxCubics += kMaxSegments;
            // The cubic may also turn out to be a quadratic. While we over-allocate by a fair
            // amount, this is still a relatively small amount of space.
            ++maxPrimitives.fMaxQuadratics;
            continue;
        }
        case SkPath::kConic_Verb:
            SkASSERT(currFanPts > 0);
            SkFAIL("Conics are not supported.");
        default:
            SkFAIL("Unexpected path verb.");
        }
    }

    fMaxFanPoints += currFanPts;
    maxPrimitives.fMaxTriangles += SkTMax(0, currFanPts - 2);

    ++fMaxPaths;
}

using CoverageOp = GrCCPRCoverageOpsBuilder::CoverageOp;

GrCCPRCoverageOpsBuilder::CoverageOp::CoverageOp(const SkISize& drawBounds,
                                             sk_sp<GrBuffer> pointsBuffer,
                                             sk_sp<GrBuffer> trianglesBuffer,
                                             const PrimitiveTallies baseInstances[kNumScissorModes],
                                             const PrimitiveTallies endInstances[kNumScissorModes],
                                             SkTArray<ScissorBatch>&& scissorBatches)
        : INHERITED(ClassID())
        , fDrawBounds(drawBounds)
        , fPointsBuffer(std::move(pointsBuffer))
        , fTrianglesBuffer(std::move(trianglesBuffer))
        , fBaseInstances{baseInstances[0], baseInstances[1]}
        , fInstanceCounts{endInstances[0] - baseInstances[0], endInstances[1] - baseInstances[1]}
        , fScissorBatches(std::move(scissorBatches)) {
    SkASSERT(fPointsBuffer);
    SkASSERT(fTrianglesBuffer);
    this->setBounds(SkRect::MakeIWH(fDrawBounds.width(), fDrawBounds.height()),
                    GrOp::HasAABloat::kNo, GrOp::IsZeroArea::kNo);
}

void CoverageOp::onExecute(GrOpFlushState* flushState) {
    using Mode = GrCCPRCoverageProcessor::Mode;

    SkDEBUGCODE(GrCCPRCoverageProcessor::Validate(flushState->drawOpArgs().fRenderTarget));

    GrPipeline pipeline(flushState->drawOpArgs().fRenderTarget, GrPipeline::ScissorState::kEnabled,
                        SkBlendMode::kPlus);

    fMeshesScratchBuffer.reserve(1 + fScissorBatches.count());
    fDynamicStatesScratchBuffer.reserve(1 + fScissorBatches.count());

    // Triangles.
    auto constexpr kTrianglesGrPrimitiveType = GrCCPRCoverageProcessor::kTrianglesGrPrimitiveType;
    this->drawMaskPrimitives(flushState, pipeline, Mode::kCombinedTriangleHullsAndEdges,
                             kTrianglesGrPrimitiveType, 3, &PrimitiveTallies::fTriangles);
    this->drawMaskPrimitives(flushState, pipeline, Mode::kTriangleCorners,
                             kTrianglesGrPrimitiveType, 3, &PrimitiveTallies::fTriangles);

    // Quadratics.
    auto constexpr kQuadraticsGrPrimitiveType = GrCCPRCoverageProcessor::kQuadraticsGrPrimitiveType;
    this->drawMaskPrimitives(flushState, pipeline, Mode::kQuadraticHulls,
                             kQuadraticsGrPrimitiveType, 3, &PrimitiveTallies::fQuadratics);
    this->drawMaskPrimitives(flushState, pipeline, Mode::kQuadraticFlatEdges,
                             kQuadraticsGrPrimitiveType, 3, &PrimitiveTallies::fQuadratics);

    // Cubics.
    auto constexpr kCubicsGrPrimitiveType = GrCCPRCoverageProcessor::kCubicsGrPrimitiveType;
    this->drawMaskPrimitives(flushState, pipeline, Mode::kSerpentineInsets,
                             kCubicsGrPrimitiveType, 4, &PrimitiveTallies::fSerpentines);
    this->drawMaskPrimitives(flushState, pipeline, Mode::kLoopInsets,
                             kCubicsGrPrimitiveType, 4, &PrimitiveTallies::fLoops);
    this->drawMaskPrimitives(flushState, pipeline, Mode::kSerpentineBorders,
                             kCubicsGrPrimitiveType, 4, &PrimitiveTallies::fSerpentines);
    this->drawMaskPrimitives(flushState, pipeline, Mode::kLoopBorders,
                             kCubicsGrPrimitiveType, 4, &PrimitiveTallies::fLoops);
}

void CoverageOp::drawMaskPrimitives(GrOpFlushState* flushState, const GrPipeline& pipeline,
                                    GrCCPRCoverageProcessor::Mode mode, GrPrimitiveType primType,
                                    int vertexCount, int PrimitiveTallies::* instanceType) const {
    SkASSERT(pipeline.getScissorState().enabled());

    fMeshesScratchBuffer.reset();
    fDynamicStatesScratchBuffer.reset();

    if (const int instanceCount = fInstanceCounts[(int)ScissorMode::kNonScissored].*instanceType) {
        const int baseInstance = fBaseInstances[(int)ScissorMode::kNonScissored].*instanceType;
        // Loops grow backwards, which is indicated by a negative instance count.
        GrMesh& mesh = fMeshesScratchBuffer.emplace_back(primType);
        mesh.setInstanced(fTrianglesBuffer.get(), abs(instanceCount),
                          baseInstance + SkTMin(instanceCount, 0), vertexCount);
        fDynamicStatesScratchBuffer.push_back().fScissorRect.setXYWH(0, 0, fDrawBounds.width(),
                                                                     fDrawBounds.height());
    }

    if (fInstanceCounts[(int)ScissorMode::kScissored].*instanceType) {
        int baseInstance = fBaseInstances[(int)ScissorMode::kScissored].*instanceType;
        for (const ScissorBatch& batch : fScissorBatches) {
            SkASSERT(this->bounds().contains(batch.fScissor));
            const int instanceCount = batch.fInstanceCounts.*instanceType;
            if (!instanceCount) {
                continue;
            }
            // Loops grow backwards, which is indicated by a negative instance count.
            GrMesh& mesh = fMeshesScratchBuffer.emplace_back(primType);
            mesh.setInstanced(fTrianglesBuffer.get(), abs(instanceCount),
                              baseInstance + SkTMin(instanceCount,0), vertexCount);
            fDynamicStatesScratchBuffer.push_back().fScissorRect = batch.fScissor;
            baseInstance += instanceCount;
        }
    }

    SkASSERT(fMeshesScratchBuffer.count() == fDynamicStatesScratchBuffer.count());

    if (!fMeshesScratchBuffer.empty()) {
        GrCCPRCoverageProcessor proc(mode, fPointsBuffer.get());
        flushState->commandBuffer()->draw(pipeline, proc, fMeshesScratchBuffer.begin(),
                                          fDynamicStatesScratchBuffer.begin(),
                                          fMeshesScratchBuffer.count(), this->bounds());
    }
}

using PrimitiveTallies = CoverageOp::PrimitiveTallies;

inline PrimitiveTallies PrimitiveTallies::operator+(const PrimitiveTallies& b) const {
    return {fTriangles + b.fTriangles,
            fQuadratics + b.fQuadratics,
            fSerpentines + b.fSerpentines,
            fLoops + b.fLoops};
}

inline PrimitiveTallies PrimitiveTallies::operator-(const PrimitiveTallies& b) const {
    return {fTriangles - b.fTriangles,
            fQuadratics - b.fQuadratics,
            fSerpentines - b.fSerpentines,
            fLoops - b.fLoops};
}

inline int PrimitiveTallies::sum() const {
    return fTriangles + fQuadratics + fSerpentines + fLoops;
}

using AccumulatingViewMatrix = GrCCPRCoverageOpsBuilder::AccumulatingViewMatrix;

inline AccumulatingViewMatrix::AccumulatingViewMatrix(const SkMatrix& m,
                                                      const SkPoint& initialPoint) {
    // m45 transforms into 45 degree space in order to find the octagon's diagonals. We could
    // use SK_ScalarRoot2Over2 if we wanted an orthonormal transform, but this is irrelevant as
    // long as the shader uses the correct inverse when coming back to device space.
    SkMatrix m45;
    m45.setSinCos(1, 1);
    m45.preConcat(m);

    fX = Sk4f(m.getScaleX(), m.getSkewY(), m45.getScaleX(), m45.getSkewY());
    fY = Sk4f(m.getSkewX(), m.getScaleY(), m45.getSkewX(), m45.getScaleY());
    fT = Sk4f(m.getTranslateX(), m.getTranslateY(), m45.getTranslateX(), m45.getTranslateY());

    Sk4f transformed = SkNx_fma(fY, Sk4f(initialPoint.y()), fT);
    transformed = SkNx_fma(fX, Sk4f(initialPoint.x()), transformed);
    fTopLeft = fBottomRight = transformed;
}

inline SkPoint AccumulatingViewMatrix::transform(const SkPoint& pt) {
    Sk4f transformed = SkNx_fma(fY, Sk4f(pt.y()), fT);
    transformed = SkNx_fma(fX, Sk4f(pt.x()), transformed);

    fTopLeft = Sk4f::Min(fTopLeft, transformed);
    fBottomRight = Sk4f::Max(fBottomRight, transformed);

    // TODO: vst1_lane_f32? (Sk4f::storeLane?)
    float data[4];
    transformed.store(data);
    return SkPoint::Make(data[0], data[1]);
}

inline void AccumulatingViewMatrix::getAccumulatedBounds(SkRect* devBounds,
                                                         SkRect* devBounds45) const {
    float topLeft[4], bottomRight[4];
    fTopLeft.store(topLeft);
    fBottomRight.store(bottomRight);
    devBounds->setLTRB(topLeft[0], topLeft[1], bottomRight[0], bottomRight[1]);
    devBounds45->setLTRB(topLeft[2], topLeft[3], bottomRight[2], bottomRight[3]);
}
