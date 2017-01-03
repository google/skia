/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrPathRendererChain.h"

#include "GrCaps.h"
#include "GrShaderCaps.h"
#include "gl/GrGLCaps.h"
#include "GrContext.h"
#include "GrGpu.h"

#include "ops/GrAAConvexPathRenderer.h"
#include "ops/GrAADistanceFieldPathRenderer.h"
#include "ops/GrAAHairLinePathRenderer.h"
#include "ops/GrAALinearizingConvexPathRenderer.h"
#include "ops/GrDashLinePathRenderer.h"
#include "ops/GrDefaultPathRenderer.h"
#include "ops/GrMSAAPathRenderer.h"
#include "ops/GrPLSPathRenderer.h"
#include "ops/GrStencilAndCoverPathRenderer.h"
#include "ops/GrTessellatingPathRenderer.h"

GrPathRendererChain::GrPathRendererChain(GrContext* context, const Options& options) {
    if (!options.fDisableAllPathRenderers) {
        const GrCaps& caps = *context->caps();
        this->addPathRenderer(new GrDashLinePathRenderer)->unref();

        if (GrPathRenderer* pr = GrStencilAndCoverPathRenderer::Create(context->resourceProvider(),
                                                                       caps)) {
            this->addPathRenderer(pr)->unref();
        }
    #ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
        if (caps.sampleShadingSupport()) {
            this->addPathRenderer(new GrMSAAPathRenderer)->unref();
        }
    #endif
        this->addPathRenderer(new GrAAHairLinePathRenderer)->unref();
        this->addPathRenderer(new GrAAConvexPathRenderer)->unref();
        this->addPathRenderer(new GrAALinearizingConvexPathRenderer)->unref();
        if (caps.shaderCaps()->plsPathRenderingSupport()) {
            this->addPathRenderer(new GrPLSPathRenderer)->unref();
        }
        if (!options.fDisableDistanceFieldRenderer) {
            this->addPathRenderer(new GrAADistanceFieldPathRenderer)->unref();
        }
        this->addPathRenderer(new GrTessellatingPathRenderer)->unref();
        this->addPathRenderer(new GrDefaultPathRenderer(caps.twoSidedStencilSupport(),
                                                        caps.stencilWrapOpsSupport()))->unref();
    }
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

GrPathRenderer* GrPathRendererChain::getPathRenderer(
        const GrPathRenderer::CanDrawPathArgs& args,
        DrawType drawType,
        GrPathRenderer::StencilSupport* stencilSupport) {
    GR_STATIC_ASSERT(GrPathRenderer::kNoSupport_StencilSupport <
                     GrPathRenderer::kStencilOnly_StencilSupport);
    GR_STATIC_ASSERT(GrPathRenderer::kStencilOnly_StencilSupport <
                     GrPathRenderer::kNoRestriction_StencilSupport);
    GrPathRenderer::StencilSupport minStencilSupport;
    if (DrawType::kStencil == drawType) {
        minStencilSupport = GrPathRenderer::kStencilOnly_StencilSupport;
    } else if (DrawType::kStencilAndColor == drawType) {
        minStencilSupport = GrPathRenderer::kNoRestriction_StencilSupport;
    } else {
        minStencilSupport = GrPathRenderer::kNoSupport_StencilSupport;
    }
    if (minStencilSupport != GrPathRenderer::kNoSupport_StencilSupport) {
        // We don't support (and shouldn't need) stenciling of non-fill paths.
        if (!args.fShape->style().isSimpleFill()) {
            return nullptr;
        }
    }

    for (int i = 0; i < fChain.count(); ++i) {
        if (fChain[i]->canDrawPath(args)) {
            if (GrPathRenderer::kNoSupport_StencilSupport != minStencilSupport) {
                GrPathRenderer::StencilSupport support = fChain[i]->getStencilSupport(*args.fShape);
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
