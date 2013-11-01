/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "Test.h"
// This is a GR test
#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "SkGpuDevice.h"
#include "../../src/gpu/GrClipMaskManager.h"

static const int X_SIZE = 12;
static const int Y_SIZE = 12;

////////////////////////////////////////////////////////////////////////////////
// note: this is unused
static GrTexture* createTexture(GrContext* context) {
    unsigned char textureData[X_SIZE][Y_SIZE][4];

    memset(textureData, 0, 4* X_SIZE * Y_SIZE);

    GrTextureDesc desc;

    // let Skia know we will be using this texture as a render target
    desc.fFlags     = kRenderTarget_GrTextureFlagBit;
    desc.fConfig    = kSkia8888_GrPixelConfig;
    desc.fWidth     = X_SIZE;
    desc.fHeight    = Y_SIZE;

    // We are initializing the texture with zeros here
    GrTexture* texture = context->createUncachedTexture(desc, textureData, 0);
    if (!texture) {
        return NULL;
    }

    return texture;
}

// Ensure that the 'getConservativeBounds' calls are returning bounds clamped
// to the render target
static void test_clip_bounds(skiatest::Reporter* reporter, GrContext* context) {

    static const int kXSize = 100;
    static const int kYSize = 100;

    GrTextureDesc desc;
    desc.fFlags     = kRenderTarget_GrTextureFlagBit;
    desc.fConfig    = kAlpha_8_GrPixelConfig;
    desc.fWidth     = kXSize;
    desc.fHeight    = kYSize;

    GrTexture* texture = context->createUncachedTexture(desc, NULL, 0);
    if (!texture) {
        return;
    }

    SkAutoUnref au(texture);

    SkIRect intScreen = SkIRect::MakeWH(kXSize, kYSize);
    SkRect screen;

    screen = SkRect::MakeWH(SkIntToScalar(kXSize),
                            SkIntToScalar(kYSize));

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

    // wrap the SkClipStack in a GrClipData
    GrClipData clipData;
    clipData.fClipStack = &stack;

    SkIRect devGrClipDataBound;
    clipData.getConservativeBounds(texture,
                                   &devGrClipDataBound,
                                   &isIntersectionOfRects);

    // make sure that GrClipData is behaving itself
    REPORTER_ASSERT(reporter, intScreen == devGrClipDataBound);
    REPORTER_ASSERT(reporter, isIntersectionOfRects);
}

////////////////////////////////////////////////////////////////////////////////
// verify that the top state of the stack matches the passed in state
static void check_state(skiatest::Reporter* reporter,
                        const GrClipMaskCache& cache,
                        const SkClipStack& clip,
                        GrTexture* mask,
                        const SkIRect& bound) {
    REPORTER_ASSERT(reporter, clip.getTopmostGenID() == cache.getLastClipGenID());

    REPORTER_ASSERT(reporter, mask == cache.getLastMask());

    SkIRect cacheBound;
    cache.getLastBound(&cacheBound);
    REPORTER_ASSERT(reporter, bound == cacheBound);
}

static void check_empty_state(skiatest::Reporter* reporter,
                              const GrClipMaskCache& cache) {
    REPORTER_ASSERT(reporter, SkClipStack::kInvalidGenID == cache.getLastClipGenID());
    REPORTER_ASSERT(reporter, NULL == cache.getLastMask());

    SkIRect emptyBound;
    emptyBound.setEmpty();

    SkIRect cacheBound;
    cache.getLastBound(&cacheBound);
    REPORTER_ASSERT(reporter, emptyBound == cacheBound);
}

////////////////////////////////////////////////////////////////////////////////
// basic test of the cache's base functionality:
//  push, pop, set, canReuse & getters
static void test_cache(skiatest::Reporter* reporter, GrContext* context) {

    if (false) { // avoid bit rot, suppress warning
        createTexture(context);
    }
    GrClipMaskCache cache;

    cache.setContext(context);

    // check initial state
    check_empty_state(reporter, cache);

    // set the current state
    SkIRect bound1;
    bound1.set(0, 0, 100, 100);

    SkClipStack clip1(bound1);

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit;
    desc.fWidth = X_SIZE;
    desc.fHeight = Y_SIZE;
    desc.fConfig = kSkia8888_GrPixelConfig;

    cache.acquireMask(clip1.getTopmostGenID(), desc, bound1);

    GrTexture* texture1 = cache.getLastMask();
    REPORTER_ASSERT(reporter, texture1);
    if (NULL == texture1) {
        return;
    }

    // check that the set took
    check_state(reporter, cache, clip1, texture1, bound1);
    REPORTER_ASSERT(reporter, texture1->getRefCnt());

    // push the state
    cache.push();

    // verify that the pushed state is initially empty
    check_empty_state(reporter, cache);
    REPORTER_ASSERT(reporter, texture1->getRefCnt());

    // modify the new state
    SkIRect bound2;
    bound2.set(-10, -10, 10, 10);

    SkClipStack clip2(bound2);

    cache.acquireMask(clip2.getTopmostGenID(), desc, bound2);

    GrTexture* texture2 = cache.getLastMask();
    REPORTER_ASSERT(reporter, texture2);
    if (NULL == texture2) {
        return;
    }

    // check that the changes took
    check_state(reporter, cache, clip2, texture2, bound2);
    REPORTER_ASSERT(reporter, texture1->getRefCnt());
    REPORTER_ASSERT(reporter, texture2->getRefCnt());

    // check to make sure canReuse works
    REPORTER_ASSERT(reporter, cache.canReuse(clip2.getTopmostGenID(), bound2));
    REPORTER_ASSERT(reporter, !cache.canReuse(clip1.getTopmostGenID(), bound1));

    // pop the state
    cache.pop();

    // verify that the old state is restored
    check_state(reporter, cache, clip1, texture1, bound1);
    REPORTER_ASSERT(reporter, texture1->getRefCnt());

    // manually clear the state
    cache.reset();

    // verify it is now empty
    check_empty_state(reporter, cache);

    // pop again - so there is no state
    cache.pop();

#if !defined(SK_DEBUG)
    // verify that the getters don't crash
    // only do in release since it generates asserts in debug
    check_empty_state(reporter, cache);
#endif
}

////////////////////////////////////////////////////////////////////////////////
static void TestClipCache(skiatest::Reporter* reporter, GrContextFactory* factory) {
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContextFactory::GLContextType glType = static_cast<GrContextFactory::GLContextType>(type);
        if (!GrContextFactory::IsRenderingGLContext(glType)) {
            continue;
        }
        GrContext* context = factory->get(glType);
        if (NULL == context) {
            continue;
        }

        test_cache(reporter, context);
        test_clip_bounds(reporter, context);
    }
}

////////////////////////////////////////////////////////////////////////////////
#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("ClipCache", ClipCacheTestClass, TestClipCache)

#endif
