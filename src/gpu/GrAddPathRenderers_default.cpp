
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrStencilAndCoverPathRenderer.h"
#include "GrAAHairLinePathRenderer.h"
#include "GrAAConvexPathRenderer.h"
#if GR_STROKE_PATH_RENDERING
#include "../../experimental/StrokePathRenderer/GrStrokePathRenderer.h"
#endif
#if GR_ANDROID_PATH_RENDERING
#include "../../experimental/AndroidPathRenderer/GrAndroidPathRenderer.h"
#endif

void GrPathRenderer::AddPathRenderers(GrContext* ctx, GrPathRendererChain* chain) {
#if GR_STROKE_PATH_RENDERING
    chain->addPathRenderer(SkNEW(GrStrokePathRenderer))->unref();
#endif
#if GR_ANDROID_PATH_RENDERING
    chain->addPathRenderer(SkNEW(GrAndroidPathRenderer))->unref();
#endif
    if (GrPathRenderer* pr = GrStencilAndCoverPathRenderer::Create(ctx)) {
        chain->addPathRenderer(pr)->unref();
    }
    if (GrPathRenderer* pr = GrAAHairLinePathRenderer::Create(ctx)) {
        chain->addPathRenderer(pr)->unref();
    }
    chain->addPathRenderer(SkNEW(GrAAConvexPathRenderer))->unref();
}
