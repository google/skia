/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrClip.h"

#include "GrSurface.h"
#include "SkRect.h"

///////////////////////////////////////////////////////////////////////////////

/**
 * getConservativeBounds returns the conservative bounding box of the clip
 * in device (as opposed to canvas) coordinates. If the bounding box is
 * the result of purely intersections of rects (with an initial replace)
 * isIntersectionOfRects will be set to true.
 */
void GrClip::getConservativeBounds(int width, int height, SkIRect* devResult,
                                   bool* isIntersectionOfRects) const {
    switch (fClipType) {
        case kWideOpen_ClipType: {
            devResult->setLTRB(0, 0, width, height);
            if (isIntersectionOfRects) {
                *isIntersectionOfRects = true;
            }
        } break;
        case kIRect_ClipType: {
            *devResult = this->irect();
            if (isIntersectionOfRects) {
                *isIntersectionOfRects = true;
            }
        } break;
        case kClipStack_ClipType: {
            SkRect devBounds;
            this->clipStack()->getConservativeBounds(-this->origin().fX,
                                                     -this->origin().fY,
                                                     width,
                                                     height,
                                                     &devBounds,
                                                     isIntersectionOfRects);
            devBounds.roundOut(devResult);
        } break;

    }
}

const GrClip& GrClip::WideOpen() {
    static const GrClip clip;
    return clip;
}
