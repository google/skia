/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif
#include "SkImage.h"
#include "SkSurface.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

static void test_flatten(skiatest::Reporter* reporter, const SkImageInfo& info) {
    // just need a safe amount of storage, but ensure that it is 4-byte aligned.
    int32_t storage[(sizeof(SkImageInfo)*2) / sizeof(int32_t)];
    SkBinaryWriteBuffer wb(storage, sizeof(storage));
    info.flatten(wb);
    SkASSERT(wb.bytesWritten() < sizeof(storage));

    SkReadBuffer rb(storage, wb.bytesWritten());

    // pick a noisy byte pattern, so we ensure that unflatten sets all of our fields
    SkImageInfo info2 = SkImageInfo::Make(0xB8, 0xB8, (SkColorType) 0xB8, (SkAlphaType) 0xB8);

    info2.unflatten(rb);
    REPORTER_ASSERT(reporter, rb.offset() == wb.bytesWritten());

    // FIXME (msarett):
    // Support flatten/unflatten of SkColorSpace objects.
    REPORTER_ASSERT(reporter, info.makeColorSpace(nullptr) == info2.makeColorSpace(nullptr));
}

DEF_TEST(ImageInfo_flattening, reporter) {
    sk_sp<SkColorSpace> spaces[] = {
        nullptr,
        SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named),
        SkColorSpace::NewNamed(SkColorSpace::kAdobeRGB_Named),
    };

    for (int ct = 0; ct <= kLastEnum_SkColorType; ++ct) {
        for (int at = 0; at <= kLastEnum_SkAlphaType; ++at) {
            for (auto& cs : spaces) {
                SkImageInfo info = SkImageInfo::Make(100, 200,
                                                     static_cast<SkColorType>(ct),
                                                     static_cast<SkAlphaType>(at),
                                                     cs);
                test_flatten(reporter, info);
            }
        }
    }
}

static void check_isopaque(skiatest::Reporter* reporter, const sk_sp<SkSurface>& surface,
                           bool expectedOpaque) {
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    REPORTER_ASSERT(reporter, image->isOpaque() == expectedOpaque);
}

DEF_TEST(ImageIsOpaqueTest, reporter) {
    SkImageInfo infoTransparent = SkImageInfo::MakeN32Premul(5, 5);
    auto surfaceTransparent(SkSurface::MakeRaster(infoTransparent));
    check_isopaque(reporter, surfaceTransparent, false);

    SkImageInfo infoOpaque = SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType);
    auto surfaceOpaque(SkSurface::MakeRaster(infoOpaque));
    check_isopaque(reporter, surfaceOpaque, true);
}

#if SK_SUPPORT_GPU

DEF_GPUTEST_FOR_GL_RENDERING_CONTEXTS(ImageIsOpaqueTest_Gpu, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    SkImageInfo infoTransparent = SkImageInfo::MakeN32Premul(5, 5);
    auto surfaceTransparent(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo, infoTransparent));
    check_isopaque(reporter, surfaceTransparent, false);

    SkImageInfo infoOpaque = SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType);
    auto surfaceOpaque(SkSurface::MakeRenderTarget(context,SkBudgeted::kNo, infoOpaque));

    check_isopaque(reporter, surfaceOpaque, true);
}

#endif
