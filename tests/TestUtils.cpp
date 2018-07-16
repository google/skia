/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "TestUtils.h"

#include "GrProxyProvider.h"
#include "GrSurfaceContext.h"
#include "GrSurfaceContextPriv.h"
#include "GrSurfaceProxy.h"
#include "GrTextureProxy.h"
#include "ProxyUtils.h"

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
    copyDstDesc.fWidth = proxy->width();
    copyDstDesc.fHeight = proxy->height();
    copyDstDesc.fConfig = kRGBA_8888_GrPixelConfig;

    for (auto flags : { kNone_GrSurfaceFlags, kRenderTarget_GrSurfaceFlag }) {
        if (kNone_GrSurfaceFlags == flags && onlyTestRTConfig) {
            continue;
        }

        copyDstDesc.fFlags = flags;
        auto origin = (kNone_GrSurfaceFlags == flags) ? kTopLeft_GrSurfaceOrigin
                                                      : kBottomLeft_GrSurfaceOrigin;

        sk_sp<GrSurfaceContext> dstContext(
                GrSurfaceProxy::TestCopy(context, copyDstDesc, origin, proxy));

        test_read_pixels(reporter, dstContext.get(), expectedPixelValues, testName);
    }
}

void test_copy_to_surface(skiatest::Reporter* reporter, GrProxyProvider* proxyProvider,
                          GrSurfaceContext* dstContext, const char* testName) {

    int pixelCnt = dstContext->width() * dstContext->height();
    SkAutoTMalloc<uint32_t> pixels(pixelCnt);
    for (int y = 0; y < dstContext->width(); ++y) {
        for (int x = 0; x < dstContext->height(); ++x) {
            pixels.get()[y * dstContext->width() + x] =
                GrPremulColor(GrColorPackRGBA(y, x, x * y, 2*y));
        }
    }

    for (auto isRT : {false, true}) {
        for (auto origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
            auto src = sk_gpu_test::MakeTextureProxyFromData(
                    dstContext->surfPriv().getContext(), isRT, dstContext->width(),
                    dstContext->height(), GrColorType::kRGBA_8888, origin, pixels.get(), 0);
            dstContext->copy(src.get());
            test_read_pixels(reporter, dstContext, pixels.get(), testName);
        }
    }
}

void fill_pixel_data(int width, int height, GrColor* data) {
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            unsigned int red = (unsigned int)(256.f * (i / (float)width));
            unsigned int green = (unsigned int)(256.f * (j / (float)height));
            data[i + j * width] = GrColorPackRGBA(red - (red >> 8), green - (green >> 8),
                                                  0xff, 0xff);
        }
    }
}

bool does_full_buffer_contain_correct_color(GrColor* srcBuffer,
                                            GrColor* dstBuffer,
                                            int width,
                                            int height) {
    GrColor* srcPtr = srcBuffer;
    GrColor* dstPtr = dstBuffer;
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            if (srcPtr[i] != dstPtr[i]) {
                return false;
            }
        }
        srcPtr += width;
        dstPtr += width;
    }
    return true;
}
