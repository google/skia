/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "src/gpu/v1/PathRendererChain.h"

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
#include "src/gpu/ops/AtlasPathRenderer.h"
#include "src/gpu/ops/DashLinePathRenderer.h"
#include "src/gpu/ops/DefaultPathRenderer.h"
#include "src/gpu/ops/SmallPathRenderer.h"
#include "src/gpu/ops/TessellationPathRenderer.h"
#include "src/gpu/ops/TriangulatingPathRenderer.h"

namespace skgpu::v1 {

PathRendererChain::PathRendererChain(GrRecordingContext* context, const Options& options) {
    const GrCaps& caps = *context->priv().caps();
    if (options.fGpuPathRenderers & GpuPathRenderers::kDashLine) {
        fChain.push_back(sk_make_sp<DashLinePathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAAConvex) {
        fChain.push_back(sk_make_sp<AAConvexPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAAHairline) {
        fChain.push_back(sk_make_sp<AAHairLinePathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAALinearizing) {
        fChain.push_back(sk_make_sp<AALinearizingConvexPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kAtlas) {
        if (auto atlasPathRenderer = AtlasPathRenderer::Make(context)) {
            fAtlasPathRenderer = atlasPathRenderer.get();
            context->priv().addOnFlushCallbackObject(atlasPathRenderer.get());
            fChain.push_back(std::move(atlasPathRenderer));
        }
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kSmall) {
        fChain.push_back(sk_make_sp<SmallPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kTriangulating) {
        fChain.push_back(sk_make_sp<TriangulatingPathRenderer>());
    }
    if (options.fGpuPathRenderers & GpuPathRenderers::kTessellation) {
        if (TessellationPathRenderer::IsSupported(caps)) {
            auto tess = sk_make_sp<TessellationPathRenderer>();
            fTessellationPathRenderer = tess.get();
            fChain.push_back(std::move(tess));
        }
    }

    // We always include the default path renderer (as well as SW), so we can draw any path
    fChain.push_back(sk_make_sp<DefaultPathRenderer>());
}

PathRenderer* PathRendererChain::getPathRenderer(const PathRenderer::CanDrawPathArgs& args,
                                                 DrawType drawType,
                                                 PathRenderer::StencilSupport* stencilSupport) {
    static_assert(PathRenderer::kNoSupport_StencilSupport <
                  PathRenderer::kStencilOnly_StencilSupport);
    static_assert(PathRenderer::kStencilOnly_StencilSupport <
                  PathRenderer::kNoRestriction_StencilSupport);
    PathRenderer::StencilSupport minStencilSupport;
    if (DrawType::kStencil == drawType) {
        minStencilSupport = PathRenderer::kStencilOnly_StencilSupport;
    } else if (DrawType::kStencilAndColor == drawType) {
        minStencilSupport = PathRenderer::kNoRestriction_StencilSupport;
    } else {
        minStencilSupport = PathRenderer::kNoSupport_StencilSupport;
    }
    if (minStencilSupport != PathRenderer::kNoSupport_StencilSupport) {
        // We don't support (and shouldn't need) stenciling of non-fill paths.
        if (!args.fShape->style().isSimpleFill()) {
            return nullptr;
        }
    }

    PathRenderer* bestPathRenderer = nullptr;
    for (const sk_sp<PathRenderer>& pr : fChain) {
        PathRenderer::StencilSupport support = PathRenderer::kNoSupport_StencilSupport;
        if (PathRenderer::kNoSupport_StencilSupport != minStencilSupport) {
            support = pr->getStencilSupport(*args.fShape);
            if (support < minStencilSupport) {
                continue;
            }
        }
        PathRenderer::CanDrawPath canDrawPath = pr->canDrawPath(args);
        if (PathRenderer::CanDrawPath::kNo == canDrawPath) {
            continue;
        }
        if (PathRenderer::CanDrawPath::kAsBackup == canDrawPath && bestPathRenderer) {
            continue;
        }
        if (stencilSupport) {
            *stencilSupport = support;
        }
        bestPathRenderer = pr.get();
        if (PathRenderer::CanDrawPath::kYes == canDrawPath) {
            break;
        }
    }
    return bestPathRenderer;
}

} // namespace skgpu::v1
