
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrPathRendererChain.h"

#include "GrCaps.h"
#include "GrContext.h"
#include "GrGpu.h"

#include "batches/GrAAConvexPathRenderer.h"
#include "batches/GrAADistanceFieldPathRenderer.h"
#include "batches/GrAAHairLinePathRenderer.h"
#include "batches/GrAALinearizingConvexPathRenderer.h"
#include "batches/GrDashLinePathRenderer.h"
#include "batches/GrDefaultPathRenderer.h"
#include "batches/GrStencilAndCoverPathRenderer.h"
#include "batches/GrTessellatingPathRenderer.h"

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

GrPathRenderer* GrPathRendererChain::getPathRenderer(const GrShaderCaps* shaderCaps,
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
        args.fShaderCaps = shaderCaps;
        args.fPipelineBuilder = pipelineBuilder;
        args.fViewMatrix = &viewMatrix;
        args.fPath = &path;
        args.fStroke = &stroke;
        args.fAntiAlias = antiAlias;
        if (fChain[i]->canDrawPath(args)) {
            if (GrPathRenderer::kNoSupport_StencilSupport != minStencilSupport) {
                GrPathRenderer::StencilSupport support =
                                                       fChain[i]->getStencilSupport(path, stroke);
                if (support < minStencilSupport) {
                    continue;
                } else if (stencilSupport) {
                    *stencilSupport = support;
                }
            }
            return fChain[i];
        }
    }
    return nullptr;
}

void GrPathRendererChain::init() {
    SkASSERT(!fInit);
    const GrCaps& caps = *fOwner->caps();
    this->addPathRenderer(new GrDashLinePathRenderer)->unref();

    if (GrPathRenderer* pr = GrStencilAndCoverPathRenderer::Create(fOwner->resourceProvider(),
                                                                   caps)) {
        this->addPathRenderer(pr)->unref();
    }
    this->addPathRenderer(new GrTessellatingPathRenderer)->unref();
    this->addPathRenderer(new GrAAHairLinePathRenderer)->unref();
    this->addPathRenderer(new GrAAConvexPathRenderer)->unref();
    this->addPathRenderer(new GrAALinearizingConvexPathRenderer)->unref();
    this->addPathRenderer(new GrAADistanceFieldPathRenderer)->unref();
    this->addPathRenderer(new GrDefaultPathRenderer(caps.twoSidedStencilSupport(),
                                                    caps.stencilWrapOpsSupport()))->unref();
    fInit = true;
}
