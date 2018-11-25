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
#include "GrContextPriv.h"
#include "GrGpu.h"

#include "ccpr/GrCoverageCountingPathRenderer.h"

#include "ops/GrAAConvexPathRenderer.h"
#include "ops/GrAAHairLinePathRenderer.h"
#include "ops/GrAALinearizingConvexPathRenderer.h"
#include "ops/GrSmallPathRenderer.h"
#include "ops/GrDashLinePathRenderer.h"
#include "ops/GrDefaultPathRenderer.h"
#include "ops/GrMSAAPathRenderer.h"
#include "ops/GrStencilAndCoverPathRenderer.h"
#include "ops/GrTessellatingPathRenderer.h"

GrPathRendererChain::GrPathRendererChain(GrContext* context, const Options& options) {
    const GrCaps& caps = *context->caps();
    if (options.fGpuPathRenderers & GpuPathRenderers::kDashLine) {
        fChain.push_back(sk_make_sp<GrDashLinePathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kStencilAndCover) {
        sk_sp<GrPathRenderer> pr(
           GrStencilAndCoverPathRenderer::Create(context->contextPriv().resourceProvider(), caps));
        if (pr) {
            fChain.push_back(std::move(pr));
        }
    }
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
    if (options.fGpuPathRenderers & GpuPathRenderers::kMSAA) {
        if (caps.sampleShadingSupport()) {
            fChain.push_back(sk_make_sp<GrMSAAPathRenderer>());
        }
    }
#endif

    // AA hairline path renderer is very specialized - no other renderer can do this job well
    fChain.push_back(sk_make_sp<GrAAHairLinePathRenderer>());

    if (options.fGpuPathRenderers & GpuPathRenderers::kCoverageCounting) {
        bool drawCachablePaths = !options.fAllowPathMaskCaching;
        if (auto ccpr = GrCoverageCountingPathRenderer::CreateIfSupported(*context->caps(),
                                                                          drawCachablePaths)) {
            fCoverageCountingPathRenderer = ccpr.get();
            context->contextPriv().addOnFlushCallbackObject(fCoverageCountingPathRenderer);
            fChain.push_back(std::move(ccpr));
        }
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAAConvex) {
        fChain.push_back(sk_make_sp<GrAAConvexPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAALinearizing) {
        fChain.push_back(sk_make_sp<GrAALinearizingConvexPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kSmall) {
        auto spr = sk_make_sp<GrSmallPathRenderer>();
        context->contextPriv().addOnFlushCallbackObject(spr.get());
        fChain.push_back(std::move(spr));
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kTessellating) {
        fChain.push_back(sk_make_sp<GrTessellatingPathRenderer>());
    }

    // We always include the default path renderer (as well as SW), so we can draw any path
    fChain.push_back(sk_make_sp<GrDefaultPathRenderer>());
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

    GrPathRenderer* bestPathRenderer = nullptr;
    for (const sk_sp<GrPathRenderer>& pr : fChain) {
        GrPathRenderer::StencilSupport support = GrPathRenderer::kNoSupport_StencilSupport;
        if (GrPathRenderer::kNoSupport_StencilSupport != minStencilSupport) {
            support = pr->getStencilSupport(*args.fShape);
            if (support < minStencilSupport) {
                continue;
            }
        }
        GrPathRenderer::CanDrawPath canDrawPath = pr->canDrawPath(args);
        if (GrPathRenderer::CanDrawPath::kNo == canDrawPath) {
            continue;
        }
        if (GrPathRenderer::CanDrawPath::kAsBackup == canDrawPath && bestPathRenderer) {
            continue;
        }
        if (stencilSupport) {
            *stencilSupport = support;
        }
        bestPathRenderer = pr.get();
        if (GrPathRenderer::CanDrawPath::kYes == canDrawPath) {
            break;
        }
    }
    return bestPathRenderer;
}
