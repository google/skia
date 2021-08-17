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
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/ops/AAConvexPathRenderer.h"
#include "src/gpu/ops/AAHairLinePathRenderer.h"
#include "src/gpu/ops/AALinearizingConvexPathRenderer.h"
#include "src/gpu/ops/DashLinePathRenderer.h"
#include "src/gpu/ops/DefaultPathRenderer.h"
#include "src/gpu/ops/GrAtlasPathRenderer.h"
#include "src/gpu/ops/GrTriangulatingPathRenderer.h"
#include "src/gpu/ops/SmallPathRenderer.h"
#include "src/gpu/tessellate/GrTessellationPathRenderer.h"

GrPathRendererChain::GrPathRendererChain(GrRecordingContext* context, const Options& options) {
    const GrCaps& caps = *context->priv().caps();
    if (options.fGpuPathRenderers & GpuPathRenderers::kDashLine) {
        fChain.push_back(sk_make_sp<skgpu::v1::DashLinePathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAAConvex) {
        fChain.push_back(sk_make_sp<skgpu::v1::AAConvexPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAAHairline) {
        fChain.push_back(sk_make_sp<skgpu::v1::AAHairLinePathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAALinearizing) {
        fChain.push_back(sk_make_sp<skgpu::v1::AALinearizingConvexPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAtlas) {
        if (auto atlasPathRenderer = GrAtlasPathRenderer::Make(context)) {
            fAtlasPathRenderer = atlasPathRenderer.get();
            context->priv().addOnFlushCallbackObject(atlasPathRenderer.get());
            fChain.push_back(std::move(atlasPathRenderer));
        }
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kSmall) {
        fChain.push_back(sk_make_sp<skgpu::v1::SmallPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kTriangulating) {
        fChain.push_back(sk_make_sp<GrTriangulatingPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kTessellation) {
        if (GrTessellationPathRenderer::IsSupported(caps)) {
            auto tess = sk_make_sp<GrTessellationPathRenderer>();
            fTessellationPathRenderer = tess.get();
            fChain.push_back(std::move(tess));
        }
    }

    // We always include the default path renderer (as well as SW), so we can draw any path
    fChain.push_back(sk_make_sp<skgpu::v1::DefaultPathRenderer>());
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
