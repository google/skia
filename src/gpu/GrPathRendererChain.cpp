
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrPathRendererChain.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrDefaultPathRenderer.h"
#include "GrGpu.h"

GrPathRendererChain::GrPathRendererChain(GrContext* context)
    : fInit(false)
    , fOwner(context) {
}

GrPathRendererChain::~GrPathRendererChain() {
    for (int i = 0; i < fChain.count(); ++i) {
        fChain[i]->unref();
    }
}

GrPathRenderer* GrPathRendererChain::addPathRenderer(GrPathRenderer* pr) {
    fChain.push_back() = pr;
    pr->ref();
    return pr;
}

GrPathRenderer* GrPathRendererChain::getPathRenderer(const GrDrawTarget* target,
                                                     const GrPipelineBuilder* pipelineBuilder,
                                                     const SkMatrix& viewMatrix,
                                                     const SkPath& path,
                                                     const GrStrokeInfo& stroke,
                                                     DrawType drawType,
                                                     StencilSupport* stencilSupport) {
    if (!fInit) {
        this->init();
    }
    bool antiAlias = (kColorAntiAlias_DrawType == drawType ||
                      kStencilAndColorAntiAlias_DrawType == drawType);

    GR_STATIC_ASSERT(GrPathRenderer::kNoSupport_StencilSupport <
                     GrPathRenderer::kStencilOnly_StencilSupport);
    GR_STATIC_ASSERT(GrPathRenderer::kStencilOnly_StencilSupport <
                     GrPathRenderer::kNoRestriction_StencilSupport);
    GrPathRenderer::StencilSupport minStencilSupport;
    if (kStencilOnly_DrawType == drawType) {
        minStencilSupport = GrPathRenderer::kStencilOnly_StencilSupport;
    } else if (kStencilAndColor_DrawType == drawType ||
               kStencilAndColorAntiAlias_DrawType == drawType) {
        minStencilSupport = GrPathRenderer::kNoRestriction_StencilSupport;
    } else {
        minStencilSupport = GrPathRenderer::kNoSupport_StencilSupport;
    }


    for (int i = 0; i < fChain.count(); ++i) {
        GrPathRenderer::CanDrawPathArgs args;
        args.fTarget = target;
        args.fPipelineBuilder = pipelineBuilder;
        args.fViewMatrix = &viewMatrix;
        args.fPath = &path;
        args.fStroke = &stroke;
        args.fAntiAlias = antiAlias;
        if (fChain[i]->canDrawPath(args)) {
            if (GrPathRenderer::kNoSupport_StencilSupport != minStencilSupport) {
                GrPathRenderer::StencilSupport support =
                    fChain[i]->getStencilSupport(target, pipelineBuilder, path, stroke);
                if (support < minStencilSupport) {
                    continue;
                } else if (stencilSupport) {
                    *stencilSupport = support;
                }
            }
            return fChain[i];
        }
    }
    return NULL;
}

void GrPathRendererChain::init() {
    SkASSERT(!fInit);
    const GrCaps* caps = fOwner->caps();
    bool twoSided = caps->twoSidedStencilSupport();
    bool wrapOp = caps->stencilWrapOpsSupport();
    GrPathRenderer::AddPathRenderers(fOwner, this);
    this->addPathRenderer(SkNEW_ARGS(GrDefaultPathRenderer,
                                     (twoSided, wrapOp)))->unref();
    fInit = true;
}
