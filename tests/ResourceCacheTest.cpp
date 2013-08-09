/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

// This is a GPU test
#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "SkGpuDevice.h"

static const int gWidth = 640;
static const int gHeight = 480;

////////////////////////////////////////////////////////////////////////////////
static void test_cache(skiatest::Reporter* reporter,
                       GrContext* context,
                       SkCanvas* canvas) {
    const SkIRect size = SkIRect::MakeWH(gWidth, gHeight);

    SkBitmap src;
    src.setConfig(SkBitmap::kARGB_8888_Config, size.width(), size.height());
    src.allocPixels();
    src.eraseColor(SK_ColorBLACK);
    size_t srcSize = src.getSize();

    size_t initialCacheSize = context->getGpuTextureCacheBytes();

    int oldMaxNum;
    size_t oldMaxBytes;
    context->getTextureCacheLimits(&oldMaxNum, &oldMaxBytes);

    // Set the cache limits so we can fit 10 "src" images and the
    // max number of textures doesn't matter
    size_t maxCacheSize = initialCacheSize + 10*srcSize;
    context->setTextureCacheLimits(1000, maxCacheSize);

    SkBitmap readback;
    readback.setConfig(SkBitmap::kARGB_8888_Config, size.width(), size.height());
    readback.allocPixels();

    for (int i = 0; i < 100; ++i) {
        canvas->drawBitmap(src, 0, 0);
        canvas->readPixels(size, &readback);

        // "modify" the src texture
        src.notifyPixelsChanged();

        size_t curCacheSize = context->getGpuTextureCacheBytes();

        // we should never go over the size limit
        REPORTER_ASSERT(reporter, curCacheSize <= maxCacheSize);
    }

    context->setTextureCacheLimits(oldMaxNum, oldMaxBytes);
}

////////////////////////////////////////////////////////////////////////////////
static void TestResourceCache(skiatest::Reporter* reporter, GrContextFactory* factory) {
    for (int type = 0; type < GrContextFactory::kLastGLContextType; ++type) {
        GrContextFactory::GLContextType glType = static_cast<GrContextFactory::GLContextType>(type);
        if (!GrContextFactory::IsRenderingGLContext(glType)) {
            continue;
        }
        GrContext* context = factory->get(glType);
        if (NULL == context) {
            continue;
        }

        GrTextureDesc desc;
        desc.fConfig = kSkia8888_GrPixelConfig;
        desc.fFlags = kRenderTarget_GrTextureFlagBit;
        desc.fWidth = gWidth;
        desc.fHeight = gHeight;

        SkAutoTUnref<GrTexture> texture(context->createUncachedTexture(desc, NULL, 0));
        SkAutoTUnref<SkGpuDevice> device(SkNEW_ARGS(SkGpuDevice, (context, texture.get())));
        SkCanvas canvas(device.get());

        test_cache(reporter, context, &canvas);
    }
}

////////////////////////////////////////////////////////////////////////////////
#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("ResourceCache", ResourceCacheTestClass, TestResourceCache)

#endif
