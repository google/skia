/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCommandBuilder.h"

#include "GrInOrderCommandBuilder.h"
#include "GrReorderCommandBuilder.h"

GrCommandBuilder* GrCommandBuilder::Create(GrGpu* gpu, bool reorder) {
    if (reorder) {
        return SkNEW_ARGS(GrReorderCommandBuilder, (gpu));
    } else {
        return SkNEW_ARGS(GrInOrderCommandBuilder, (gpu));
    }
}

GrTargetCommands::Cmd* GrCommandBuilder::recordClear(const SkIRect* rect,
                                                     GrColor color,
                                                     bool canIgnoreRect,
                                                     GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);

    SkIRect r;
    if (NULL == rect) {
        // We could do something smart and remove previous draws and clears to
        // the current render target. If we get that smart we have to make sure
        // those draws aren't read before this clear (render-to-texture).
        r.setLTRB(0, 0, renderTarget->width(), renderTarget->height());
        rect = &r;
    }
    Clear* clr = GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), Clear, (renderTarget));
    GrColorIsPMAssert(color);
    clr->fColor = color;
    clr->fRect = *rect;
    clr->fCanIgnoreRect = canIgnoreRect;
    return clr;
}

GrTargetCommands::Cmd* GrCommandBuilder::recordClearStencilClip(const SkIRect& rect,
                                                                bool insideClip,
                                                                GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);

    ClearStencilClip* clr = GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(),
                                                     ClearStencilClip,
                                                     (renderTarget));
    clr->fRect = rect;
    clr->fInsideClip = insideClip;
    return clr;
}

GrTargetCommands::Cmd* GrCommandBuilder::recordDiscard(GrRenderTarget* renderTarget) {
    SkASSERT(renderTarget);

    Clear* clr = GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), Clear, (renderTarget));
    clr->fColor = GrColor_ILLEGAL;
    return clr;
}

GrTargetCommands::Cmd* GrCommandBuilder::recordCopySurface(GrSurface* dst,
                                                           GrSurface* src,
                                                           const SkIRect& srcRect,
                                                           const SkIPoint& dstPoint) {
    CopySurface* cs = GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), CopySurface, (dst, src));
    cs->fSrcRect = srcRect;
    cs->fDstPoint = dstPoint;
    return cs;
}

GrTargetCommands::Cmd*
GrCommandBuilder::recordXferBarrierIfNecessary(const GrPipeline& pipeline,
                                               const GrDrawTargetCaps& caps) {
    const GrXferProcessor& xp = *pipeline.getXferProcessor();
    GrRenderTarget* rt = pipeline.getRenderTarget();

    GrXferBarrierType barrierType;
    if (!xp.willNeedXferBarrier(rt, caps, &barrierType)) {
        return NULL;
    }

    XferBarrier* xb = GrNEW_APPEND_TO_RECORDER(*this->cmdBuffer(), XferBarrier, (rt));
    xb->fBarrierType = barrierType;
    return xb;
}
