
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 
#include "GrAAHairLinePathRenderer.h"
#include "GrAAConvexPathRenderer.h"

void GrPathRenderer::AddPathRenderers(GrContext* ctx,
                                      GrPathRendererChain::UsageFlags flags,
                                      GrPathRendererChain* chain) {
    if (!(GrPathRendererChain::kNonAAOnly_UsageFlag & flags)) {

        if (GrPathRenderer* pr = GrAAHairLinePathRenderer::Create(ctx)) {
            chain->addPathRenderer(pr)->unref();
        }
        // Disabled for now.
        //chain->addPathRenderer(new GrAAConvexPathRenderer())->unref();
    }
}
