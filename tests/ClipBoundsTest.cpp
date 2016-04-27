/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
// This is a GR test
#if SK_SUPPORT_GPU
#include "GrClipMaskManager.h"
#include "GrContext.h"

// Ensure that the 'getConservativeBounds' calls are returning bounds clamped
// to the render target
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrClipBounds, reporter, ctxInfo) {
    static const int kXSize = 100;
    static const int kYSize = 100;

    const SkIRect intScreen = SkIRect::MakeWH(kXSize, kYSize);
    const SkRect screen = SkRect::Make(intScreen);

    SkRect clipRect(screen);
    clipRect.outset(10, 10);

    // create a clip stack that will (trivially) reduce to a single rect that
    // is larger than the screen
    SkClipStack stack;
    stack.clipDevRect(clipRect, SkRegion::kReplace_Op, false);

    bool isIntersectionOfRects = true;
    SkRect devStackBounds;

    stack.getConservativeBounds(0, 0, kXSize, kYSize,
                                &devStackBounds,
                                &isIntersectionOfRects);

    // make sure that the SkClipStack is behaving itself
    REPORTER_ASSERT(reporter, screen == devStackBounds);
    REPORTER_ASSERT(reporter, isIntersectionOfRects);

    // wrap the SkClipStack in a GrClip
    GrClip clipData;
    clipData.setClipStack(&stack);

    SkIRect devGrClipBound;
    clipData.getConservativeBounds(kXSize, kYSize,
                                   &devGrClipBound,
                                   &isIntersectionOfRects);

    // make sure that GrClip is behaving itself
    REPORTER_ASSERT(reporter, intScreen == devGrClipBound);
    REPORTER_ASSERT(reporter, isIntersectionOfRects);
}

#endif
