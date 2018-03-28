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
#include "GrSoftwarePathRenderer.h"

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

GrPathRendererChain::GrPathRendererChain(GrContext* context, const Options& options)
        // AA hairline path renderer is very specialized - no other renderer can do this job well.
        : fAAHairLinePathRenderer(sk_make_sp<GrAAHairLinePathRenderer>())
        // We always create the default and SW path renderers so we can draw any path.
        , fDefaultPathRenderer(sk_make_sp<GrDefaultPathRenderer>())
        , fSoftwarePathRenderer(
                sk_make_sp<GrSoftwarePathRenderer>(context->contextPriv().proxyProvider(),
                                                   options.fAllowPathMaskCaching)) {
    const GrCaps& caps = *context->caps();
    if (options.fGpuPathRenderers & GpuPathRenderers::kDashLine) {
        fDashLinePathRenderer = sk_make_sp<GrDashLinePathRenderer>();
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kCoverageCounting) {
        bool clipCachablePaths = !options.fAllowPathMaskCaching;
        fCoverageCountingPathRenderer =
                GrCoverageCountingPathRenderer::CreateIfSupported(*context->caps(),
                                                                  clipCachablePaths);
        if (fCoverageCountingPathRenderer) {
            context->contextPriv().addOnFlushCallbackObject(fCoverageCountingPathRenderer.get());
        }
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAAConvex) {
        fAAConvexPathRenderer = sk_make_sp<GrAAConvexPathRenderer>();
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAALinearizing) {
        fAALinearizingConvexPathRenderer = sk_make_sp<GrAALinearizingConvexPathRenderer>();
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kSmall) {
        fSmallPathRenderer = sk_make_sp<GrSmallPathRenderer>();
        context->contextPriv().addOnFlushCallbackObject(fSmallPathRenderer.get());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kTessellating) {
        fTessellatingPathRenderer = sk_make_sp<GrTessellatingPathRenderer>();
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kStencilAndCover) {
        fStencilAndCoverPathRenderer.reset(
                GrStencilAndCoverPathRenderer::Create(context->contextPriv().resourceProvider(),
                                                      caps));
    }
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
    if ((options.fGpuPathRenderers & GpuPathRenderers::kMSAA) && caps.sampleShadingSupport()) {
        fMSAAPathRenderer = sk_make_sp<GrMSAAPathRenderer>();
    }
#endif
}

GrPathRendererChain::~GrPathRendererChain() {}

GrPathRenderer* GrPathRendererChain::getPathRenderer(const CanDrawPathArgs& args, DrawType drawType,
                                                     StencilSupport* outStencilSupport)
{
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

#define TRY(PR) \
    if (PR && PR->canDrawPath(minStencilSupport, args, outStencilSupport)) \
        return PR.get()

    if (args.fShape->style().isDashed()) {
        TRY(fDashLinePathRenderer);
    }
    if (GrAAType::kCoverage == args.fAAType) {
        TRY(fAAHairLinePathRenderer);
        TRY(fAAConvexPathRenderer);
        TRY(fAALinearizingConvexPathRenderer);
        TRY(fSmallPathRenderer);
        TRY(fTessellatingPathRenderer);
        if (fCoverageCountingPathRenderer &&
            fSoftwarePathRenderer->willDrawCached(minStencilSupport, args, outStencilSupport)) {
            return fSoftwarePathRenderer.get();
        }
        TRY(fCoverageCountingPathRenderer);
    } else {
        TRY(fStencilAndCoverPathRenderer);
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
        TRY(fMSAAPathRenderer);
#endif
        TRY(fTessellatingPathRenderer);
        TRY(fDefaultPathRenderer);
    }
    if (!args.fShape->style().applies()) {
        TRY(fSoftwarePathRenderer);
        SkDebugf("WARNING: No path renderer found to draw the path.\n");
    }

#undef TRY

    return nullptr;
}
