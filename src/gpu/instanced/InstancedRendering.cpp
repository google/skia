/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "InstancedRendering.h"

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

GrDrawOp* InstancedRendering::recordRect(const SkRect& rect, const SkMatrix& viewMatrix,
                                         GrColor color, GrAA aa,
                                         const GrInstancedPipelineInfo& info, GrAAType* aaType) {
    return this->recordShape(ShapeType::kRect, rect, viewMatrix, color, rect, aa, info, aaType);
}

GrDrawOp* InstancedRendering::recordRect(const SkRect& rect, const SkMatrix& viewMatrix,
                                         GrColor color, const SkRect& localRect, GrAA aa,
                                         const GrInstancedPipelineInfo& info, GrAAType* aaType) {
    return this->recordShape(ShapeType::kRect, rect, viewMatrix, color, localRect, aa, info,
                             aaType);
}

GrDrawOp* InstancedRendering::recordRect(const SkRect& rect, const SkMatrix& viewMatrix,
                                         GrColor color, const SkMatrix& localMatrix,
                                         GrAA aa, const GrInstancedPipelineInfo& info,
                                         GrAAType* aaType) {
    if (localMatrix.hasPerspective()) {
        return nullptr; // Perspective is not yet supported in the local matrix.
    }
    if (Batch* batch = this->recordShape(ShapeType::kRect, rect, viewMatrix, color, rect, aa,
                                         info, aaType)) {
        batch->getSingleInstance().fInfo |= kLocalMatrix_InfoFlag;
        batch->appendParamsTexel(localMatrix.getScaleX(), localMatrix.getSkewX(),
                                 localMatrix.getTranslateX());
        batch->appendParamsTexel(localMatrix.getSkewY(), localMatrix.getScaleY(),
                                 localMatrix.getTranslateY());
        batch->fInfo.fHasLocalMatrix = true;
        return batch;
    }
    return nullptr;
}

GrDrawOp* InstancedRendering::recordOval(const SkRect& oval, const SkMatrix& viewMatrix,
                                         GrColor color, GrAA aa,
                                         const GrInstancedPipelineInfo& info, GrAAType* aaType) {
    return this->recordShape(ShapeType::kOval, oval, viewMatrix, color, oval, aa, info, aaType);
}

GrDrawOp* InstancedRendering::recordRRect(const SkRRect& rrect, const SkMatrix& viewMatrix,
                                          GrColor color, GrAA aa,
                                          const GrInstancedPipelineInfo& info, GrAAType* aaType) {
    if (Batch* batch = this->recordShape(GetRRectShapeType(rrect), rrect.rect(), viewMatrix, color,
                                         rrect.rect(), aa, info, aaType)) {
        batch->appendRRectParams(rrect);
        return batch;
    }
    return nullptr;
}

GrDrawOp* InstancedRendering::recordDRRect(const SkRRect& outer, const SkRRect& inner,
                                           const SkMatrix& viewMatrix, GrColor color,
                                           GrAA aa, const GrInstancedPipelineInfo& info,
                                           GrAAType* aaType) {
    if (inner.getType() > SkRRect::kSimple_Type) {
       return nullptr; // Complex inner round rects are not yet supported.
    }
    if (SkRRect::kEmpty_Type == inner.getType()) {
        return this->recordRRect(outer, viewMatrix, color, aa, info, aaType);
    }
    if (Batch* batch = this->recordShape(GetRRectShapeType(outer), outer.rect(), viewMatrix, color,
                                         outer.rect(), aa, info, aaType)) {
        batch->appendRRectParams(outer);
        ShapeType innerShapeType = GetRRectShapeType(inner);
        batch->fInfo.fInnerShapeTypes |= GetShapeFlag(innerShapeType);
        batch->getSingleInstance().fInfo |= ((int)innerShapeType << kInnerShapeType_InfoBit);
        batch->appendParamsTexel(inner.rect().asScalars(), 4);
        batch->appendRRectParams(inner);
        return batch;
    }
    return nullptr;
}

InstancedRendering::Batch* InstancedRendering::recordShape(ShapeType type, const SkRect& bounds,
                                                           const SkMatrix& viewMatrix,
                                                           GrColor color, const SkRect& localRect,
                                                           GrAA aa,
                                                           const GrInstancedPipelineInfo& info,
                                                           GrAAType* aaType) {
    SkASSERT(State::kRecordingDraws == fState);

    if (info.fIsRenderingToFloat && fGpu->caps()->avoidInstancedDrawsToFPTargets()) {
        return nullptr;
    }

    AntialiasMode antialiasMode;
    if (!this->selectAntialiasMode(viewMatrix, aa, info, aaType, &antialiasMode)) {
        return nullptr;
    }

    Batch* batch = this->createBatch();
    batch->fInfo.fAntialiasMode = antialiasMode;
    batch->fInfo.fShapeTypes = GetShapeFlag(type);
    batch->fInfo.fCannotDiscard = !info.fCanDiscard;

    Instance& instance = batch->getSingleInstance();
    instance.fInfo = (int)type << kShapeType_InfoBit;

    Batch::HasAABloat aaBloat = (antialiasMode == AntialiasMode::kCoverage)
                                ? Batch::HasAABloat::kYes
                                : Batch::HasAABloat::kNo;
    Batch::IsZeroArea zeroArea = (bounds.isEmpty()) ? Batch::IsZeroArea::kYes
                                                    : Batch::IsZeroArea::kNo;

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
        SkRect batchBounds;
        batchBounds.fLeft = m[2] - devBoundsHalfWidth;
        batchBounds.fRight = m[2] + devBoundsHalfWidth;
        batchBounds.fTop = m[5] - devBoundsHalfHeight;
        batchBounds.fBottom = m[5] + devBoundsHalfHeight;
        batch->setBounds(batchBounds, aaBloat, zeroArea);

        // TODO: Is this worth the CPU overhead?
        batch->fInfo.fNonSquare =
            fabsf(devBoundsHalfHeight - devBoundsHalfWidth) > 0.5f || // Early out.
            fabs(m[0] * m[3] + m[1] * m[4]) > 1e-3f || // Skew?
            fabs(m[0] * m[0] + m[1] * m[1] - m[3] * m[3] - m[4] * m[4]) > 1e-2f; // Diff. lengths?
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
        batch->appendParamsTexel(shapeMatrix[SkMatrix::kMPersp0], shapeMatrix[SkMatrix::kMPersp1],
                                 shapeMatrix[SkMatrix::kMPersp2]);
        batch->fInfo.fHasPerspective = true;

        batch->setBounds(bounds, aaBloat, zeroArea);
        batch->fInfo.fNonSquare = true;
    }

    instance.fColor = color;

    const float* rectAsFloats = localRect.asScalars(); // Ensure SkScalar == float.
    memcpy(&instance.fLocalRect, rectAsFloats, 4 * sizeof(float));

    batch->fPixelLoad = batch->bounds().height() * batch->bounds().width();
    return batch;
}

inline bool InstancedRendering::selectAntialiasMode(const SkMatrix& viewMatrix, GrAA aa,
                                                    const GrInstancedPipelineInfo& info,
                                                    GrAAType* aaType,
                                                    AntialiasMode* antialiasMode) {
    SkASSERT(!info.fColorDisabled || info.fDrawingShapeToStencil);
    SkASSERT(!info.fIsMixedSampled || info.fIsMultisampled);
    SkASSERT(GrCaps::InstancedSupport::kNone != fGpu->caps()->instancedSupport());

    if (!info.fIsMultisampled || fGpu->caps()->multisampleDisableSupport()) {
        if (GrAA::kNo == aa) {
            if (info.fDrawingShapeToStencil && !info.fCanDiscard) {
                // We can't draw to the stencil buffer without discard (or sample mask if MSAA).
                return false;
            }
            *antialiasMode = AntialiasMode::kNone;
            *aaType = GrAAType::kNone;
            return true;
        }

        if (info.canUseCoverageAA() && viewMatrix.preservesRightAngles()) {
            *antialiasMode = AntialiasMode::kCoverage;
            *aaType = GrAAType::kCoverage;
            return true;
        }
    }

    if (info.fIsMultisampled &&
        fGpu->caps()->instancedSupport() >= GrCaps::InstancedSupport::kMultisampled) {
        if (!info.fIsMixedSampled || info.fColorDisabled) {
            *antialiasMode = AntialiasMode::kMSAA;
            *aaType = GrAAType::kMSAA;
            return true;
        }
        if (fGpu->caps()->instancedSupport() >= GrCaps::InstancedSupport::kMixedSampled) {
            *antialiasMode = AntialiasMode::kMixedSamples;
            *aaType = GrAAType::kMixedSamples;
            return true;
        }
    }

    return false;
}

InstancedRendering::Batch::Batch(uint32_t classID, InstancedRendering* ir)
    : INHERITED(classID),
      fInstancedRendering(ir),
      fIsTracked(false),
      fNumDraws(1),
      fNumChangesInGeometry(0) {
    fHeadDraw = fTailDraw = fInstancedRendering->fDrawPool.allocate();
#ifdef SK_DEBUG
    fHeadDraw->fGeometry = {-1, 0};
#endif
    fHeadDraw->fNext = nullptr;
}

InstancedRendering::Batch::~Batch() {
    if (fIsTracked) {
        fInstancedRendering->fTrackedBatches.remove(this);
    }

    Draw* draw = fHeadDraw;
    while (draw) {
        Draw* next = draw->fNext;
        fInstancedRendering->fDrawPool.release(draw);
        draw = next;
    }
}

void InstancedRendering::Batch::appendRRectParams(const SkRRect& rrect) {
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

void InstancedRendering::Batch::appendParamsTexel(const SkScalar* vals, int count) {
    SkASSERT(!fIsTracked);
    SkASSERT(count <= 4 && count >= 0);
    const float* valsAsFloats = vals; // Ensure SkScalar == float.
    memcpy(&fParams.push_back(), valsAsFloats, count * sizeof(float));
    fInfo.fHasParams = true;
}

void InstancedRendering::Batch::appendParamsTexel(SkScalar x, SkScalar y, SkScalar z, SkScalar w) {
    SkASSERT(!fIsTracked);
    ParamsTexel& texel = fParams.push_back();
    texel.fX = SkScalarToFloat(x);
    texel.fY = SkScalarToFloat(y);
    texel.fZ = SkScalarToFloat(z);
    texel.fW = SkScalarToFloat(w);
    fInfo.fHasParams = true;
}

void InstancedRendering::Batch::appendParamsTexel(SkScalar x, SkScalar y, SkScalar z) {
    SkASSERT(!fIsTracked);
    ParamsTexel& texel = fParams.push_back();
    texel.fX = SkScalarToFloat(x);
    texel.fY = SkScalarToFloat(y);
    texel.fZ = SkScalarToFloat(z);
    fInfo.fHasParams = true;
}

void InstancedRendering::Batch::computePipelineOptimizations(GrInitInvariantOutput* color,
                                                            GrInitInvariantOutput* coverage,
                                                            GrBatchToXPOverrides* overrides) const {
    color->setKnownFourComponents(this->getSingleInstance().fColor);

    if (AntialiasMode::kCoverage == fInfo.fAntialiasMode ||
        (AntialiasMode::kNone == fInfo.fAntialiasMode &&
         !fInfo.isSimpleRects() && fInfo.fCannotDiscard)) {
        coverage->setUnknownSingleComponent();
    } else {
        coverage->setKnownSingleComponent(255);
    }
}

void InstancedRendering::Batch::initBatchTracker(const GrXPOverridesForBatch& overrides) {
    Draw& draw = this->getSingleDraw(); // This will assert if we have > 1 command.
    SkASSERT(draw.fGeometry.isEmpty());
    SkASSERT(SkIsPow2(fInfo.fShapeTypes));
    SkASSERT(!fIsTracked);

    if (kRect_ShapeFlag == fInfo.fShapeTypes) {
        draw.fGeometry = InstanceProcessor::GetIndexRangeForRect(fInfo.fAntialiasMode);
    } else if (kOval_ShapeFlag == fInfo.fShapeTypes) {
        draw.fGeometry = InstanceProcessor::GetIndexRangeForOval(fInfo.fAntialiasMode,
                                                                 this->bounds());
    } else {
        draw.fGeometry = InstanceProcessor::GetIndexRangeForRRect(fInfo.fAntialiasMode);
    }

    if (!fParams.empty()) {
        SkASSERT(fInstancedRendering->fParams.count() < (int)kParamsIdx_InfoMask); // TODO: cleaner.
        this->getSingleInstance().fInfo |= fInstancedRendering->fParams.count();
        fInstancedRendering->fParams.push_back_n(fParams.count(), fParams.begin());
    }

    GrColor overrideColor;
    if (overrides.getOverrideColorIfSet(&overrideColor)) {
        SkASSERT(State::kRecordingDraws == fInstancedRendering->fState);
        this->getSingleInstance().fColor = overrideColor;
    }
    fInfo.fUsesLocalCoords = overrides.readsLocalCoords();
    fInfo.fCannotTweakAlphaForCoverage = !overrides.canTweakAlphaForCoverage();

    fInstancedRendering->fTrackedBatches.addToTail(this);
    fIsTracked = true;
}

bool InstancedRendering::Batch::onCombineIfPossible(GrOp* other, const GrCaps& caps) {
    Batch* that = static_cast<Batch*>(other);
    SkASSERT(fInstancedRendering == that->fInstancedRendering);
    SkASSERT(fTailDraw);
    SkASSERT(that->fTailDraw);

    if (!BatchInfo::CanCombine(fInfo, that->fInfo) ||
        !GrPipeline::CanCombine(*this->pipeline(), this->bounds(),
                                *that->pipeline(), that->bounds(), caps)) {
        return false;
    }

    BatchInfo combinedInfo = fInfo | that->fInfo;
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

    // Adopt the other batch's draws.
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

    if (fTrackedBatches.isEmpty()) {
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

void InstancedRendering::Batch::onDraw(GrOpFlushState* state, const SkRect& bounds) {
    SkASSERT(State::kFlushing == fInstancedRendering->fState);
    SkASSERT(state->gpu() == fInstancedRendering->gpu());

    state->gpu()->handleDirtyContext();
    if (GrXferBarrierType barrierType = this->pipeline()->xferBarrierType(*state->gpu()->caps())) {
        state->gpu()->xferBarrier(this->pipeline()->getRenderTarget(), barrierType);
    }

    InstanceProcessor instProc(fInfo, fInstancedRendering->fParamsBuffer.get());
    fInstancedRendering->onDraw(*this->pipeline(), instProc, this);
}

void InstancedRendering::endFlush() {
    // The caller is expected to delete all tracked batches (i.e. batches whose initBatchTracker
    // method has been called) before ending the flush.
    SkASSERT(fTrackedBatches.isEmpty());
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
