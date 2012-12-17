
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrStencilAndCoverPathRenderer.h"
#include "GrAAHairLinePathRenderer.h"
#include "GrAAConvexPathRenderer.h"

void GrPathRenderer::AddPathRenderers(GrContext* ctx, GrPathRendererChain* chain) {
    if (GrPathRenderer* pr = GrStencilAndCoverPathRenderer::Create(ctx)) {
        chain->addPathRenderer(pr)->unref();
    }
    if (GrPathRenderer* pr = GrAAHairLinePathRenderer::Create(ctx)) {
        chain->addPathRenderer(pr)->unref();
    }
    chain->addPathRenderer(SkNEW(GrAAConvexPathRenderer))->unref();
}
