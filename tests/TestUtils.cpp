/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "TestUtils.h"

#if SK_SUPPORT_GPU

#include "GrSurfaceContext.h"
#include "GrSurfaceProxy.h"
#include "GrTextureProxy.h"

void test_read_pixels(skiatest::Reporter* reporter,
                      GrSurfaceContext* srcContext, uint32_t expectedPixelValues[],
                      const char* testName) {
    int pixelCnt = srcContext->width() * srcContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    memset(pixels.get(), 0, sizeof(uint32_t)*pixelCnt);

    SkImageInfo ii = SkImageInfo::Make(srcContext->width(), srcContext->height(),
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    bool read = srcContext->readPixels(ii, pixels.get(), 0, 0, 0);
    if (!read) {
        ERRORF(reporter, "%s: Error reading from texture.", testName);
    }

    for (int i = 0; i < pixelCnt; ++i) {
        if (pixels.get()[i] != expectedPixelValues[i]) {
            ERRORF(reporter, "%s: Error, pixel value %d should be 0x%08x, got 0x%08x.",
                   testName, i, expectedPixelValues[i], pixels.get()[i]);
            break;
        }
    }
}

void test_write_pixels(skiatest::Reporter* reporter,
                       GrSurfaceContext* dstContext, bool expectedToWork,
                       const char* testName) {
    int pixelCnt = dstContext->width() * dstContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    for (int y = 0; y < dstContext->width(); ++y) {
        for (int x = 0; x < dstContext->height(); ++x) {
            pixels.get()[y * dstContext->width() + x] =
                GrPremulColor(GrColorPackRGBA(x, y, x + y, 2*y));
        }
    }

    SkImageInfo ii = SkImageInfo::Make(dstContext->width(), dstContext->height(),
                                       kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    bool write = dstContext->writePixels(ii, pixels.get(), 0, 0, 0);
    if (!write) {
        if (expectedToWork) {
            ERRORF(reporter, "%s: Error writing to texture.", testName);
        }
        return;
    }

    if (write && !expectedToWork) {
        ERRORF(reporter, "%s: writePixels succeeded when it wasn't supposed to.", testName);
        return;
    }

    test_read_pixels(reporter, dstContext, pixels.get(), testName);
}

void test_copy_from_surface(skiatest::Reporter* reporter, GrContext* context,
                            GrSurfaceProxy* proxy, uint32_t expectedPixelValues[],
                            bool onlyTestRTConfig, const char* testName) {
    GrSurfaceDesc copyDstDesc;
    copyDstDesc.fConfig = kRGBA_8888_GrPixelConfig;
    copyDstDesc.fWidth = proxy->width();
    copyDstDesc.fHeight = proxy->height();

    for (auto flags : { kNone_GrSurfaceFlags, kRenderTarget_GrSurfaceFlag }) {
        if (kNone_GrSurfaceFlags == flags && onlyTestRTConfig) {
            continue;
        }

        copyDstDesc.fFlags = flags;
        copyDstDesc.fOrigin = (kNone_GrSurfaceFlags == flags) ? kTopLeft_GrSurfaceOrigin
                                                              : kBottomLeft_GrSurfaceOrigin;

        sk_sp<GrSurfaceContext> dstContext(GrSurfaceProxy::TestCopy(context, copyDstDesc, proxy));

        test_read_pixels(reporter, dstContext.get(), expectedPixelValues, testName);
    }
}

void test_copy_to_surface(skiatest::Reporter* reporter, GrResourceProvider* resourceProvider,
                          GrSurfaceContext* dstContext, const char* testName) {

    int pixelCnt = dstContext->width() * dstContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    for (int y = 0; y < dstContext->width(); ++y) {
        for (int x = 0; x < dstContext->height(); ++x) {
            pixels.get()[y * dstContext->width() + x] =
                GrPremulColor(GrColorPackRGBA(y, x, x * y, 2*y));
        }
    }

    GrSurfaceDesc copySrcDesc;
    copySrcDesc.fConfig = kRGBA_8888_GrPixelConfig;
    copySrcDesc.fWidth = dstContext->width();
    copySrcDesc.fHeight = dstContext->height();

    for (auto flags : { kNone_GrSurfaceFlags, kRenderTarget_GrSurfaceFlag }) {
        copySrcDesc.fFlags = flags;
        copySrcDesc.fOrigin = (kNone_GrSurfaceFlags == flags) ? kTopLeft_GrSurfaceOrigin
                                                              : kBottomLeft_GrSurfaceOrigin;

        sk_sp<GrTextureProxy> src(GrSurfaceProxy::MakeDeferred(resourceProvider,
                                                               copySrcDesc,
                                                               SkBudgeted::kYes, pixels.get(), 0));
        dstContext->copy(src.get());

        test_read_pixels(reporter, dstContext, pixels.get(), testName);
    }
}

#endif
