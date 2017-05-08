/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "InstancedRendering.h"
#include "GrAppliedClip.h"
#include "GrCaps.h"
#include "GrOpFlushState.h"
#include "GrPipeline.h"
#include "GrResourceProvider.h"
#include "instanced/InstanceProcessor.h"

namespace gr_instanced {

InstancedRendering::InstancedRendering(GrGpu* gpu)
    : fGpu(SkRef(gpu)),
      fState(State::kRecordingDraws),
      fDrawPool(1024, 1024) {
}

std::unique_ptr<GrDrawOp> InstancedRendering::recordRect(const SkRect& rect,
                                                         const SkMatrix& viewMatrix,
                                                         GrPaint&& paint, GrAA aa,
                                                         const GrInstancedPipelineInfo& info) {
    return this->recordShape(ShapeType::kRect, rect, viewMatrix, std::move(paint), rect, aa, info);
}

std::unique_ptr<GrDrawOp> InstancedRendering::recordRect(const SkRect& rect,
                                                         const SkMatrix& viewMatrix,
                                                         GrPaint&& paint, const SkRect& localRect,
                                                         GrAA aa,
                                                         const GrInstancedPipelineInfo& info) {
    return this->recordShape(ShapeType::kRect, rect, viewMatrix, std::move(paint), localRect, aa,
                             info);
}

std::unique_ptr<GrDrawOp> InstancedRendering::recordRect(const SkRect& rect,
                                                         const SkMatrix& viewMatrix,
                                                         GrPaint&& paint,
                                                         const SkMatrix& localMatrix, GrAA aa,
                                                         const GrInstancedPipelineInfo& info) {
    if (localMatrix.hasPerspective()) {
        return nullptr; // Perspective is not yet supported in the local matrix.
    }
    if (std::unique_ptr<Op> op = this->recordShape(ShapeType::kRect, rect, viewMatrix,
                                                   std::move(paint), rect, aa, info)) {
        op->getSingleInstance().fInfo |= kLocalMatrix_InfoFlag;
        op->appendParamsTexel(localMatrix.getScaleX(), localMatrix.getSkewX(),
                              localMatrix.getTranslateX());
        op->appendParamsTexel(localMatrix.getSkewY(), localMatrix.getScaleY(),
                              localMatrix.getTranslateY());
        op->fInfo.fHasLocalMatrix = true;
        return std::move(op);
    }
    return nullptr;
}

std::unique_ptr<GrDrawOp> InstancedRendering::recordOval(const SkRect& oval,
                                                         const SkMatrix& viewMatrix,
                                                         GrPaint&& paint, GrAA aa,
                                                         const GrInstancedPipelineInfo& info) {
    return this->recordShape(ShapeType::kOval, oval, viewMatrix, std::move(paint), oval, aa, info);
}

std::unique_ptr<GrDrawOp> InstancedRendering::recordRRect(const SkRRect& rrect,
                                                          const SkMatrix& viewMatrix,
                                                          GrPaint&& paint, GrAA aa,
                                                          const GrInstancedPipelineInfo& info) {
    if (std::unique_ptr<Op> op =
                this->recordShape(GetRRectShapeType(rrect), rrect.rect(), viewMatrix,
                                  std::move(paint), rrect.rect(), aa, info)) {
        op->appendRRectParams(rrect);
        return std::move(op);
    }
    return nullptr;
}

std::unique_ptr<GrDrawOp> InstancedRendering::recordDRRect(const SkRRect& outer,
                                                           const SkRRect& inner,
                                                           const SkMatrix& viewMatrix,
                                                           GrPaint&& paint, GrAA aa,
                                                           const GrInstancedPipelineInfo& info) {
    if (inner.getType() > SkRRect::kSimple_Type) {
       return nullptr; // Complex inner round rects are not yet supported.
    }
    if (SkRRect::kEmpty_Type == inner.getType()) {
        return this->recordRRect(outer, viewMatrix, std::move(paint), aa, info);
    }
    if (std::unique_ptr<Op> op =
                this->recordShape(GetRRectShapeType(outer), outer.rect(), viewMatrix,
                                  std::move(paint), outer.rect(), aa, info)) {
        op->appendRRectParams(outer);
        ShapeType innerShapeType = GetRRectShapeType(inner);
        op->fInfo.fInnerShapeTypes |= GetShapeFlag(innerShapeType);
        op->getSingleInstance().fInfo |= ((int)innerShapeType << kInnerShapeType_InfoBit);
        op->appendParamsTexel(inner.rect().asScalars(), 4);
        op->appendRRectParams(inner);
        return std::move(op);
    }
    return nullptr;
}

std::unique_ptr<InstancedRendering::Op> InstancedRendering::recordShape(
        ShapeType type, const SkRect& bounds, const SkMatrix& viewMatrix, GrPaint&& paint,
        const SkRect& localRect, GrAA aa, const GrInstancedPipelineInfo& info) {
    SkASSERT(State::kRecordingDraws == fState);

    if (info.fIsRenderingToFloat && fGpu->caps()->avoidInstancedDrawsToFPTargets()) {
        return nullptr;
    }

    GrAAType aaType;
    if (!this->selectAntialiasMode(viewMatrix, aa, info, &aaType)) {
        return nullptr;
    }

    GrColor color = paint.getColor();
    std::unique_ptr<Op> op = this->makeOp(std::move(paint));
    op->fInfo.setAAType(aaType);
    op->fInfo.fShapeTypes = GetShapeFlag(type);
    op->fInfo.fCannotDiscard = true;
    Instance& instance = op->getSingleInstance();
    instance.fInfo = (int)type << kShapeType_InfoBit;

    Op::HasAABloat aaBloat =
            (aaType == GrAAType::kCoverage) ? Op::HasAABloat::kYes : Op::HasAABloat::kNo;
    Op::IsZeroArea zeroArea = (bounds.isEmpty()) ? Op::IsZeroArea::kYes : Op::IsZeroArea::kNo;

    // The instanced shape renderer draws rectangles of [-1, -1, +1, +1], so we find the matrix that
    // will map this rectangle to the same device coordinates as "viewMatrix * bounds".
    float sx = 0.5f * bounds.width();
    float sy = 0.5f * bounds.height();
    float tx = sx + bounds.fLeft;
    float ty = sy + bounds.fTop;
    if (!viewMatrix.hasPerspective()) {
        float* m = instance.fShapeMatrix2x3;
        m[0] = viewMatrix.getScaleX() * sx;
        m[1] = viewMatrix.getSkewX() * sy;
        m[2] = viewMatrix.getTranslateX() +
               viewMatrix.getScaleX() * tx + viewMatrix.getSkewX() * ty;

        m[3] = viewMatrix.getSkewY() * sx;
        m[4] = viewMatrix.getScaleY() * sy;
        m[5] = viewMatrix.getTranslateY() +
               viewMatrix.getSkewY() * tx + viewMatrix.getScaleY() * ty;

        // Since 'm' is a 2x3 matrix that maps the rect [-1, +1] into the shape's device-space quad,
        // it's quite simple to find the bounding rectangle:
        float devBoundsHalfWidth = fabsf(m[0]) + fabsf(m[1]);
        float devBoundsHalfHeight = fabsf(m[3]) + fabsf(m[4]);
        SkRect opBounds;
        opBounds.fLeft = m[2] - devBoundsHalfWidth;
        opBounds.fRight = m[2] + devBoundsHalfWidth;
        opBounds.fTop = m[5] - devBoundsHalfHeight;
        opBounds.fBottom = m[5] + devBoundsHalfHeight;
        op->setBounds(opBounds, aaBloat, zeroArea);

        // TODO: Is this worth the CPU overhead?
        op->fInfo.fNonSquare =
                fabsf(devBoundsHalfHeight - devBoundsHalfWidth) > 0.5f ||  // Early out.
                fabs(m[0] * m[3] + m[1] * m[4]) > 1e-3f ||                 // Skew?
                fabs(m[0] * m[0] + m[1] * m[1] - m[3] * m[3] - m[4] * m[4]) >
                        1e-2f;  // Diff. lengths?
    } else {
        SkMatrix shapeMatrix(viewMatrix);
        shapeMatrix.preTranslate(tx, ty);
        shapeMatrix.preScale(sx, sy);
        instance.fInfo |= kPerspective_InfoFlag;

        float* m = instance.fShapeMatrix2x3;
        m[0] = SkScalarToFloat(shapeMatrix.getScaleX());
        m[1] = SkScalarToFloat(shapeMatrix.getSkewX());
        m[2] = SkScalarToFloat(shapeMatrix.getTranslateX());
        m[3] = SkScalarToFloat(shapeMatrix.getSkewY());
        m[4] = SkScalarToFloat(shapeMatrix.getScaleY());
        m[5] = SkScalarToFloat(shapeMatrix.getTranslateY());

        // Send the perspective column as a param.
        op->appendParamsTexel(shapeMatrix[SkMatrix::kMPersp0], shapeMatrix[SkMatrix::kMPersp1],
                              shapeMatrix[SkMatrix::kMPersp2]);
        op->fInfo.fHasPerspective = true;

        op->setBounds(bounds, aaBloat, zeroArea);
        op->fInfo.fNonSquare = true;
    }

    instance.fColor = color;

    const float* rectAsFloats = localRect.asScalars(); // Ensure SkScalar == float.
    memcpy(&instance.fLocalRect, rectAsFloats, 4 * sizeof(float));

    op->fPixelLoad = op->bounds().height() * op->bounds().width();
    return op;
}

inline bool InstancedRendering::selectAntialiasMode(const SkMatrix& viewMatrix, GrAA aa,
                                                    const GrInstancedPipelineInfo& info,
                                                    GrAAType* aaType) {
    SkASSERT(!info.fIsMixedSampled || info.fIsMultisampled);
    SkASSERT(GrCaps::InstancedSupport::kNone != fGpu->caps()->instancedSupport());

    if (!info.fIsMultisampled || fGpu->caps()->multisampleDisableSupport()) {
        if (GrAA::kNo == aa) {
            *aaType = GrAAType::kNone;
            return true;
        }

        if (info.canUseCoverageAA() && viewMatrix.preservesRightAngles()) {
            *aaType = GrAAType::kCoverage;
            return true;
        }
    }

    if (info.fIsMultisampled &&
        fGpu->caps()->instancedSupport() >= GrCaps::InstancedSupport::kMultisampled) {
        if (!info.fIsMixedSampled) {
            *aaType = GrAAType::kMSAA;
            return true;
        }
        if (fGpu->caps()->instancedSupport() >= GrCaps::InstancedSupport::kMixedSampled) {
            *aaType = GrAAType::kMixedSamples;
            return true;
        }
    }

    return false;
}

InstancedRendering::Op::Op(uint32_t classID, GrPaint&& paint, InstancedRendering* ir)
        : INHERITED(classID)
        , fInstancedRendering(ir)
        , fProcessors(std::move(paint))
        , fIsTracked(false)
        , fRequiresBarrierOnOverlap(false)
        , fNumDraws(1)
        , fNumChangesInGeometry(0) {
    fHeadDraw = fTailDraw = fInstancedRendering->fDrawPool.allocate();
#ifdef SK_DEBUG
    fHeadDraw->fGeometry = {-1, 0};
#endif
    fHeadDraw->fNext = nullptr;
}

InstancedRendering::Op::~Op() {
    if (fIsTracked) {
        fInstancedRendering->fTrackedOps.remove(this);
    }

    Draw* draw = fHeadDraw;
    while (draw) {
        Draw* next = draw->fNext;
        fInstancedRendering->fDrawPool.release(draw);
        draw = next;
    }
}

void InstancedRendering::Op::appendRRectParams(const SkRRect& rrect) {
    SkASSERT(!fIsTracked);
    switch (rrect.getType()) {
        case SkRRect::kSimple_Type: {
            const SkVector& radii = rrect.getSimpleRadii();
            this->appendParamsTexel(radii.x(), radii.y(), rrect.width(), rrect.height());
            return;
        }
        case SkRRect::kNinePatch_Type: {
            float twoOverW = 2 / rrect.width();
            float twoOverH = 2 / rrect.height();
            const SkVector& radiiTL = rrect.radii(SkRRect::kUpperLeft_Corner);
            const SkVector& radiiBR = rrect.radii(SkRRect::kLowerRight_Corner);
            this->appendParamsTexel(radiiTL.x() * twoOverW, radiiBR.x() * twoOverW,
                                    radiiTL.y() * twoOverH, radiiBR.y() * twoOverH);
            return;
        }
        case SkRRect::kComplex_Type: {
            /**
             * The x and y radii of each arc are stored in separate vectors,
             * in the following order:
             *
             *        __x1 _ _ _ x3__
             *    y1 |               | y2
             *
             *       |               |
             *
             *    y3 |__   _ _ _   __| y4
             *          x2       x4
             *
             */
            float twoOverW = 2 / rrect.width();
            float twoOverH = 2 / rrect.height();
            const SkVector& radiiTL = rrect.radii(SkRRect::kUpperLeft_Corner);
            const SkVector& radiiTR = rrect.radii(SkRRect::kUpperRight_Corner);
            const SkVector& radiiBR = rrect.radii(SkRRect::kLowerRight_Corner);
            const SkVector& radiiBL = rrect.radii(SkRRect::kLowerLeft_Corner);
            this->appendParamsTexel(radiiTL.x() * twoOverW, radiiBL.x() * twoOverW,
                                    radiiTR.x() * twoOverW, radiiBR.x() * twoOverW);
            this->appendParamsTexel(radiiTL.y() * twoOverH, radiiTR.y() * twoOverH,
                                    radiiBL.y() * twoOverH, radiiBR.y() * twoOverH);
            return;
        }
        default: return;
    }
}

void InstancedRendering::Op::appendParamsTexel(const SkScalar* vals, int count) {
    SkASSERT(!fIsTracked);
    SkASSERT(count <= 4 && count >= 0);
    const float* valsAsFloats = vals; // Ensure SkScalar == float.
    memcpy(&fParams.push_back(), valsAsFloats, count * sizeof(float));
    fInfo.fHasParams = true;
}

void InstancedRendering::Op::appendParamsTexel(SkScalar x, SkScalar y, SkScalar z, SkScalar w) {
    SkASSERT(!fIsTracked);
    ParamsTexel& texel = fParams.push_back();
    texel.fX = SkScalarToFloat(x);
    texel.fY = SkScalarToFloat(y);
    texel.fZ = SkScalarToFloat(z);
    texel.fW = SkScalarToFloat(w);
    fInfo.fHasParams = true;
}

void InstancedRendering::Op::appendParamsTexel(SkScalar x, SkScalar y, SkScalar z) {
    SkASSERT(!fIsTracked);
    ParamsTexel& texel = fParams.push_back();
    texel.fX = SkScalarToFloat(x);
    texel.fY = SkScalarToFloat(y);
    texel.fZ = SkScalarToFloat(z);
    fInfo.fHasParams = true;
}

bool InstancedRendering::Op::xpRequiresDstTexture(const GrCaps& caps, const GrAppliedClip* clip) {
    SkASSERT(State::kRecordingDraws == fInstancedRendering->fState);
    GrProcessorAnalysisCoverage coverageInput;
    bool isMixedSamples = false;
    if (GrAAType::kCoverage == fInfo.aaType() ||
        (GrAAType::kNone == fInfo.aaType() && !fInfo.isSimpleRects() && fInfo.fCannotDiscard)) {
        coverageInput = GrProcessorAnalysisCoverage::kSingleChannel;
    } else {
        coverageInput = GrProcessorAnalysisCoverage::kNone;
        isMixedSamples = GrAAType::kMixedSamples == fInfo.aaType();
    }
    GrProcessorSet::Analysis analysis =
            fProcessors.finalize(this->getSingleInstance().fColor, coverageInput, clip,
                                 isMixedSamples, caps, &this->getSingleDraw().fInstance.fColor);

    Draw& draw = this->getSingleDraw(); // This will assert if we have > 1 command.
    SkASSERT(draw.fGeometry.isEmpty());
    SkASSERT(SkIsPow2(fInfo.fShapeTypes));
    SkASSERT(!fIsTracked);

    if (kRect_ShapeFlag == fInfo.fShapeTypes) {
        draw.fGeometry = InstanceProcessor::GetIndexRangeForRect(fInfo.aaType());
    } else if (kOval_ShapeFlag == fInfo.fShapeTypes) {
        draw.fGeometry = InstanceProcessor::GetIndexRangeForOval(fInfo.aaType(), this->bounds());
    } else {
        draw.fGeometry = InstanceProcessor::GetIndexRangeForRRect(fInfo.aaType());
    }

    if (!fParams.empty()) {
        SkASSERT(fInstancedRendering->fParams.count() < (int)kParamsIdx_InfoMask); // TODO: cleaner.
        this->getSingleInstance().fInfo |= fInstancedRendering->fParams.count();
        fInstancedRendering->fParams.push_back_n(fParams.count(), fParams.begin());
    }

    fInfo.fCannotTweakAlphaForCoverage = !analysis.isCompatibleWithCoverageAsAlpha();

    fInfo.fUsesLocalCoords = analysis.usesLocalCoords();
    fRequiresBarrierOnOverlap = analysis.requiresBarrierBetweenOverlappingDraws();
    return analysis.requiresDstTexture();
}

void InstancedRendering::Op::wasRecorded() {
    SkASSERT(!fIsTracked);
    fInstancedRendering->fTrackedOps.addToTail(this);
    fIsTracked = true;
}

bool InstancedRendering::Op::onCombineIfPossible(GrOp* other, const GrCaps& caps) {
    Op* that = static_cast<Op*>(other);
    SkASSERT(fInstancedRendering == that->fInstancedRendering);
    SkASSERT(fTailDraw);
    SkASSERT(that->fTailDraw);

    if (!OpInfo::CanCombine(fInfo, that->fInfo) || fProcessors != that->fProcessors) {
        return false;
    }

    SkASSERT(fRequiresBarrierOnOverlap == that->fRequiresBarrierOnOverlap);
    if (fRequiresBarrierOnOverlap && this->bounds().intersects(that->bounds())) {
        return false;
    }
    OpInfo combinedInfo = fInfo | that->fInfo;
    if (!combinedInfo.isSimpleRects()) {
        // This threshold was chosen with the "shapes_mixed" bench on a MacBook with Intel graphics.
        // There seems to be a wide range where it doesn't matter if we combine or not. What matters
        // is that the itty bitty rects combine with other shapes and the giant ones don't.
        constexpr SkScalar kMaxPixelsToGeneralizeRects = 256 * 256;
        if (fInfo.isSimpleRects() && fPixelLoad > kMaxPixelsToGeneralizeRects) {
            return false;
        }
        if (that->fInfo.isSimpleRects() && that->fPixelLoad > kMaxPixelsToGeneralizeRects) {
            return false;
        }
    }

    this->joinBounds(*that);
    fInfo = combinedInfo;
    fPixelLoad += that->fPixelLoad;
    // Adopt the other op's draws.
    fNumDraws += that->fNumDraws;
    fNumChangesInGeometry += that->fNumChangesInGeometry;
    if (fTailDraw->fGeometry != that->fHeadDraw->fGeometry) {
        ++fNumChangesInGeometry;
    }
    fTailDraw->fNext = that->fHeadDraw;
    fTailDraw = that->fTailDraw;

    that->fHeadDraw = that->fTailDraw = nullptr;

    return true;
}

void InstancedRendering::beginFlush(GrResourceProvider* rp) {
    SkASSERT(State::kRecordingDraws == fState);
    fState = State::kFlushing;

    if (fTrackedOps.isEmpty()) {
        return;
    }

    if (!fVertexBuffer) {
        fVertexBuffer.reset(InstanceProcessor::FindOrCreateVertexBuffer(fGpu.get()));
        if (!fVertexBuffer) {
            return;
        }
    }

    if (!fIndexBuffer) {
      fIndexBuffer.reset(InstanceProcessor::FindOrCreateIndex8Buffer(fGpu.get()));
        if (!fIndexBuffer) {
            return;
        }
    }

    if (!fParams.empty()) {
        fParamsBuffer.reset(rp->createBuffer(fParams.count() * sizeof(ParamsTexel),
                                             kTexel_GrBufferType, kDynamic_GrAccessPattern,
                                             GrResourceProvider::kNoPendingIO_Flag |
                                             GrResourceProvider::kRequireGpuMemory_Flag,
                                             fParams.begin()));
        if (!fParamsBuffer) {
            return;
        }
    }

    this->onBeginFlush(rp);
}

void InstancedRendering::Op::onExecute(GrOpFlushState* state) {
    SkASSERT(State::kFlushing == fInstancedRendering->fState);
    SkASSERT(state->gpu() == fInstancedRendering->gpu());

    state->gpu()->handleDirtyContext();

    GrPipeline pipeline;
    GrPipeline::InitArgs args;
    args.fAppliedClip = state->drawOpArgs().fAppliedClip;
    args.fCaps = &state->caps();
    args.fProcessors = &fProcessors;
    args.fFlags = GrAATypeIsHW(fInfo.aaType()) ? GrPipeline::kHWAntialias_Flag : 0;
    args.fRenderTarget = state->drawOpArgs().fRenderTarget;
    args.fDstTexture = state->drawOpArgs().fDstTexture;
    pipeline.init(args);

    if (GrXferBarrierType barrierType = pipeline.xferBarrierType(*state->gpu()->caps())) {
        state->gpu()->xferBarrier(pipeline.getRenderTarget(), barrierType);
    }
    InstanceProcessor instProc(fInfo, fInstancedRendering->fParamsBuffer.get());
    fInstancedRendering->onDraw(pipeline, instProc, this);
}

void InstancedRendering::endFlush() {
    // The caller is expected to delete all tracked ops (i.e. ops whose applyPipelineOptimizations
    // method has been called) before ending the flush.
    SkASSERT(fTrackedOps.isEmpty());
    fParams.reset();
    fParamsBuffer.reset();
    this->onEndFlush();
    fState = State::kRecordingDraws;
    // Hold on to the shape coords and index buffers.
}

void InstancedRendering::resetGpuResources(ResetType resetType) {
    fVertexBuffer.reset();
    fIndexBuffer.reset();
    fParamsBuffer.reset();
    this->onResetGpuResources(resetType);
}

}
