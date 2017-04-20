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
#include "GrRenderTargetOpList.h"
#include "GrResourceProvider.h"
#include "instanced/InstanceProcessor.h"

namespace gr_instanced {

InstancedRenderingAllocator::InstancedRenderingAllocator(const GrCaps* caps)
    : fDrawPool(1024, 1024)
    , fCaps(SkRef(caps)) {
}

InstancedRenderingAllocator::~InstancedRenderingAllocator() {
    SkSafeUnref(fCaps);
}

InstancedRendering::InstancedRendering(GrGpu* gpu)
    : fGpu(SkRef(gpu))
    , fState1(State::kRecordingDraws) {
}

std::unique_ptr<GrDrawOp> InstancedRenderingAllocator::recordRect(
                                                        const SkRect& rect,
                                                        const SkMatrix& viewMatrix,
                                                        GrPaint&& paint, GrAA aa,
                                                        const GrInstancedPipelineInfo& info) {
    return this->recordShape(ShapeType::kRect, rect, viewMatrix, std::move(paint), rect, aa, info);
}

std::unique_ptr<GrDrawOp> InstancedRenderingAllocator::recordRect(
                                                         const SkRect& rect,
                                                         const SkMatrix& viewMatrix,
                                                         GrPaint&& paint, const SkRect& localRect,
                                                         GrAA aa,
                                                         const GrInstancedPipelineInfo& info) {
    return this->recordShape(ShapeType::kRect, rect, viewMatrix, std::move(paint), localRect, aa,
                             info);
}

std::unique_ptr<GrDrawOp> InstancedRenderingAllocator::recordRect(
                                                         const SkRect& rect,
                                                         const SkMatrix& viewMatrix,
                                                         GrPaint&& paint,
                                                         const SkMatrix& localMatrix, GrAA aa,
                                                         const GrInstancedPipelineInfo& info) {
    if (localMatrix.hasPerspective()) {
        return nullptr; // Perspective is not yet supported in the local matrix.
    }
    if (std::unique_ptr<InstancedOp> op = this->recordShape(ShapeType::kRect, rect, viewMatrix,
                                                            std::move(paint), rect, aa, info)) {
        op->getSingleInstance1().fInfo1 |= kLocalMatrix_InfoFlag;
        op->appendParamsTexel(localMatrix.getScaleX(), localMatrix.getSkewX(),
                              localMatrix.getTranslateX());
        op->appendParamsTexel(localMatrix.getSkewY(), localMatrix.getScaleY(),
                              localMatrix.getTranslateY());
        op->fInfo.fHasLocalMatrix = true;
        return std::move(op);
    }
    return nullptr;
}

std::unique_ptr<GrDrawOp> InstancedRenderingAllocator::recordOval(
                                                         const SkRect& oval,
                                                         const SkMatrix& viewMatrix,
                                                         GrPaint&& paint, GrAA aa,
                                                         const GrInstancedPipelineInfo& info) {
    return this->recordShape(ShapeType::kOval, oval, viewMatrix, std::move(paint), oval, aa, info);
}

std::unique_ptr<GrDrawOp> InstancedRenderingAllocator::recordRRect(
                                                          const SkRRect& rrect,
                                                          const SkMatrix& viewMatrix,
                                                          GrPaint&& paint, GrAA aa,
                                                          const GrInstancedPipelineInfo& info) {
    if (std::unique_ptr<InstancedOp> op =
                this->recordShape(GetRRectShapeType(rrect), rrect.rect(), viewMatrix,
                                  std::move(paint), rrect.rect(), aa, info)) {
        op->appendRRectParams(rrect);
        return std::move(op);
    }
    return nullptr;
}

std::unique_ptr<GrDrawOp> InstancedRenderingAllocator::recordDRRect(
                                                           const SkRRect& outer,
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
    if (std::unique_ptr<InstancedOp> op =
                this->recordShape(GetRRectShapeType(outer), outer.rect(), viewMatrix,
                                  std::move(paint), outer.rect(), aa, info)) {
        op->appendRRectParams(outer);
        ShapeType innerShapeType = GetRRectShapeType(inner);
        op->fInfo.fInnerShapeTypes |= GetShapeFlag(innerShapeType);
        op->getSingleInstance1().fInfo1 |= ((int)innerShapeType << kInnerShapeType_InfoBit);
        op->appendParamsTexel(inner.rect().asScalars(), 4);
        op->appendRRectParams(inner);
        return std::move(op);
    }
    return nullptr;
}

std::unique_ptr<InstancedOp> InstancedRenderingAllocator::recordShape(
        ShapeType type, const SkRect& bounds, const SkMatrix& viewMatrix, GrPaint&& paint,
        const SkRect& localRect, GrAA aa, const GrInstancedPipelineInfo& info) {
    //SkASSERT(State::kRecordingDraws == fState1);

    if (info.fIsRenderingToFloat && fCaps->avoidInstancedDrawsToFPTargets()) {
        return nullptr;
    }

    GrAAType aaType;
    if (!this->selectAntialiasMode(viewMatrix, aa, info, &aaType)) {
        return nullptr;
    }

    GrColor color = paint.getColor();
    std::unique_ptr<InstancedOp> op = this->makeOp(std::move(paint));
    op->fInfo.setAAType(aaType);
    op->fInfo.fShapeTypes = GetShapeFlag(type);
    op->fInfo.fCannotDiscard = true;
    Instance& instance = op->getSingleInstance1();
    instance.fInfo1 = (int)type << kShapeType_InfoBit;

    InstancedOp::HasAABloat aaBloat =
            (aaType == GrAAType::kCoverage) ? InstancedOp::HasAABloat::kYes
                                            : InstancedOp::HasAABloat::kNo;
    InstancedOp::IsZeroArea zeroArea = bounds.isEmpty() ? InstancedOp::IsZeroArea::kYes
                                                        : InstancedOp::IsZeroArea::kNo;

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
        instance.fInfo1 |= kPerspective_InfoFlag;

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

inline bool InstancedRenderingAllocator::selectAntialiasMode(
                                                    const SkMatrix& viewMatrix, GrAA aa,
                                                    const GrInstancedPipelineInfo& info,
                                                    GrAAType* aaType) {
    SkASSERT(!info.fIsMixedSampled || info.fIsMultisampled);
    SkASSERT(GrCaps::InstancedSupport::kNone != fCaps->instancedSupport());

    if (!info.fIsMultisampled || fCaps->multisampleDisableSupport()) {
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
        fCaps->instancedSupport() >= GrCaps::InstancedSupport::kMultisampled) {
        if (!info.fIsMixedSampled) {
            *aaType = GrAAType::kMSAA;
            return true;
        }
        if (fCaps->instancedSupport() >= GrCaps::InstancedSupport::kMixedSampled) {
            *aaType = GrAAType::kMixedSamples;
            return true;
        }
    }

    return false;
}

InstancedOp::InstancedOp(uint32_t classID, GrPaint&& paint, InstancedRenderingAllocator* alloc)
        : INHERITED(classID)
        , fAllocator(alloc)
        , fInstancedRendering1(nullptr)
        , fProcessors(std::move(paint))
        , fIsTracked(false)
        , fRequiresBarrierOnOverlap(false)
        , fNumDraws(1)
        , fNumChangesInGeometry(0)
        , fHeadDraw1(nullptr)
        , fTailDraw1(nullptr) {
    fHeadDraw1 = fTailDraw1 = alloc->fDrawPool.allocate();
#ifdef SK_DEBUG
    fHeadDraw1->fGeometry = {-1, 0};
#endif
    fHeadDraw1->fNext = nullptr;

    static int sID = 0;

    fID = sID++;
}

InstancedOp::~InstancedOp() {
    if (fIsTracked) {
        //SkDebugf("removing %d\n", this->fID);
        fInstancedRendering1->removeOp(this);
    }

    Draw* draw = fHeadDraw1;
    while (draw) {
        Draw* next = draw->fNext;
        fAllocator->fDrawPool.release(draw);
        draw = next;
    }
}

void InstancedOp::appendRRectParams(const SkRRect& rrect) {
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

void InstancedOp::appendParamsTexel(const SkScalar* vals, int count) {
    SkASSERT(!fIsTracked);
    SkASSERT(count <= 4 && count >= 0);
    const float* valsAsFloats = vals; // Ensure SkScalar == float.
    memcpy(&fParams.push_back(), valsAsFloats, count * sizeof(float));
    fInfo.fHasParams = true;
}

void InstancedOp::appendParamsTexel(SkScalar x, SkScalar y, SkScalar z, SkScalar w) {
    SkASSERT(!fIsTracked);
    ParamsTexel& texel = fParams.push_back();
    texel.fX = SkScalarToFloat(x);
    texel.fY = SkScalarToFloat(y);
    texel.fZ = SkScalarToFloat(z);
    texel.fW = SkScalarToFloat(w);
    fInfo.fHasParams = true;
}

void InstancedOp::appendParamsTexel(SkScalar x, SkScalar y, SkScalar z) {
    SkASSERT(!fIsTracked);
    ParamsTexel& texel = fParams.push_back();
    texel.fX = SkScalarToFloat(x);
    texel.fY = SkScalarToFloat(y);
    texel.fZ = SkScalarToFloat(z);
    fInfo.fHasParams = true;
}

void InstancedOp::glom() {
    SkASSERT(fInstancedRendering1);

    this->getSingleInstance1().fInfo1 |= fInstancedRendering1->addOpParams(this);
}

bool InstancedOp::xpRequiresDstTexture(const GrCaps& caps, const GrAppliedClip* clip) {
//    SkASSERT(InstancedRendering::State::kRecordingDraws == fInstancedRendering->fState1);
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
            fProcessors.finalize(this->getSingleInstance1().fColor, coverageInput, clip,
                                 isMixedSamples, caps, &this->getSingleDraw1().fInstance.fColor);

    Draw& draw = this->getSingleDraw1(); // This will assert if we have > 1 command.
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

    fInfo.fCannotTweakAlphaForCoverage = !analysis.isCompatibleWithCoverageAsAlpha();

    fInfo.fUsesLocalCoords = analysis.usesLocalCoords();
    fRequiresBarrierOnOverlap = analysis.requiresBarrierBetweenOverlappingDraws();
    return analysis.requiresDstTexture();
}

void InstancedOp::wasRecorded(GrRenderTargetOpList* opList) {
    SkASSERT(!fInstancedRendering1);
    SkASSERT(!fIsTracked);

    fInstancedRendering1 = opList->instancedRendering();

    this->glom();
    //SkDebugf("adding %d\n", this->fID);
    fInstancedRendering1->addOp(this);
    fIsTracked = true;
}

// called after/in recordOp
bool InstancedOp::onCombineIfPossible(GrOp* other, const GrCaps& caps) {
    InstancedOp* that = static_cast<InstancedOp*>(other);
    SkASSERT(!that->fInstancedRendering1 || (fInstancedRendering1 == that->fInstancedRendering1));
    SkASSERT(fTailDraw1);
    SkASSERT(that->fTailDraw1);

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

    if (!that->fInstancedRendering1) {
        that->fInstancedRendering1 = fInstancedRendering1;
        that->glom();
    }

    this->joinBounds(*that);
    fInfo = combinedInfo;
    fPixelLoad += that->fPixelLoad;
    // Adopt the other op's draws.
    fNumDraws += that->fNumDraws;
    fNumChangesInGeometry += that->fNumChangesInGeometry;
    if (fTailDraw1->fGeometry != that->fHeadDraw1->fGeometry) {
        ++fNumChangesInGeometry;
    }
    fTailDraw1->fNext = that->fHeadDraw1;
    fTailDraw1 = that->fTailDraw1;

    that->fHeadDraw1 = that->fTailDraw1 = nullptr;

    return true;
}

void InstancedRendering::beginFlush(GrResourceProvider* rp) {
    SkASSERT(State::kRecordingDraws == fState1);
    fState1 = State::kFlushing;

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

void InstancedRendering::draw2(const GrPipeline& pipeline,
                              OpInfo info,
                              const InstancedOp* baseOp) {
    InstanceProcessor instProc(info, fParamsBuffer.get());

    this->onDraw1(pipeline, instProc, baseOp);
}

void InstancedOp::onExecute(GrOpFlushState* state) {
    SkASSERT(fInstancedRendering1->isFlushing());
    SkASSERT(state->gpu() == fInstancedRendering1->gpu());

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
    fInstancedRendering1->draw2(pipeline, fInfo, this);
}

void InstancedRendering::endFlush() {
    // The caller is expected to delete all tracked ops (i.e. ops whose applyPipelineOptimizations
    // method has been called) before ending the flush.
    SkASSERT(fTrackedOps.isEmpty());
    fParams.reset();
    fParamsBuffer.reset();
    this->onEndFlush();
    fState1 = State::kRecordingDraws;
    // Hold on to the shape coords and index buffers.
}

void InstancedRendering::resetGpuResources(ResetType resetType) {
    fVertexBuffer.reset();
    fIndexBuffer.reset();
    fParamsBuffer.reset();
    this->onResetGpuResources(resetType);
}

}
