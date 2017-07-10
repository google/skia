/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrClip.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"
#include "GrTexture.h"
#include "effects/GrSimpleTextureEffect.h"

template <typename I>
static SK_WHEN(std::is_integral<I>::value && 4 == sizeof(I), void)
check_pixels(skiatest::Reporter* reporter, int w, int h, const I exepctedData[],
             const I actualData[], const char* testName) {
    for (int j = 0; j < h; ++j) {
        for (int i = 0; i < w; ++i) {
            I expected = exepctedData[j * w + i];
            I actual = actualData[j * w + i];
            if (expected != actual) {
                ERRORF(reporter, "[%s] Expected 0x08%x, got 0x%08x at %d, %d.", testName, expected,
                       actual, i, j);
                return;
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(IntTexture, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    if (!context->caps()->isConfigTexturable(kRGBA_8888_sint_GrPixelConfig)) {
        return;
    }
    static const int kS = UINT8_MAX + 1;
    static const size_t kRowBytes = kS * sizeof(int32_t);

    GrSurfaceDesc desc;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fConfig = kRGBA_8888_sint_GrPixelConfig;
    desc.fWidth = kS;
    desc.fHeight = kS;

    std::unique_ptr<int32_t[]> testData(new int32_t[kS * kS]);
    for (int j = 0; j < kS; ++j) {
        for (int i = 0; i < kS; ++i) {
            uint32_t r = i - INT8_MIN;
            uint32_t g = j - INT8_MIN;
            uint32_t b = INT8_MAX - r;
            uint32_t a = INT8_MAX - g;
            testData.get()[j * kS + i] = (a << 24) | (b << 16) | (g << 8) | r;
        }
    }

    // Test that attempting to create a integer texture with multiple MIP levels fails.
    {
        GrMipLevel levels[2];
        levels[0].fPixels = testData.get();
        levels[0].fRowBytes = kRowBytes;
        levels[1].fPixels = testData.get();
        levels[1].fRowBytes = (kS / 2) * sizeof(int32_t);

        sk_sp<GrTextureProxy> temp(GrSurfaceProxy::MakeDeferredMipMap(context->resourceProvider(),
                                                                      desc,
                                                                      SkBudgeted::kYes,
                                                                      levels, 2));
        REPORTER_ASSERT(reporter, !temp);
    }

    // Test that we can create an integer texture.
    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                               desc, SkBudgeted::kYes,
                                                               testData.get(),
                                                               kRowBytes);
    REPORTER_ASSERT(reporter, proxy);
    if (!proxy) {
        return;
    }

    sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeWrappedSurfaceContext(
                                                                    std::move(proxy), nullptr);
    if (!sContext) {
        return;
    }

    std::unique_ptr<int32_t[]> readData(new int32_t[kS * kS]);
    // Test that reading to a non-integer config fails.
    {
        bool success = context->contextPriv().readSurfacePixels(sContext.get(),
                                                                0, 0, kS, kS,
                                                                kRGBA_8888_GrPixelConfig,
                                                                nullptr, readData.get());
        REPORTER_ASSERT(reporter, !success);
    }
    {
        std::unique_ptr<uint16_t[]> halfData(new uint16_t[4 * kS * kS]);
        bool success = context->contextPriv().readSurfacePixels(sContext.get(),
                                                                0, 0, kS, kS,
                                                                kRGBA_half_GrPixelConfig,
                                                                nullptr, halfData.get());
        REPORTER_ASSERT(reporter, !success);
    }
    {
        // Can read back as ints. (ES only requires being able to read back into 32bit ints which
        // we don't support. Right now this test is counting on GR_RGBA_INTEGER/GL_BYTE being the
        // implementation-dependent second format).
        sk_bzero(readData.get(), sizeof(int32_t) * kS * kS);
        bool success = context->contextPriv().readSurfacePixels(sContext.get(),
                                                                0, 0, kS, kS,
                                                                kRGBA_8888_sint_GrPixelConfig,
                                                                nullptr, readData.get());
        REPORTER_ASSERT(reporter, success);
        if (success) {
            check_pixels(reporter, kS, kS, testData.get(), readData.get(), "readPixels");
        }
    }
    {
        // readPixels should fail if we attempt to use the unpremul flag with an integer texture.
        bool success = context->contextPriv().readSurfacePixels(
                                                sContext.get(),
                                                0, 0, kS, kS,
                                                kRGBA_8888_sint_GrPixelConfig,
                                                nullptr, readData.get(), 0,
                                                GrContextPriv::kUnpremul_PixelOpsFlag);
        REPORTER_ASSERT(reporter, !success);
    }

    // Test that copying from one integer texture to another succeeds.
    {
        sk_sp<GrSurfaceContext> dstContext(GrSurfaceProxy::TestCopy(context, desc,
                                                                    sContext->asSurfaceProxy()));
        REPORTER_ASSERT(reporter, dstContext);
        if (!dstContext || !dstContext->asTextureProxy()) {
            return;
        }

        sk_bzero(readData.get(), sizeof(int32_t) * kS * kS);
        bool success = context->contextPriv().readSurfacePixels(dstContext.get(), 0, 0, kS, kS,
                                                                kRGBA_8888_sint_GrPixelConfig,
                                                                nullptr, readData.get());
        REPORTER_ASSERT(reporter, success);
        if (success) {
            check_pixels(reporter, kS, kS, testData.get(), readData.get(), "copyIntegerToInteger");
        }
    }


    // Test that copying to a non-integer (8888) texture fails.
    {
        GrSurfaceDesc nonIntDesc = desc;
        nonIntDesc.fConfig = kRGBA_8888_GrPixelConfig;

        sk_sp<GrSurfaceContext> dstContext(GrSurfaceProxy::TestCopy(context, nonIntDesc,
                                                                    sContext->asSurfaceProxy()));
        REPORTER_ASSERT(reporter, !dstContext);
    }

    // Test that copying to a non-integer (RGBA_half) texture fails.
    if (context->caps()->isConfigTexturable(kRGBA_half_GrPixelConfig)) {
        GrSurfaceDesc nonIntDesc = desc;
        nonIntDesc.fConfig = kRGBA_half_GrPixelConfig;

        sk_sp<GrSurfaceContext> dstContext(GrSurfaceProxy::TestCopy(context, nonIntDesc,
                                                                    sContext->asSurfaceProxy()));
        REPORTER_ASSERT(reporter, !dstContext);
    }

    // We overwrite the top left quarter of the texture with the bottom right quarter of the
    // original data.
    const void* bottomRightQuarter = testData.get() + kS / 2 * kS + kS / 2;

    {
        // Can't write pixels from a non-int config.
        bool success = context->contextPriv().writeSurfacePixels(sContext.get(),
                                                                 0, 0, kS/2, kS/2,
                                                                 kRGBA_8888_GrPixelConfig, nullptr,
                                                                 bottomRightQuarter, kRowBytes);
        REPORTER_ASSERT(reporter, !success);
    }
    {
        // Can't use unpremul flag.
        bool success = context->contextPriv().writeSurfacePixels(
                                            sContext.get(),
                                            0, 0, kS/2, kS/2,
                                            kRGBA_8888_sint_GrPixelConfig,
                                            nullptr,
                                            bottomRightQuarter, kRowBytes,
                                            GrContextPriv::kUnpremul_PixelOpsFlag);
        REPORTER_ASSERT(reporter, !success);
    }
    {
        bool success = context->contextPriv().writeSurfacePixels(sContext.get(),
                                                                 0, 0, kS/2, kS/2,
                                                                 kRGBA_8888_sint_GrPixelConfig,
                                                                 nullptr,
                                                                 bottomRightQuarter, kRowBytes);
        REPORTER_ASSERT(reporter, success);
        if (!success) {
            return;
        }

        sk_bzero(readData.get(), sizeof(int32_t) * kS * kS);
        success = context->contextPriv().readSurfacePixels(sContext.get(),
                                                           0, 0, kS, kS,
                                                           kRGBA_8888_sint_GrPixelConfig,
                                                           nullptr, readData.get(), 0);
        REPORTER_ASSERT(reporter, success);
        if (!success) {
            return;
        }
        std::unique_ptr<int32_t[]> overwrittenTestData(new int32_t[kS * kS]);
        memcpy(overwrittenTestData.get(), testData.get(), sizeof(int32_t) * kS * kS);
        char* dst = (char*)overwrittenTestData.get();
        char* src = (char*)(testData.get() + kS/2 * kS + kS/2);
        for (int i = 0; i < kS/2; ++i) {
            memcpy(dst, src, sizeof(int32_t) * kS/2);
            dst += kRowBytes;
            src += kRowBytes;
        }
        check_pixels(reporter, kS, kS, overwrittenTestData.get(), readData.get(), "overwrite");
    }

    // Test drawing from the integer texture to a fixed point texture. To avoid any premul issues
    // we init the int texture with 0s and 1s and make alpha always be 1. We expect that 1s turn
    // into 0xffs and zeros stay zero.
    std::unique_ptr<uint32_t[]> expectedData(new uint32_t[kS * kS]);
    std::unique_ptr<uint32_t[]> actualData(new uint32_t[kS * kS]);
    for (int i = 0; i < kS*kS; ++i) {
        int32_t a = 0x1;
        int32_t b = ((i & 0x1) ? 1 : 0);
        int32_t g = ((i & 0x1) ? 0 : 1);
        int32_t r = ((i & 0x2) ? 1 : 0);
        testData.get()[i] = (a << 24) | (b << 16) | (g << 8) | r;
        expectedData.get()[i] = ((0xFF * a) << 24) | ((0xFF * b) << 16) |
                                ((0xFF * g) << 8) | (0xFF * r);
    }
    context->contextPriv().writeSurfacePixels(sContext.get(),
                                              0, 0, kS, kS,
                                              kRGBA_8888_sint_GrPixelConfig, nullptr,
                                              testData.get(), 0);

    sk_sp<GrRenderTargetContext> rtContext = context->makeDeferredRenderTargetContext(
            SkBackingFit::kExact, kS, kS, kRGBA_8888_GrPixelConfig, nullptr);

    struct {
        GrSamplerParams::FilterMode fMode;
        const char* fName;
    } kNamedFilters[] ={
        { GrSamplerParams::kNone_FilterMode, "filter-none" },
        { GrSamplerParams::kBilerp_FilterMode, "filter-bilerp" },
        { GrSamplerParams::kMipMap_FilterMode, "filter-mipmap" }
    };

    for (auto filter : kNamedFilters) {
        sk_sp<GrFragmentProcessor> fp(GrSimpleTextureEffect::Make(sContext->asTextureProxyRef(),
                                                                  nullptr,
                                                                  SkMatrix::I(),
                                                                  filter.fMode));
        REPORTER_ASSERT(reporter, fp);
        if (!fp) {
            return;
        }
        rtContext->clear(nullptr, 0xDDAABBCC, true);
        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.addColorFragmentProcessor(fp);
        rtContext->drawPaint(GrNoClip(), std::move(paint), SkMatrix::I());
        SkImageInfo readInfo = SkImageInfo::Make(kS, kS, kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType);
        rtContext->readPixels(readInfo, actualData.get(), 0, 0, 0);
        check_pixels(reporter, kS, kS, expectedData.get(), actualData.get(), filter.fName);
    }

    {
        // No rendering to integer textures.
        GrSurfaceDesc intRTDesc = desc;
        intRTDesc.fFlags = kRenderTarget_GrSurfaceFlag;
        sk_sp<GrTexture> temp(context->resourceProvider()->createTexture(intRTDesc,
                                                                         SkBudgeted::kYes));
        REPORTER_ASSERT(reporter, !temp);
    }
}

#endif
