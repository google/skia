/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test.

#include "tests/Test.h"

#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "src/gpu/GrRenderTargetContext.h"
#include "src/gpu/GrTextureProxy.h"

static const int kSize = 64;

static std::unique_ptr<GrRenderTargetContext> get_rtc(GrContext* ctx) {
    return ctx->priv().makeDeferredRenderTargetContext(SkBackingFit::kExact, kSize, kSize,
                                                       GrColorType::kRGBA_8888, nullptr);
}

static void check_instantiation_status(skiatest::Reporter* reporter,
                                       GrRenderTargetContext* rtCtx,
                                       bool wrappedExpectation) {
    REPORTER_ASSERT(reporter, rtCtx->testingOnly_IsInstantiated() == wrappedExpectation);

    GrTextureProxy* tProxy = rtCtx->asTextureProxy();
    REPORTER_ASSERT(reporter, tProxy);

    REPORTER_ASSERT(reporter, tProxy->isInstantiated() == wrappedExpectation);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(RenderTargetContextTest, reporter, ctxInfo) {
    GrContext* ctx = ctxInfo.grContext();

    // Calling instantiate on a GrRenderTargetContext's textureProxy also instantiates the
    // GrRenderTargetContext
    {
        auto rtCtx = get_rtc(ctx);

        check_instantiation_status(reporter, rtCtx.get(), false);

        GrTextureProxy* tProxy = rtCtx->asTextureProxy();
        REPORTER_ASSERT(reporter, tProxy);

        REPORTER_ASSERT(reporter, tProxy->instantiate(ctx->priv().resourceProvider()));

        check_instantiation_status(reporter, rtCtx.get(), true);
    }

    // readPixels switches a deferred rtCtx to wrapped
    {
        auto rtCtx = get_rtc(ctx);

        check_instantiation_status(reporter, rtCtx.get(), false);

        SkImageInfo dstInfo = SkImageInfo::MakeN32Premul(kSize, kSize);
        SkAutoTMalloc<uint32_t> dstBuffer(kSize * kSize);
        static const size_t kRowBytes = sizeof(uint32_t) * kSize;

        bool result = rtCtx->readPixels(dstInfo, dstBuffer.get(), kRowBytes, {0, 0});
        REPORTER_ASSERT(reporter, result);

        check_instantiation_status(reporter, rtCtx.get(), true);
    }

    // TODO: in a future world we should be able to add a test that the majority of
    // GrRenderTargetContext calls do not force the instantiation of a deferred
    // GrRenderTargetContext
}
