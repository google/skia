/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/GrPathRendererChain.h"

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/ccpr/GrCoverageCountingPathRenderer.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/GrAAConvexPathRenderer.h"
#include "src/gpu/ops/GrAAHairLinePathRenderer.h"
#include "src/gpu/ops/GrAALinearizingConvexPathRenderer.h"
#include "src/gpu/ops/GrDashLinePathRenderer.h"
#include "src/gpu/ops/GrDefaultPathRenderer.h"
#include "src/gpu/ops/GrSmallPathRenderer.h"
#include "src/gpu/ops/GrTriangulatingPathRenderer.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

GrPathRendererChain::GrPathRendererChain(GrRecordingContext* context, const Options& options) {
    const GrCaps& caps = *context->priv().caps();
    if (options.fGpuPathRenderers & GpuPathRenderers::kDashLine) {
        fChain.push_back(sk_make_sp<GrDashLinePathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAAConvex) {
        fChain.push_back(sk_make_sp<GrAAConvexPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kCoverageCounting) {
        fCoverageCountingPathRenderer = GrCoverageCountingPathRenderer::CreateIfSupported(context);
        if (fCoverageCountingPathRenderer) {
            // Don't add to the chain. This is only for clips.
            // TODO: Remove from here.
            context->priv().addOnFlushCallbackObject(fCoverageCountingPathRenderer.get());
        }
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAAHairline) {
        fChain.push_back(sk_make_sp<GrAAHairLinePathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAALinearizing) {
        fChain.push_back(sk_make_sp<GrAALinearizingConvexPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kSmall) {
        fChain.push_back(sk_make_sp<GrSmallPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kTriangulating) {
        fChain.push_back(sk_make_sp<GrTriangulatingPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kTessellation) {
        if (GrTessellationPathRenderer::IsSupported(caps)) {
            auto tess = sk_make_sp<GrTessellationPathRenderer>(context);
            fTessellationPathRenderer = tess.get();
            context->priv().addOnFlushCallbackObject(tess.get());
            fChain.push_back(std::move(tess));
        }
    }

    // We always include the default path renderer (as well as SW), so we can draw any path
    fChain.push_back(sk_make_sp<GrDefaultPathRenderer>());
}

GrPathRenderer* GrPathRendererChain::getPathRenderer(
        const GrPathRenderer::CanDrawPathArgs& args,
        DrawType drawType,
        GrPathRenderer::StencilSupport* stencilSupport) {
    static_assert(GrPathRenderer::kNoSupport_StencilSupport <
                  GrPathRenderer::kStencilOnly_StencilSupport);
    static_assert(GrPathRenderer::kStencilOnly_StencilSupport <
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
