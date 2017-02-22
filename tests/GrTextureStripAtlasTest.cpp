/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#if SK_SUPPORT_GPU

#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrTextureStripAtlas.h"
#include "GrTypes.h"

// This tests that GrTextureStripAtlas flushes pending IO on the texture it acquires.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(GrTextureStripAtlasFlush, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    GrSurfaceDesc desc;
    desc.fWidth = 32;
    desc.fHeight = 32;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    sk_sp<GrSurfaceProxy> srcProxy;

    {
        SkAutoTMalloc<uint32_t> pixels(desc.fWidth * desc.fHeight);
        memset(pixels.get(), 0xFF, sizeof(uint32_t) * desc.fWidth * desc.fHeight);

        srcProxy = GrSurfaceProxy::MakeDeferred(*context->caps(), context->textureProvider(),
                                                desc, SkBudgeted::kYes,
                                                pixels.get(), 0);
    }

    // Add a pending read to the src texture, and then make it available for reuse.
    sk_sp<GrSurfaceContext> dstContext;
    GrSurface* srcSurface;

    {
        GrSurfaceDesc targetDesc = desc;
        targetDesc.fFlags = kRenderTarget_GrSurfaceFlag;

        // We can't use GrSurfaceProxy::Copy bc we may be changing the dst proxy type
        dstContext = context->contextPriv().makeDeferredSurfaceContext(targetDesc,
                                                                       SkBackingFit::kExact,
                                                                       SkBudgeted::kYes);
        REPORTER_ASSERT(reporter, dstContext);

        if (!dstContext->copy(srcProxy.get())) {
            return;
        }

        srcSurface = srcProxy->instantiate(context->textureProvider());
        // TODO: maybe add an assert here on srcSurface's ref state to ensure it is what we
        // expect.
        srcProxy.reset();
    }

    // Create an atlas with parameters that allow it to reuse the texture.
    GrTextureStripAtlas* atlas;

    {
        GrTextureStripAtlas::Desc atlasDesc;
        atlasDesc.fContext = context;
        atlasDesc.fConfig = desc.fConfig;
        atlasDesc.fWidth = desc.fWidth;
        atlasDesc.fHeight = desc.fHeight;
        atlasDesc.fRowHeight = desc.fHeight;
        atlas = GrTextureStripAtlas::GetAtlas(atlasDesc);
    }

    // Write to the atlas' texture.
    int lockedRow;

    {
        SkImageInfo info = SkImageInfo::MakeN32(desc.fWidth, desc.fHeight, kPremul_SkAlphaType);
        SkBitmap bitmap;
        bitmap.allocPixels(info);
        bitmap.eraseColor(SK_ColorBLACK);
        lockedRow = atlas->lockRow(bitmap);
    }

    // The atlas' use of its texture shouldn't change which pixels got copied to the target.
    {
        SkAutoTMalloc<uint8_t> actualPixels(sizeof(uint32_t) * desc.fWidth * desc.fHeight);

        SkImageInfo ii = SkImageInfo::Make(desc.fWidth, desc.fHeight,
                                           kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        bool success = dstContext->readPixels(ii, actualPixels.get(), 0, 0, 0);
        REPORTER_ASSERT(reporter, success);

        bool good = true;

        const uint8_t* bytes = actualPixels.get();
        for (size_t i = 0; i < sizeof(uint32_t) * desc.fWidth * desc.fHeight; ++i, ++bytes) {
            if (0xFF != *bytes) {
                good = false;
                break;
            }
        }

        REPORTER_ASSERT(reporter, good);
    }

    if (!context->caps()->preferVRAMUseOverFlushes()) {
        sk_sp<GrTextureProxy> proxy = atlas->asTextureProxyRef();
        GrTexture* tex = proxy->instantiate(context->textureProvider());

        // This is kindof dodgy since we released it!
        REPORTER_ASSERT(reporter, srcSurface == tex);
    }

    atlas->unlockRow(lockedRow);
}

#endif
