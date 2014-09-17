
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrStencilAndCoverPathRenderer.h"
#include "GrContext.h"
#include "GrDrawTargetCaps.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "SkStrokeRec.h"

GrPathRenderer* GrStencilAndCoverPathRenderer::Create(GrContext* context) {
    SkASSERT(context);
    SkASSERT(context->getGpu());
    if (context->getGpu()->caps()->pathRenderingSupport()) {
        return SkNEW_ARGS(GrStencilAndCoverPathRenderer, (context->getGpu()));
    } else {
        return NULL;
    }
}

GrStencilAndCoverPathRenderer::GrStencilAndCoverPathRenderer(GrGpu* gpu) {
    SkASSERT(gpu->caps()->pathRenderingSupport());
    fGpu = gpu;
    gpu->ref();
}

GrStencilAndCoverPathRenderer::~GrStencilAndCoverPathRenderer() {
    fGpu->unref();
}

bool GrStencilAndCoverPathRenderer::canDrawPath(const SkPath& path,
                                                const SkStrokeRec& stroke,
                                                const GrDrawTarget* target,
                                                bool antiAlias) const {
    return !stroke.isHairlineStyle() &&
           !antiAlias && // doesn't do per-path AA, relies on the target having MSAA
           target->getDrawState().getRenderTarget()->getStencilBuffer() &&
           target->getDrawState().getStencil().isDisabled();
}

GrPathRenderer::StencilSupport GrStencilAndCoverPathRenderer::onGetStencilSupport(
                                                        const SkPath&,
                                                        const SkStrokeRec& ,
                                                        const GrDrawTarget*) const {
    return GrPathRenderer::kStencilOnly_StencilSupport;
}

static GrPath* get_gr_path(GrGpu* gpu, const SkPath& skPath, const SkStrokeRec& stroke) {
    GrContext* ctx = gpu->getContext();
    GrResourceKey resourceKey = GrPath::ComputeKey(skPath, stroke);
    SkAutoTUnref<GrPath> path(static_cast<GrPath*>(ctx->findAndRefCachedResource(resourceKey)));
    if (NULL == path || !path->isEqualTo(skPath, stroke)) {
        path.reset(gpu->pathRendering()->createPath(skPath, stroke));
        ctx->addResourceToCache(resourceKey, path);
    }
    return path.detach();
}

void GrStencilAndCoverPathRenderer::onStencilPath(const SkPath& path,
                                                  const SkStrokeRec& stroke,
                                                  GrDrawTarget* target) {
    SkASSERT(!path.isInverseFillType());
    SkAutoTUnref<GrPath> p(get_gr_path(fGpu, path, stroke));
    target->stencilPath(p, path.getFillType());
}

bool GrStencilAndCoverPathRenderer::onDrawPath(const SkPath& path,
                                               const SkStrokeRec& stroke,
                                               GrDrawTarget* target,
                                               bool antiAlias) {
    SkASSERT(!antiAlias);
    SkASSERT(!stroke.isHairlineStyle());

    GrDrawState* drawState = target->drawState();
    SkASSERT(drawState->getStencil().isDisabled());

    SkAutoTUnref<GrPath> p(get_gr_path(fGpu, path, stroke));

    if (path.isInverseFillType()) {
        GR_STATIC_CONST_SAME_STENCIL(kInvertedStencilPass,
            kZero_StencilOp,
            kZero_StencilOp,
            // We know our rect will hit pixels outside the clip and the user bits will be 0
            // outside the clip. So we can't just fill where the user bits are 0. We also need to
            // check that the clip bit is set.
            kEqualIfInClip_StencilFunc,
            0xffff,
            0x0000,
            0xffff);

        *drawState->stencil() = kInvertedStencilPass;
    } else {
        GR_STATIC_CONST_SAME_STENCIL(kStencilPass,
            kZero_StencilOp,
            kZero_StencilOp,
            kNotEqual_StencilFunc,
            0xffff,
            0x0000,
            0xffff);

        *drawState->stencil() = kStencilPass;
    }

    target->drawPath(p, path.getFillType());

    target->drawState()->stencil()->setDisabled();
    return true;
}
