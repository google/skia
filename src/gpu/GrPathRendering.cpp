/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpu.h"
#include "GrPathRendering.h"
#include "GrRenderTarget.h"
#include "SkDescriptor.h"
#include "SkScalerContext.h"
#include "SkGlyph.h"
#include "SkMatrix.h"
#include "SkTypeface.h"

const GrUserStencilSettings& GrPathRendering::GetStencilPassSettings(FillType fill) {
    switch (fill) {
        default:
            SK_ABORT("Unexpected path fill.");
        case GrPathRendering::kWinding_FillType: {
            constexpr static GrUserStencilSettings kWindingStencilPass(
                GrUserStencilSettings::StaticInit<
                    0xffff,
                    GrUserStencilTest::kAlwaysIfInClip,
                    0xffff,
                    GrUserStencilOp::kIncWrap,
                    GrUserStencilOp::kIncWrap,
                    0xffff>()
            );
            return kWindingStencilPass;
        }
        case GrPathRendering::kEvenOdd_FillType: {
            constexpr static GrUserStencilSettings kEvenOddStencilPass(
                GrUserStencilSettings::StaticInit<
                    0xffff,
                    GrUserStencilTest::kAlwaysIfInClip,
                    0xffff,
                    GrUserStencilOp::kInvert,
                    GrUserStencilOp::kInvert,
                    0xffff>()
            );
            return kEvenOddStencilPass;
        }
    }
}

void GrPathRendering::stencilPath(const StencilPathArgs& args, const GrPath* path) {
    fGpu->handleDirtyContext();
    this->onStencilPath(args, path);
}

void GrPathRendering::drawPath(GrRenderTarget* renderTarget, GrSurfaceOrigin origin,
                               const GrPrimitiveProcessor& primProc,
                               const GrPipeline& pipeline,
                               const GrPipeline::FixedDynamicState& fixedDynamicState,
                               // Cover pass settings in pipeline.
                               const GrStencilSettings& stencilPassSettings,
                               const GrPath* path) {
    fGpu->handleDirtyContext();
    if (GrXferBarrierType barrierType = pipeline.xferBarrierType(renderTarget->asTexture(),
                                                                 *fGpu->caps())) {
        fGpu->xferBarrier(renderTarget, barrierType);
    }
    this->onDrawPath(renderTarget, origin, primProc, pipeline, fixedDynamicState,
                     stencilPassSettings, path);
}
