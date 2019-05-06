/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "src/core/SkClipOpPriv.h"
#include "src/core/SkClipStack.h"
#include "src/gpu/GrClipStackClip.h"
#include "tests/Test.h"

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
    stack.clipRect(clipRect, SkMatrix::I(), kReplace_SkClipOp, false);

    bool isIntersectionOfRects = true;
    SkRect devStackBounds;

    stack.getConservativeBounds(0, 0, kXSize, kYSize,
                                &devStackBounds,
                                &isIntersectionOfRects);

    // make sure that the SkClipStack is behaving itself
    REPORTER_ASSERT(reporter, screen == devStackBounds);
    REPORTER_ASSERT(reporter, isIntersectionOfRects);

    // wrap the SkClipStack in a GrClip
    GrClipStackClip clipData(&stack);

    SkIRect devGrClipBound;
    clipData.getConservativeBounds(kXSize, kYSize,
                                   &devGrClipBound,
                                   &isIntersectionOfRects);

    // make sure that GrClip is behaving itself
    REPORTER_ASSERT(reporter, intScreen == devGrClipBound);
    REPORTER_ASSERT(reporter, isIntersectionOfRects);
}
