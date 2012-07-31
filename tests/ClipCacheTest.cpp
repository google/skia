/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
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
    desc.fConfig    = kSkia8888_PM_GrPixelConfig;
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

    GrAutoUnref au(texture);

    SkIRect intScreen = SkIRect::MakeWH(kXSize, kYSize);
    SkRect screen = SkRect::MakeWH(SkIntToScalar(kXSize), 
                                   SkIntToScalar(kYSize));
    SkRect clipRect(screen);
    clipRect.outset(10, 10);

    // create a clip stack that will (trivially) reduce to a single rect that
    // is larger than the screen
    SkClipStack stack;
    stack.clipDevRect(clipRect, SkRegion::kReplace_Op, false);

    bool isIntersectionOfRects = true;
    SkRect stackBounds;

    stack.getConservativeBounds(0, 0, kXSize, kYSize, 
                                &stackBounds, 
                                &isIntersectionOfRects);

    // make sure that the SkClipStack is behaving itself
    REPORTER_ASSERT(reporter, screen == stackBounds);
    REPORTER_ASSERT(reporter, isIntersectionOfRects);

    // convert the SkClipStack to a GrClip
    SkGrClipIterator iter;
    iter.reset(stack);

    GrClip clip;
    clip.setFromIterator(&iter, 0, 0, stackBounds);

    const GrRect& grBound = clip.getConservativeBounds();

    // make sure that GrClip is behaving itself
    REPORTER_ASSERT(reporter, clipRect == grBound);
    REPORTER_ASSERT(reporter, clip.isRect());

    // wrap the GrClip in a GrClipData
    GrClipData clipData;
    clipData.fClipStack = &clip;

    SkIRect intGrBound;
    clipData.getConservativeBounds(texture,
                                   &intGrBound,
                                   &isIntersectionOfRects);

    // make sure that GrClipData is behaving itself
    REPORTER_ASSERT(reporter, intScreen == intGrBound);
    REPORTER_ASSERT(reporter, isIntersectionOfRects);
}

////////////////////////////////////////////////////////////////////////////////
// verify that the top state of the stack matches the passed in state
static void check_state(skiatest::Reporter* reporter,
                        const GrClipMaskCache& cache,
                        const GrClip& clip,
                        GrTexture* mask,
                        const GrIRect& bound) {
    GrClip cacheClip;
    cache.getLastClip(&cacheClip);
    REPORTER_ASSERT(reporter, clip == cacheClip);

    REPORTER_ASSERT(reporter, mask == cache.getLastMask());

    GrIRect cacheBound;
    cache.getLastBound(&cacheBound);
    REPORTER_ASSERT(reporter, bound == cacheBound);
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

    GrClip emptyClip;
    emptyClip.setEmpty();

    GrIRect emptyBound;
    emptyBound.setEmpty();

    // check initial state
    check_state(reporter, cache, emptyClip, NULL, emptyBound);

    // set the current state
    GrIRect bound1;
    bound1.set(0, 0, 100, 100);

    GrClip clip1;
    clip1.setFromIRect(bound1);

    GrTextureDesc desc;
    desc.fFlags = kRenderTarget_GrTextureFlagBit;
    desc.fWidth = X_SIZE;
    desc.fHeight = Y_SIZE;
    desc.fConfig = kSkia8888_PM_GrPixelConfig;

    cache.acquireMask(clip1, desc, bound1);

    GrTexture* texture1 = cache.getLastMask();
    REPORTER_ASSERT(reporter, texture1);
    if (NULL == texture1) {
        return;
    }

    // check that the set took
    check_state(reporter, cache, clip1, texture1, bound1);
    REPORTER_ASSERT(reporter, 1 == texture1->getRefCnt());

    // push the state
    cache.push();

    // verify that the pushed state is initially empty
    check_state(reporter, cache, emptyClip, NULL, emptyBound);
    REPORTER_ASSERT(reporter, 1 == texture1->getRefCnt());

    // modify the new state
    GrIRect bound2;
    bound2.set(-10, -10, 10, 10);

    GrClip clip2;
    clip2.setEmpty();
    clip2.setFromIRect(bound2);

    cache.acquireMask(clip2, desc, bound2);

    GrTexture* texture2 = cache.getLastMask();
    REPORTER_ASSERT(reporter, texture2);
    if (NULL == texture2) {
        return;
    }

    // check that the changes took
    check_state(reporter, cache, clip2, texture2, bound2);
    REPORTER_ASSERT(reporter, 1 == texture1->getRefCnt());
    REPORTER_ASSERT(reporter, 1 == texture2->getRefCnt());

    // check to make sure canReuse works
    REPORTER_ASSERT(reporter, cache.canReuse(clip2, 10, 10));
    REPORTER_ASSERT(reporter, !cache.canReuse(clip1, 10, 10));

    // pop the state
    cache.pop();

    // verify that the old state is restored
    check_state(reporter, cache, clip1, texture1, bound1);
    REPORTER_ASSERT(reporter, 1 == texture1->getRefCnt());
    REPORTER_ASSERT(reporter, 1 == texture2->getRefCnt());

    // manually clear the state
    cache.reset();

    // verify it is now empty
    check_state(reporter, cache, emptyClip, NULL, emptyBound);
    REPORTER_ASSERT(reporter, 1 == texture1->getRefCnt());
    REPORTER_ASSERT(reporter, 1 == texture2->getRefCnt());

    // pop again - so there is no state
    cache.pop();

#if !defined(SK_DEBUG)
    // verify that the getters don't crash
    // only do in release since it generates asserts in debug
    check_state(reporter, cache, emptyClip, NULL, emptyBound);
#endif
    REPORTER_ASSERT(reporter, 1 == texture1->getRefCnt());
    REPORTER_ASSERT(reporter, 1 == texture2->getRefCnt());
}

////////////////////////////////////////////////////////////////////////////////
static void TestClipCache(skiatest::Reporter* reporter, GrContext* context) {

    test_cache(reporter, context);
    test_clip_bounds(reporter, context);
}

////////////////////////////////////////////////////////////////////////////////
#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("ClipCache", ClipCacheTestClass, TestClipCache)
