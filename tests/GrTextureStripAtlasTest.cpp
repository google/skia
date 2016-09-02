/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrGpu.h"
#include "GrTextureStripAtlas.h"
#include "GrTypes.h"
#include "SkGpuDevice.h"

// This tests that GrTextureStripAtlas flushes pending IO on the texture it acquires.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrTextureStripAtlasFlush, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrSurfaceDesc desc;
    desc.fWidth = 32;
    desc.fHeight = 32;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    GrTexture* texture = context->textureProvider()->createTexture(desc, SkBudgeted::kYes,
                                                                   nullptr, 0);

    GrSurfaceDesc targetDesc = desc;
    targetDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    GrTexture* target = context->textureProvider()->createTexture(targetDesc, SkBudgeted::kYes,
                                                                  nullptr, 0);

    SkAutoTMalloc<uint32_t> pixels(desc.fWidth * desc.fHeight);
    memset(pixels.get(), 0xFF, sizeof(uint32_t) * desc.fWidth * desc.fHeight);
    texture->writePixels(0, 0, desc.fWidth, desc.fHeight, kRGBA_8888_GrPixelConfig, pixels.get());

    // Add a pending read to the texture, and then make it available for reuse.
    context->copySurface(target, texture);
    texture->unref();

    // Create an atlas with parameters that allow it to reuse the texture.
    GrTextureStripAtlas::Desc atlasDesc;
    atlasDesc.fContext = context;
    atlasDesc.fConfig = desc.fConfig;
    atlasDesc.fWidth = desc.fWidth;
    atlasDesc.fHeight = desc.fHeight;
    atlasDesc.fRowHeight = 1;
    GrTextureStripAtlas* atlas = GrTextureStripAtlas::GetAtlas(atlasDesc);

    // Write to the atlas' texture.
    SkImageInfo info = SkImageInfo::MakeN32(desc.fWidth, desc.fHeight, kPremul_SkAlphaType);
    size_t rowBytes = desc.fWidth * GrBytesPerPixel(desc.fConfig);
    SkBitmap bitmap;
    bitmap.allocPixels(info, rowBytes);
    memset(bitmap.getPixels(), 1, rowBytes * desc.fHeight);
    int row = atlas->lockRow(bitmap);
    if (!context->caps()->preferVRAMUseOverFlushes())
        REPORTER_ASSERT(reporter, texture == atlas->getTexture());

    // The atlas' use of its texture shouldn't change which pixels got copied to the target.
    SkAutoTMalloc<uint32_t> actualPixels(desc.fWidth * desc.fHeight);
    bool success = target->readPixels(0, 0, desc.fWidth, desc.fHeight, kRGBA_8888_GrPixelConfig,
                                      actualPixels.get());
    REPORTER_ASSERT(reporter, success);
    REPORTER_ASSERT(reporter,
                    !memcmp(pixels.get(), actualPixels.get(),
                            sizeof(uint32_t) * desc.fWidth * desc.fHeight));
    target->unref();
    atlas->unlockRow(row);
}

#endif
