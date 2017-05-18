/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "Test.h"

// MDB TODO: With the move of the discard calls to the RenderTargetContext, deferred RTCs are being
// instantiated early. This test can be re-enabled once discards do not force an instantiation
// (i.e., when renderTargetProxies carry the op IORefs)
#if 0

#if SK_SUPPORT_GPU
#include "GrTextureProxy.h"
#include "GrRenderTargetContext.h"

static const int kSize = 64;

static sk_sp<GrRenderTargetContext> get_rtc(GrContext* ctx) {
    return ctx->makeDeferredRenderTargetContext(SkBackingFit::kExact,
                                                kSize, kSize,
                                                kRGBA_8888_GrPixelConfig, nullptr);
}

static void check_is_wrapped_status(skiatest::Reporter* reporter,
                                    GrRenderTargetContext* rtCtx,
                                    bool wrappedExpectation) {
    REPORTER_ASSERT(reporter, rtCtx->isWrapped_ForTesting() == wrappedExpectation);

    GrTextureProxy* tProxy = rtCtx->asTextureProxy();
    REPORTER_ASSERT(reporter, tProxy);

    REPORTER_ASSERT(reporter, tProxy->isWrapped_ForTesting() == wrappedExpectation);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(RenderTargetContextTest, reporter, ctxInfo) {
    GrContext* ctx = ctxInfo.grContext();

    // Calling instantiate on a GrRenderTargetContext's textureProxy also instantiates the
    // GrRenderTargetContext
    {
        sk_sp<GrRenderTargetContext> rtCtx(get_rtc(ctx));

        check_is_wrapped_status(reporter, rtCtx.get(), false);

        GrTextureProxy* tProxy = rtCtx->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);

        GrTexture* tex = tProxy->instantiate(ctx->resourceProvider());
        REPORTER_ASSERT(reporter, tex);

        check_is_wrapped_status(reporter, rtCtx.get(), true);
    }

    // readPixels switches a deferred rtCtx to wrapped
    {
        sk_sp<GrRenderTargetContext> rtCtx(get_rtc(ctx));

        check_is_wrapped_status(reporter, rtCtx.get(), false);

        SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(kSize, kSize);
        SkAutoTMalloc<uint32_t> dstBuffer(kSize * kSize);
        static const size_t kRowBytes = sizeof(uint32_t) * kSize;

        bool result = rtCtx->readPixels(dstInfo, dstBuffer.get(), kRowBytes, 0, 0);
        REPORTER_ASSERT(reporter, result);

        check_is_wrapped_status(reporter, rtCtx.get(), true);
    }

    // TODO: in a future world we should be able to add a test that the majority of
    // GrRenderTargetContext calls do not force the instantiation of a deferred 
    // GrRenderTargetContext
}
#endif
#endif
