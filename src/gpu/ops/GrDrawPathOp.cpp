/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDrawPathOp.h"
#include "GrRenderTargetPriv.h"
#include "SkTemplates.h"
#include "GrRenderTargetContext.h"
#include "GrAppliedClip.h"

GrDrawPathOpBase::GrDrawPathOpBase(uint32_t classID, const SkMatrix& viewMatrix, GrPaint&& paint,
                 GrPathRendering::FillType fill)
        : INHERITED(classID), fViewMatrix(viewMatrix), fProcessorSet(std::move(paint)), fColor(paint.getColor()), fFillType(fill) {}

static void pre_translate_transform_values(const float* xforms,
                                           GrPathRendering::PathTransformType type, int count,
                                           SkScalar x, SkScalar y, float* dst);

SkString GrDrawPathOp::dumpInfo() const {
    SkString string;
    string.printf("PATH: 0x%p", fPath.get());
    //string.append(DumpPipelineInfo(*this->pipeline()));
    string.append(INHERITED::dumpInfo());
    return string;
}

void GrDrawPathOp::onExecute(GrOpFlushState* state, const SkRect& bounds, const GrAppliedClip* clip) {
    GrProgramDesc desc;

    SkAlignedSTStorage<1, GrPipeline> pipelineStorage;
    GrPipeline::CreateArgs args;
    args.fProcessors = &this->processors();
    args.fFlags = GrPipeline::kHWAntialias_Flag;
    args.fUserStencil = &GrPathRendering::GetStencilPassSettings(this->fillType());
    args.fAppliedClip = clip;
    args.fRenderTargetContext = this->renderTargetContext();
    args.fCaps = &state->caps();
    args.fDstTexture = this->dstTexture();

    GrPipelineOptimizations optimizations;
    const GrPipeline* pipeline = GrPipeline::CreateAt(pipelineStorage.get(), args, &optimizations);

    GrColor color = this->color();
    optimizations.getOverrideColorIfSet(&color);
    sk_sp<GrPathProcessor> pathProc(GrPathProcessor::Create(color, this->viewMatrix()));

    // TODO: Make GrPathRendering work with pipeline's stencil settings?
    GrStencilSettings stencil;
    stencil.reset(*args.fUserStencil, clip->hasStencilClip(), this->renderTargetContext()->accessRenderTarget()->numStencilSamples());
    state->gpu()->pathRendering()->drawPath(*pipeline, *pathProc,
                                            stencil, fPath.get());
}

SkString GrDrawPathRangeOp::dumpInfo() const {
    SkString string;
    string.printf("RANGE: 0x%p COUNTS: [", fPathRange.get());
    for (DrawList::Iter iter(fDraws); iter.get(); iter.next()) {
        string.appendf("%d, ", iter.get()->fInstanceData->count());
    }
    string.remove(string.size() - 2, 2);
    string.append("]");
    //string.append(DumpPipelineInfo(*this->pipeline()));
    string.append(INHERITED::dumpInfo());
    return string;
}

GrDrawPathRangeOp::GrDrawPathRangeOp(const SkMatrix& viewMatrix, SkScalar scale, SkScalar x,
                                     SkScalar y, GrPaint&& paint, GrPathRendering::FillType fill,
                                     GrPathRange* range, const InstanceData* instanceData,
                                     const SkRect& bounds)
        : INHERITED(ClassID(), viewMatrix, std::move(paint), fill)
        , fPathRange(range)
        , fTotalPathCount(instanceData->count())
        , fScale(scale) {
    fDraws.addToHead()->set(instanceData, x, y);
    this->setBounds(bounds, HasAABloat::kNo, IsZeroArea::kNo);
}

bool GrDrawPathRangeOp::onCombineIfPossible(GrOp* t, const GrCaps& caps) {
    return false;
}

void GrDrawPathRangeOp::onExecute(GrOpFlushState* state, const SkRect& bounds, const GrAppliedClip*) {
    const Draw& head = *fDraws.head();

    SkMatrix drawMatrix(this->viewMatrix());
    drawMatrix.preScale(fScale, fScale);
    drawMatrix.preTranslate(head.fX, head.fY);

    SkMatrix localMatrix;
    localMatrix.setScale(fScale, fScale);
    localMatrix.preTranslate(head.fX, head.fY);

    sk_sp<GrPathProcessor> pathProc(
            GrPathProcessor::Create(this->color(), drawMatrix, localMatrix));

    if (fDraws.count() == 1) {
//        const InstanceData& instances = *head.fInstanceData;
#if 0
        state->gpu()->pathRendering()->drawPaths(*this->pipeline(),
                                                 *pathProc,
                                                 this->stencilPassSettings(),
                                                 fPathRange.get(),
                                                 instances.indices(),
                                                 GrPathRange::kU16_PathIndexType,
                                                 instances.transformValues(),
                                                 instances.transformType(),
                                                 instances.count());
#endif
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

#if 0
        state->gpu()->pathRendering()->drawPaths(*this->pipeline(),
                                                 *pathProc,
                                                 this->stencilPassSettings(),
                                                 fPathRange.get(),
                                                 indexStorage,
                                                 GrPathRange::kU16_PathIndexType,
                                                 transformStorage,
                                                 this->transformType(),
                                                 fTotalPathCount);
#endif
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
