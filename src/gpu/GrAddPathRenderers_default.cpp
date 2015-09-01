
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrContext.h"
#include "GrGpu.h"

#include "batches/GrAAConvexPathRenderer.h"
#include "batches/GrAAHairLinePathRenderer.h"
#include "batches/GrAALinearizingConvexPathRenderer.h"
#include "batches/GrAADistanceFieldPathRenderer.h"
#include "batches/GrDashLinePathRenderer.h"
#include "batches/GrStencilAndCoverPathRenderer.h"
#include "batches/GrTessellatingPathRenderer.h"

void GrPathRenderer::AddPathRenderers(GrContext* ctx, GrPathRendererChain* chain) {
    chain->addPathRenderer(new GrDashLinePathRenderer)->unref();

    if (GrPathRenderer* pr = GrStencilAndCoverPathRenderer::Create(ctx->resourceProvider(),
                                                                   *ctx->caps())) {
        chain->addPathRenderer(pr)->unref();
    }
    chain->addPathRenderer(new GrTessellatingPathRenderer)->unref();
    if (GrPathRenderer* pr = GrAAHairLinePathRenderer::Create()) {
        chain->addPathRenderer(pr)->unref();
    }
    chain->addPathRenderer(new GrAAConvexPathRenderer)->unref();
    chain->addPathRenderer(new GrAALinearizingConvexPathRenderer)->unref();
    chain->addPathRenderer(new GrAADistanceFieldPathRenderer)->unref();
}
