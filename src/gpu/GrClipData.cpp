/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClipData.h"

#include "GrSurface.h"
#include "SkRect.h"

///////////////////////////////////////////////////////////////////////////////

/**
 * getConservativeBounds returns the conservative bounding box of the clip
 * in device (as opposed to canvas) coordinates. If the bounding box is
 * the result of purely intersections of rects (with an initial replace)
 * isIntersectionOfRects will be set to true.
 */
void GrClipData::getConservativeBounds(int width, int height,
                                       SkIRect* devResult,
                                       bool* isIntersectionOfRects) const {
    SkRect devBounds;

    fClipStack->getConservativeBounds(-fOrigin.fX,
                                      -fOrigin.fY,
                                      width,
                                      height,
                                      &devBounds,
                                      isIntersectionOfRects);

    devBounds.roundOut(devResult);
}
