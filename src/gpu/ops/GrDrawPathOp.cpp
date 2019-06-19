/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/GrRecordingContext.h"
#include "include/private/SkTemplates.h"
#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrMemoryPool.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/ops/GrDrawPathOp.h"

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

#ifdef SK_DEBUG
SkString GrDrawPathOp::dumpInfo() const {
    SkString string;
    string.printf("PATH: 0x%p", fPath.get());
    string.append(INHERITED::dumpInfo());
    return string;
}
#endif

GrPipeline::InitArgs GrDrawPathOpBase::pipelineInitArgs(const GrOpFlushState& state) {
    GrPipeline::InitArgs args;
    if (fDoAA) {
        args.fInputFlags |= GrPipeline::InputFlags::kHWAntialias;
    }
    args.fUserStencil = &kCoverPass;
    args.fCaps = &state.caps();
    args.fResourceProvider = state.resourceProvider();
    args.fDstProxy = state.drawOpArgs().fDstProxy;
    args.fOutputSwizzle = state.drawOpArgs().fOutputSwizzle;
    return args;
}

const GrProcessorSet::Analysis& GrDrawPathOpBase::doProcessorAnalysis(
        const GrCaps& caps, const GrAppliedClip* clip, GrFSAAType fsaaType,
        GrClampType clampType) {
    fAnalysis = fProcessorSet.finalize(
            fInputColor, GrProcessorAnalysisCoverage::kNone, clip, &kCoverPass, fsaaType, caps,
            clampType, &fInputColor);
    return fAnalysis;
}

//////////////////////////////////////////////////////////////////////////////

void init_stencil_pass_settings(const GrOpFlushState& flushState,
                                GrPathRendering::FillType fillType, GrStencilSettings* stencil) {
    const GrAppliedClip* appliedClip = flushState.drawOpArgs().fAppliedClip;
    bool stencilClip = appliedClip && appliedClip->hasStencilClip();
    stencil->reset(GrPathRendering::GetStencilPassSettings(fillType), stencilClip,
                   flushState.drawOpArgs().renderTarget()->renderTargetPriv().numStencilBits());
}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrDrawOp> GrDrawPathOp::Make(GrRecordingContext* context,
                                             const SkMatrix& viewMatrix,
                                             GrPaint&& paint,
                                             GrAA aa,
                                             GrPath* path) {
    GrOpMemoryPool* pool = context->priv().opMemoryPool();

    return pool->allocate<GrDrawPathOp>(viewMatrix, std::move(paint), aa, path);
}

void GrDrawPathOp::onExecute(GrOpFlushState* state, const SkRect& chainBounds) {
    GrAppliedClip appliedClip = state->detachAppliedClip();
    GrPipeline::FixedDynamicState fixedDynamicState(appliedClip.scissorState().rect());
    GrPipeline pipeline(this->pipelineInitArgs(*state), this->detachProcessors(),
                        std::move(appliedClip));
    sk_sp<GrPathProcessor> pathProc(GrPathProcessor::Create(this->color(), this->viewMatrix()));

    GrStencilSettings stencil;
    init_stencil_pass_settings(*state, this->fillType(), &stencil);
    state->gpu()->pathRendering()->drawPath(state->drawOpArgs().renderTarget(),
                                            state->drawOpArgs().origin(),
                                            *pathProc, pipeline, fixedDynamicState, stencil,
                                            fPath.get());
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
