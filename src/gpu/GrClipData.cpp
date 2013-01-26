
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClipData.h"
#include "GrSurface.h"
#include "GrRect.h"

///////////////////////////////////////////////////////////////////////////////

/**
 * getConservativeBounds returns the conservative bounding box of the clip
 * in device (as opposed to canvas) coordinates. If the bounding box is
 * the result of purely intersections of rects (with an initial replace)
 * isIntersectionOfRects will be set to true.
 */
void GrClipData::getConservativeBounds(const GrSurface* surface,
                                       GrIRect* devResult,
                                       bool* isIntersectionOfRects) const {
    GrRect devBounds;

    fClipStack->getConservativeBounds(-fOrigin.fX,
                                      -fOrigin.fY,
                                      surface->width(),
                                      surface->height(),
                                      &devBounds,
                                      isIntersectionOfRects);

    devBounds.roundOut(devResult);
}
