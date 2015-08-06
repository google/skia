
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrStencilAndCoverPathRenderer.h"
#include "GrAAHairLinePathRenderer.h"
#include "GrAAConvexPathRenderer.h"
#include "GrAALinearizingConvexPathRenderer.h"
#include "GrAADistanceFieldPathRenderer.h"
#include "GrContext.h"
#include "GrDashLinePathRenderer.h"
#include "GrGpu.h"
#include "GrTessellatingPathRenderer.h"

void GrPathRenderer::AddPathRenderers(GrContext* ctx, GrPathRendererChain* chain) {
    chain->addPathRenderer(SkNEW(GrDashLinePathRenderer))->unref();

    if (GrPathRenderer* pr = GrStencilAndCoverPathRenderer::Create(ctx->resourceProvider(),
                                                                   *ctx->caps())) {
        chain->addPathRenderer(pr)->unref();
    }
    chain->addPathRenderer(SkNEW(GrTessellatingPathRenderer))->unref();
    if (GrPathRenderer* pr = GrAAHairLinePathRenderer::Create()) {
        chain->addPathRenderer(pr)->unref();
    }
    chain->addPathRenderer(SkNEW(GrAAConvexPathRenderer))->unref();
    chain->addPathRenderer(SkNEW(GrAALinearizingConvexPathRenderer))->unref();
    chain->addPathRenderer(SkNEW(GrAADistanceFieldPathRenderer))->unref();
}
