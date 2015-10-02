
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
#include "GrContextFactory.h"
#include "SkGpuDevice.h"

// Ensure that the 'getConservativeBounds' calls are returning bounds clamped
// to the render target
static void test_clip_bounds(skiatest::Reporter* reporter, GrContext* context) {

    static const int kXSize = 100;
    static const int kYSize = 100;

    GrSurfaceDesc desc;
    desc.fFlags     = kRenderTarget_GrSurfaceFlag;
    desc.fConfig    = kAlpha_8_GrPixelConfig;
    desc.fWidth     = kXSize;
    desc.fHeight    = kYSize;

    SkAutoTUnref<GrTexture> texture(
        context->textureProvider()->createTexture(desc, false, nullptr, 0));
    if (!texture) {
        return;
    }

    SkIRect intScreen = SkIRect::MakeWH(kXSize, kYSize);
    SkRect screen = SkRect::Make(intScreen);

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
    clipData.getConservativeBounds(texture,
                                   &devGrClipBound,
                                   &isIntersectionOfRects);

    // make sure that GrClip is behaving itself
    REPORTER_ASSERT(reporter, intScreen == devGrClipBound);
    REPORTER_ASSERT(reporter, isIntersectionOfRects);
}

DEF_GPUTEST(GrClipBounds, reporter, factory) {
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContextFactory::GLContextType glType = static_cast<GrContextFactory::GLContextType>(type);
        if (!GrContextFactory::IsRenderingGLContext(glType)) {
            continue;
        }
        GrContext* context = factory->get(glType);
        if (nullptr == context) {
            continue;
        }
        test_clip_bounds(reporter, context);
    }
}

#endif
