/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrCoordTransform.h"
#include "GrResourceProvider.h"
#include "GrTextureProxy.h"

void GrCoordTransform::reset(GrResourceProvider* resourceProvider, const SkMatrix& m,
                             GrTextureProxy* proxy, bool normalize) {
    SkASSERT(proxy);
    SkASSERT(!fInProcessor);

    fMatrix = m;
    // MDB TODO: just GrCaps is needed for this method
    // MDB TODO: once all the coord transforms take a proxy just store it here and
    // instantiate later
    fTexture = proxy->instantiate(resourceProvider);
    fNormalize = normalize;
    fReverseY = kBottomLeft_GrSurfaceOrigin == proxy->origin();
}
