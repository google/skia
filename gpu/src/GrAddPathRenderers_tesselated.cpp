
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrTesselatedPathRenderer.h"


void GrPathRenderer::AddPathRenderers(GrContext*,
                                      GrPathRendererChain::UsageFlags flags,
                                      GrPathRendererChain* chain) {
    chain->addPathRenderer(new GrTesselatedPathRenderer())->unref();
}
