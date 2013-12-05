
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrPathRenderer.h"

GrPathRenderer::GrPathRenderer() {
}

void GrPathRenderer::GetPathDevBounds(const SkPath& path,
                                      int devW, int devH,
                                      const SkMatrix& matrix,
                                      SkRect* bounds) {
    if (path.isInverseFillType()) {
        *bounds = SkRect::MakeWH(SkIntToScalar(devW), SkIntToScalar(devH));
        return;
    }
    *bounds = path.getBounds();
    matrix.mapRect(bounds);
}
