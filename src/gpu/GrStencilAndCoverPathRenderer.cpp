
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

/*
 * For now paths only natively support winding and even odd fill types
 */
static GrPathRendering::FillType convert_skpath_filltype(SkPath::FillType fill) {
    switch (fill) {
        default:
            SkFAIL("Incomplete Switch\n");
        case SkPath::kWinding_FillType:
        case SkPath::kInverseWinding_FillType:
            return GrPathRendering::kWinding_FillType;
        case SkPath::kEvenOdd_FillType:
        case SkPath::kInverseEvenOdd_FillType:
            return GrPathRendering::kEvenOdd_FillType;
    }
}

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

bool GrStencilAndCoverPathRenderer::canDrawPath(const GrDrawTarget* target,
                                                const GrDrawState* drawState,
                                                const SkMatrix& viewMatrix,
                                                const SkPath& path,
                                                const SkStrokeRec& stroke,
                                                bool antiAlias) const {
    return !stroke.isHairlineStyle() &&
           !antiAlias && // doesn't do per-path AA, relies on the target having MSAA
           drawState->getRenderTarget()->getStencilBuffer() &&
           drawState->getStencil().isDisabled();
}

GrPathRenderer::StencilSupport
GrStencilAndCoverPathRenderer::onGetStencilSupport(const GrDrawTarget*,
                                                   const GrDrawState*,
                                                   const SkPath&,
                                                   const SkStrokeRec&) const {
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

void GrStencilAndCoverPathRenderer::onStencilPath(GrDrawTarget* target,
                                                  GrDrawState* drawState,
                                                  const SkMatrix& viewMatrix,
                                                  const SkPath& path,
                                                  const SkStrokeRec& stroke) {
    SkASSERT(!path.isInverseFillType());
    SkAutoTUnref<GrPathProcessor> pp(GrPathProcessor::Create(GrColor_WHITE, viewMatrix));
    SkAutoTUnref<GrPath> p(get_gr_path(fGpu, path, stroke));
    target->stencilPath(drawState, pp, p, convert_skpath_filltype(path.getFillType()));
}

bool GrStencilAndCoverPathRenderer::onDrawPath(GrDrawTarget* target,
                                               GrDrawState* drawState,
                                               GrColor color,
                                               const SkMatrix& viewMatrix,
                                               const SkPath& path,
                                               const SkStrokeRec& stroke,
                                               bool antiAlias) {
    SkASSERT(!antiAlias);
    SkASSERT(!stroke.isHairlineStyle());

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

        drawState->setStencil(kInvertedStencilPass);

        // fake inverse with a stencil and cover
        SkAutoTUnref<GrPathProcessor> pp(GrPathProcessor::Create(GrColor_WHITE, viewMatrix));
        target->stencilPath(drawState, pp, p, convert_skpath_filltype(path.getFillType()));

        SkMatrix invert = SkMatrix::I();
        SkRect bounds = SkRect::MakeLTRB(0, 0,
                                         SkIntToScalar(drawState->getRenderTarget()->width()),
                                         SkIntToScalar(drawState->getRenderTarget()->height()));
        SkMatrix vmi;
        // mapRect through persp matrix may not be correct
        if (!viewMatrix.hasPerspective() && viewMatrix.invert(&vmi)) {
            vmi.mapRect(&bounds);
            // theoretically could set bloat = 0, instead leave it because of matrix inversion
            // precision.
            SkScalar bloat = viewMatrix.getMaxScale() * SK_ScalarHalf;
            bounds.outset(bloat, bloat);
        } else {
            if (!viewMatrix.invert(&invert)) {
                return false;
            }
        }
        const SkMatrix& viewM = viewMatrix.hasPerspective() ? SkMatrix::I() : viewMatrix;
        target->drawRect(drawState, color, viewM, bounds, NULL, &invert);
    } else {
        GR_STATIC_CONST_SAME_STENCIL(kStencilPass,
            kZero_StencilOp,
            kZero_StencilOp,
            kNotEqual_StencilFunc,
            0xffff,
            0x0000,
            0xffff);

        drawState->setStencil(kStencilPass);
        SkAutoTUnref<GrPathProcessor> pp(GrPathProcessor::Create(color, viewMatrix));
        target->drawPath(drawState, pp, p, convert_skpath_filltype(path.getFillType()));
    }

    drawState->stencil()->setDisabled();
    return true;
}
