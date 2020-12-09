/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ops/GrDrawPathOp.h"

#include "include/gpu/GrRecordingContext.h"
#include "include/private/SkTemplates.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrProgramInfo.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/ops/GrSimpleMeshDrawOpHelper.h"

static constexpr GrUserStencilSettings kCoverPass{
        GrUserStencilSettings::StaticInit<
                0x0000,
                GrUserStencilTest::kNotEqual,
                0xffff,
                GrUserStencilOp::kZero,
                GrUserStencilOp::kKeep,
                0xffff>()
};

GrDrawPathOpBase::GrDrawPathOpBase(uint32_t classID, const SkMatrix& viewMatrix, GrPaint&& paint,
                                   GrPathRendering::FillType fill, GrAA aa)
        : INHERITED(classID)
        , fViewMatrix(viewMatrix)
        , fInputColor(paint.getColor4f())
        , fFillType(fill)
        , fDoAA(GrAA::kYes == aa)
        , fProcessorSet(std::move(paint)) {}

#if GR_TEST_UTILS
SkString GrDrawPathOp::onDumpInfo() const {
    return SkStringPrintf("PATH: 0x%p", fPath.get());
}
#endif

const GrProcessorSet::Analysis& GrDrawPathOpBase::doProcessorAnalysis(
        const GrCaps& caps, const GrAppliedClip* clip, bool hasMixedSampledCoverage,
        GrClampType clampType) {
    fAnalysis = fProcessorSet.finalize(
            fInputColor, GrProcessorAnalysisCoverage::kNone, clip, &kCoverPass,
            hasMixedSampledCoverage, caps, clampType, &fInputColor);
    return fAnalysis;
}

//////////////////////////////////////////////////////////////////////////////

void init_stencil_pass_settings(const GrOpFlushState& flushState,
                                GrPathRendering::FillType fillType, GrStencilSettings* stencil) {
    const GrAppliedClip* appliedClip = flushState.drawOpArgs().appliedClip();
    bool stencilClip = appliedClip && appliedClip->hasStencilClip();
    GrRenderTarget* rt = flushState.drawOpArgs().rtProxy()->peekRenderTarget();
    stencil->reset(GrPathRendering::GetStencilPassSettings(fillType), stencilClip,
                   rt->numStencilBits());
}

//////////////////////////////////////////////////////////////////////////////

GrOp::Owner GrDrawPathOp::Make(GrRecordingContext* context,
                               const SkMatrix& viewMatrix,
                               GrPaint&& paint,
                               GrAA aa,
                               sk_sp<const GrPath> path) {
    return GrOp::Make<GrDrawPathOp>(context, viewMatrix, std::move(paint), aa, std::move(path));
}

void GrDrawPathOp::onExecute(GrOpFlushState* flushState, const SkRect& chainBounds) {
    GrPipeline::InputFlags pipelineFlags = GrPipeline::InputFlags::kNone;
    if (this->doAA()) {
        pipelineFlags |= GrPipeline::InputFlags::kHWAntialias;
    }

    auto pipeline = GrSimpleMeshDrawOpHelper::CreatePipeline(flushState,
                                                             this->detachProcessorSet(),
                                                             pipelineFlags);

    sk_sp<GrPathProcessor> pathProc(GrPathProcessor::Create(this->color(), this->viewMatrix()));

    GrProgramInfo programInfo(flushState->writeView(),
                              pipeline,
                              &kCoverPass,
                              pathProc.get(),
                              GrPrimitiveType::kPath,
                              0,
                              flushState->renderPassBarriers(),
                              flushState->colorLoadOp());

    flushState->bindPipelineAndScissorClip(programInfo, this->bounds());
    flushState->bindTextures(programInfo.primProc(), nullptr, programInfo.pipeline());

    GrStencilSettings stencil;
    init_stencil_pass_settings(*flushState, this->fillType(), &stencil);
    flushState->gpu()->pathRendering()->drawPath(flushState->rtProxy()->peekRenderTarget(),
                                                 programInfo, stencil, fPath.get());
}

//////////////////////////////////////////////////////////////////////////////

inline void pre_translate_transform_values(const float* xforms,
                                           GrPathRendering::PathTransformType type, int count,
                                           SkScalar x, SkScalar y, float* dst) {
    if (0 == x && 0 == y) {
        memcpy(dst, xforms, count * GrPathRendering::PathTransformSize(type) * sizeof(float));
        return;
    }
    switch (type) {
        case GrPathRendering::kNone_PathTransformType:
            SK_ABORT("Cannot pre-translate kNone_PathTransformType.");
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
            SK_ABORT("Unknown transform type.");
            break;
    }
}
