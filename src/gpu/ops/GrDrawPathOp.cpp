/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawPathOp.h"
#include "GrAppliedClip.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetPriv.h"
#include "SkTemplates.h"

GrDrawPathOpBase::GrDrawPathOpBase(uint32_t classID, const SkMatrix& viewMatrix, GrPaint&& paint,
                                   GrPathRendering::FillType fill, GrAAType aaType)
        : INHERITED(classID)
        , fViewMatrix(viewMatrix)
        , fInputColor(paint.getColor())
        , fProcessorSet(std::move(paint))
        , fFillType(fill)
        , fAAType(aaType) {
    SkASSERT(fAAType != GrAAType::kCoverage);
}

SkString GrDrawPathOp::dumpInfo() const {
    SkString string;
    string.printf("PATH: 0x%p", fPath.get());
    string.append(INHERITED::dumpInfo());
    return string;
}

void GrDrawPathOpBase::initPipeline(const GrOpFlushState& state, GrPipeline* pipeline) {
    static constexpr GrUserStencilSettings kCoverPass{
            GrUserStencilSettings::StaticInit<
                    0x0000,
                    GrUserStencilTest::kNotEqual,
                    0xffff,
                    GrUserStencilOp::kZero,
                    GrUserStencilOp::kKeep,
                    0xffff>()
    };
    GrPipeline::InitArgs args;
    args.fProcessors = &this->processors();
    args.fFlags = GrAATypeIsHW(fAAType) ? GrPipeline::kHWAntialias_Flag : 0;
    args.fUserStencil = &kCoverPass;
    args.fAppliedClip = state.drawOpArgs().fAppliedClip;
    args.fRenderTarget = state.drawOpArgs().fRenderTarget;
    args.fCaps = &state.caps();
    args.fDstTexture = state.drawOpArgs().fDstTexture;

    return pipeline->init(args);
}

//////////////////////////////////////////////////////////////////////////////

void init_stencil_pass_settings(const GrOpFlushState& flushState,
                                GrPathRendering::FillType fillType, GrStencilSettings* stencil) {
    const GrAppliedClip* appliedClip = flushState.drawOpArgs().fAppliedClip;
    bool stencilClip = appliedClip && appliedClip->hasStencilClip();
    stencil->reset(GrPathRendering::GetStencilPassSettings(fillType), stencilClip,
                   flushState.drawOpArgs().fRenderTarget->renderTargetPriv().numStencilBits());
}

//////////////////////////////////////////////////////////////////////////////

void GrDrawPathOp::onExecute(GrOpFlushState* state) {
    GrPipeline pipeline;
    this->initPipeline(*state, &pipeline);
    sk_sp<GrPathProcessor> pathProc(GrPathProcessor::Create(this->color(), this->viewMatrix()));

    GrStencilSettings stencil;
    init_stencil_pass_settings(*state, this->fillType(), &stencil);
    state->gpu()->pathRendering()->drawPath(pipeline, *pathProc, stencil, fPath.get());
}

//////////////////////////////////////////////////////////////////////////////

SkString GrDrawPathRangeOp::dumpInfo() const {
    SkString string;
    string.printf("RANGE: 0x%p COUNTS: [", fPathRange.get());
    for (DrawList::Iter iter(fDraws); iter.get(); iter.next()) {
        string.appendf("%d, ", iter.get()->fInstanceData->count());
    }
    string.remove(string.size() - 2, 2);
    string.append("]");
    string.append(INHERITED::dumpInfo());
    return string;
}

GrDrawPathRangeOp::GrDrawPathRangeOp(const SkMatrix& viewMatrix, SkScalar scale, SkScalar x,
                                     SkScalar y, GrPaint&& paint, GrPathRendering::FillType fill,
                                     GrAAType aaType, GrPathRange* range,
                                     const InstanceData* instanceData, const SkRect& bounds)
        : INHERITED(ClassID(), viewMatrix, std::move(paint), fill, aaType)
        , fPathRange(range)
        , fTotalPathCount(instanceData->count())
        , fScale(scale) {
    fDraws.addToHead()->set(instanceData, x, y);
    this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
}

static void pre_translate_transform_values(const float* xforms,
                                           GrPathRendering::PathTransformType type, int count,
                                           SkScalar x, SkScalar y, float* dst);

bool GrDrawPathRangeOp::onCombineIfPossible(GrOp* t, const GrCaps& caps) {
    GrDrawPathRangeOp* that = t->cast<GrDrawPathRangeOp>();
    if (this->fPathRange.get() != that->fPathRange.get() ||
        this->transformType() != that->transformType() || this->fScale != that->fScale ||
        this->color() != that->color() || !this->viewMatrix().cheapEqualTo(that->viewMatrix())) {
        return false;
    }
    if (this->processors() != that->processors()) {
        return false;
    }
    switch (fDraws.head()->fInstanceData->transformType()) {
        case GrPathRendering::kNone_PathTransformType:
            if (this->fDraws.head()->fX != that->fDraws.head()->fX ||
                this->fDraws.head()->fY != that->fDraws.head()->fY) {
                return false;
            }
            break;
        case GrPathRendering::kTranslateX_PathTransformType:
            if (this->fDraws.head()->fY != that->fDraws.head()->fY) {
                return false;
            }
            break;
        case GrPathRendering::kTranslateY_PathTransformType:
            if (this->fDraws.head()->fX != that->fDraws.head()->fX) {
                return false;
            }
            break;
        default:
            break;
    }
    // TODO: Check some other things here. (winding, opaque, pathProc color, vm, ...)
    // Try to combine this call with the previous DrawPaths. We do this by stenciling all the
    // paths together and then covering them in a single pass. This is not equivalent to two
    // separate draw calls, so we can only do it if there is no blending (no overlap would also
    // work). Note that it's also possible for overlapping paths to cancel each other's winding
    // numbers, and we only partially account for this by not allowing even/odd paths to be
    // combined. (Glyphs in the same font tend to wind the same direction so it works out OK.)

    if (GrPathRendering::kWinding_FillType != this->fillType() ||
        GrPathRendering::kWinding_FillType != that->fillType()) {
        return false;
    }
    if (!this->processorAnalysis().canCombineOverlappedStencilAndCover()) {
        return false;
    }
    fTotalPathCount += that->fTotalPathCount;
    while (Draw* head = that->fDraws.head()) {
        Draw* draw = fDraws.addToTail();
        draw->fInstanceData.reset(head->fInstanceData.release());
        draw->fX = head->fX;
        draw->fY = head->fY;
        that->fDraws.popHead();
    }
    this->joinBounds(*that);
    return true;
}

void GrDrawPathRangeOp::onExecute(GrOpFlushState* state) {
    const Draw& head = *fDraws.head();

    SkMatrix drawMatrix(this->viewMatrix());
    drawMatrix.preScale(fScale, fScale);
    drawMatrix.preTranslate(head.fX, head.fY);

    SkMatrix localMatrix;
    localMatrix.setScale(fScale, fScale);
    localMatrix.preTranslate(head.fX, head.fY);

    sk_sp<GrPathProcessor> pathProc(
            GrPathProcessor::Create(this->color(), drawMatrix, localMatrix));

    GrPipeline pipeline;
    this->initPipeline(*state, &pipeline);
    GrStencilSettings stencil;
    init_stencil_pass_settings(*state, this->fillType(), &stencil);
    if (fDraws.count() == 1) {
        const InstanceData& instances = *head.fInstanceData;
        state->gpu()->pathRendering()->drawPaths(pipeline,
                                                 *pathProc,
                                                 stencil,
                                                 fPathRange.get(),
                                                 instances.indices(),
                                                 GrPathRange::kU16_PathIndexType,
                                                 instances.transformValues(),
                                                 instances.transformType(),
                                                 instances.count());
    } else {
        int floatsPerTransform = GrPathRendering::PathTransformSize(this->transformType());
        SkAutoSTMalloc<4096, float> transformStorage(floatsPerTransform * fTotalPathCount);
        SkAutoSTMalloc<2048, uint16_t> indexStorage(fTotalPathCount);
        int idx = 0;
        for (DrawList::Iter iter(fDraws); iter.get(); iter.next()) {
            const Draw& draw = *iter.get();
            const InstanceData& instances = *draw.fInstanceData;
            memcpy(&indexStorage[idx], instances.indices(), instances.count() * sizeof(uint16_t));
            pre_translate_transform_values(instances.transformValues(), this->transformType(),
                                           instances.count(), draw.fX - head.fX, draw.fY - head.fY,
                                           &transformStorage[floatsPerTransform * idx]);
            idx += instances.count();

            // TODO: Support mismatched transform types if we start using more types other than 2D.
            SkASSERT(instances.transformType() == this->transformType());
        }
        SkASSERT(idx == fTotalPathCount);

        state->gpu()->pathRendering()->drawPaths(pipeline,
                                                 *pathProc,
                                                 stencil,
                                                 fPathRange.get(),
                                                 indexStorage,
                                                 GrPathRange::kU16_PathIndexType,
                                                 transformStorage,
                                                 this->transformType(),
                                                 fTotalPathCount);
    }
}

inline void pre_translate_transform_values(const float* xforms,
                                           GrPathRendering::PathTransformType type, int count,
                                           SkScalar x, SkScalar y, float* dst) {
    if (0 == x && 0 == y) {
        memcpy(dst, xforms, count * GrPathRendering::PathTransformSize(type) * sizeof(float));
        return;
    }
    switch (type) {
        case GrPathRendering::kNone_PathTransformType:
            SkFAIL("Cannot pre-translate kNone_PathTransformType.");
            break;
        case GrPathRendering::kTranslateX_PathTransformType:
            SkASSERT(0 == y);
            for (int i = 0; i < count; i++) {
                dst[i] = xforms[i] + x;
            }
            break;
        case GrPathRendering::kTranslateY_PathTransformType:
            SkASSERT(0 == x);
            for (int i = 0; i < count; i++) {
                dst[i] = xforms[i] + y;
            }
            break;
        case GrPathRendering::kTranslate_PathTransformType:
            for (int i = 0; i < 2 * count; i += 2) {
                dst[i] = xforms[i] + x;
                dst[i + 1] = xforms[i + 1] + y;
            }
            break;
        case GrPathRendering::kAffine_PathTransformType:
            for (int i = 0; i < 6 * count; i += 6) {
                dst[i] = xforms[i];
                dst[i + 1] = xforms[i + 1];
                dst[i + 2] = xforms[i] * x + xforms[i + 1] * y + xforms[i + 2];
                dst[i + 3] = xforms[i + 3];
                dst[i + 4] = xforms[i + 4];
                dst[i + 5] = xforms[i + 3] * x + xforms[i + 4] * y + xforms[i + 5];
            }
            break;
        default:
            SkFAIL("Unknown transform type.");
            break;
    }
}
