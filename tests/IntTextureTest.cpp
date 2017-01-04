/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrRenderTargetContext.h"
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
    GrSurfaceDesc desc;
    desc.fConfig = kRGBA_8888_sint_GrPixelConfig;
    desc.fWidth = kS;
    desc.fHeight = kS;
    sk_sp<GrTexture> texture;

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

    // Test that attempting to create a integer texture with multiple MIP level fails.
    GrMipLevel levels[2];
    levels[0].fPixels = testData.get();
    levels[0].fRowBytes = kS * sizeof(int32_t);
    levels[1].fPixels = testData.get();
    levels[1].fRowBytes = (kS / 2) * sizeof(int32_t);
    texture.reset(context->textureProvider()->createMipMappedTexture(desc, SkBudgeted::kYes, levels,
                                                                     2));
    REPORTER_ASSERT(reporter, !texture);

    // Test that we can create a integer texture.
    texture.reset(context->textureProvider()->createTexture(desc, SkBudgeted::kYes,
                                                            levels[0].fPixels,
                                                            levels[0].fRowBytes));
    REPORTER_ASSERT(reporter, texture);
    if (!texture) {
        return;
    }

    // Test that reading to a non-integer config fails.
    std::unique_ptr<int32_t[]> readData(new int32_t[kS * kS]);
    bool success = texture->readPixels(0, 0, kS, kS, kRGBA_8888_GrPixelConfig, readData.get());
    REPORTER_ASSERT(reporter, !success);
    std::unique_ptr<uint16_t[]> halfData(new uint16_t[4 * kS * kS]);
    success = texture->readPixels(0, 0, kS, kS, kRGBA_half_GrPixelConfig, halfData.get());
    REPORTER_ASSERT(reporter, !success);

    // Can read back as ints. (ES only requires being able to read back into 32bit ints which
    // we don't support. Right now this test is counting on GR_RGBA_INTEGER/GL_BYTE being the
    // implementation-dependent second format).
    sk_bzero(readData.get(), sizeof(int32_t) * kS * kS);
    success = texture->readPixels(0, 0, kS, kS, kRGBA_8888_sint_GrPixelConfig, readData.get());
    REPORTER_ASSERT(reporter, success);
    if (success) {
        check_pixels(reporter, kS, kS, testData.get(), readData.get(), "readPixels");
    }

    // readPixels should fail if we attempt to use the unpremul flag with an integer texture.
    success = texture->readPixels(0, 0, kS, kS, kRGBA_8888_sint_GrPixelConfig, readData.get(), 0,
                                  GrContext::kUnpremul_PixelOpsFlag);
    REPORTER_ASSERT(reporter, !success);

    // Test that copying from one integer texture to another succeeds.
    {
        sk_sp<GrSurfaceProxy> copy(GrSurfaceProxy::TestCopy(context, desc,
                                                            texture.get(), SkBudgeted::kYes));
        REPORTER_ASSERT(reporter, copy);
        if (!copy) {
            return;
        }

        GrSurface* copySurface = copy->instantiate(context->textureProvider());
        REPORTER_ASSERT(reporter, copySurface);
        if (!copySurface) {
            return;
        }

        sk_bzero(readData.get(), sizeof(int32_t) * kS * kS);
        success = copySurface->readPixels(0, 0, kS, kS,
                                          kRGBA_8888_sint_GrPixelConfig, readData.get());
        REPORTER_ASSERT(reporter, success);
        if (success) {
            check_pixels(reporter, kS, kS, testData.get(), readData.get(), "copyIntegerToInteger");
        }
    }


    // Test that copying to a non-integer (8888) texture fails.
    {
        GrSurfaceDesc nonIntDesc = desc;
        nonIntDesc.fConfig = kRGBA_8888_GrPixelConfig;

        sk_sp<GrSurfaceProxy> copy(GrSurfaceProxy::TestCopy(context, nonIntDesc,
                                                            texture.get(), SkBudgeted::kYes));
        REPORTER_ASSERT(reporter, !copy);
    }

    // Test that copying to a non-integer (RGBA_half) texture fails.
    if (context->caps()->isConfigTexturable(kRGBA_half_GrPixelConfig)) {
        GrSurfaceDesc nonIntDesc = desc;
        nonIntDesc.fConfig = kRGBA_half_GrPixelConfig;

        sk_sp<GrSurfaceProxy> copy(GrSurfaceProxy::TestCopy(context, nonIntDesc,
                                                            texture.get(), SkBudgeted::kYes));
        REPORTER_ASSERT(reporter, !copy);
    }

    // We overwrite the top left quarter of the texture with the bottom right quarter of the
    // original data.
    const void* bottomRightQuarter = testData.get() + kS / 2 * kS + kS / 2;
    size_t rowBytes = kS * sizeof(int32_t);

    // Can't write pixels from a non-int config.
    success = texture->writePixels(0, 0, kS/2, kS/2, kRGBA_8888_GrPixelConfig, bottomRightQuarter,
                                   rowBytes);
    REPORTER_ASSERT(reporter, !success);

    // Can't use unpremul flag.
    success = texture->writePixels(0, 0, kS/2, kS/2, kRGBA_8888_sint_GrPixelConfig,
                                   bottomRightQuarter, rowBytes,
                                   GrContext::kUnpremul_PixelOpsFlag);
    REPORTER_ASSERT(reporter, !success);

    success = texture->writePixels(0, 0, kS/2, kS/2, kRGBA_8888_sint_GrPixelConfig,
                                   bottomRightQuarter, rowBytes);
    REPORTER_ASSERT(reporter, success);
    if (!success) {
        return;
    }
    sk_bzero(readData.get(), sizeof(int32_t) * kS * kS);
    success = texture->readPixels(0, 0, kS, kS, kRGBA_8888_sint_GrPixelConfig, readData.get());
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
        dst += rowBytes;
        src += rowBytes;
    }
    check_pixels(reporter, kS, kS, overwrittenTestData.get(), readData.get(), "overwrite");

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
    texture->writePixels(0, 0, kS, kS, kRGBA_8888_sint_GrPixelConfig, testData.get());
    sk_sp<GrRenderTargetContext> rtContext = context->makeRenderTargetContext(
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
        SkMatrix m;
        m.setIDiv(kS, kS);
        sk_sp<GrFragmentProcessor> fp(GrSimpleTextureEffect::Make(texture.get(), nullptr, m,
                                                                  filter.fMode));
        REPORTER_ASSERT(reporter, fp);
        if (!fp) {
            return;
        }
        rtContext->clear(nullptr, 0xDDAABBCC, true);
        GrPaint paint;
        paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
        paint.addColorFragmentProcessor(fp);
        rtContext->drawPaint(GrNoClip(), paint, SkMatrix::I());
        SkImageInfo readInfo = SkImageInfo::Make(kS, kS, kRGBA_8888_SkColorType,
                                                 kPremul_SkAlphaType);
        rtContext->readPixels(readInfo, actualData.get(), 0, 0, 0);
        check_pixels(reporter, kS, kS, expectedData.get(), actualData.get(), filter.fName);
    }

    // No rendering to integer textures.
    GrSurfaceDesc intRTDesc = desc;
    intRTDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    texture.reset(context->textureProvider()->createTexture(intRTDesc, SkBudgeted::kYes));
    REPORTER_ASSERT(reporter, !texture);
}

#endif
