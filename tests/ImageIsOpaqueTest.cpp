/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#endif
#include "SkImage.h"
#include "SkSurface.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"

static void test_flatten(skiatest::Reporter* reporter, const SkImageInfo& info) {
    // just need a safe amount of storage, but ensure that it is 4-byte aligned.
    int32_t storage[(sizeof(SkImageInfo)*2) / sizeof(int32_t)];
    SkWriteBuffer wb(storage, sizeof(storage));
    info.flatten(wb);
    SkASSERT(wb.bytesWritten() < sizeof(storage));

    SkReadBuffer rb(storage, wb.bytesWritten());
    SkImageInfo info2;

    // pick a noisy byte pattern, so we ensure that unflatten sets all of our fields
    memset(&info2, 0xB8, sizeof(info2));

    info2.unflatten(rb);
    REPORTER_ASSERT(reporter, rb.offset() == wb.bytesWritten());
    REPORTER_ASSERT(reporter, info == info2);
}

DEF_TEST(ImageInfo_flattening, reporter) {
    for (int ct = 0; ct <= kLastEnum_SkColorType; ++ct) {
        for (int at = 0; at <= kLastEnum_SkAlphaType; ++at) {
            for (int pt = 0; pt <= kLastEnum_SkColorProfileType; ++pt) {
                SkImageInfo info = SkImageInfo::Make(100, 200,
                                                     static_cast<SkColorType>(ct),
                                                     static_cast<SkAlphaType>(at),
                                                     static_cast<SkColorProfileType>(pt));
                test_flatten(reporter, info);
            }
        }
    }
}

static void check_isopaque(skiatest::Reporter* reporter, SkSurface* surface, bool expectedOpaque) {
    SkAutoTUnref<SkImage> image(surface->newImageSnapshot());
    REPORTER_ASSERT(reporter, image->isOpaque() == expectedOpaque);
}

DEF_TEST(ImageIsOpaqueTest, reporter) {
    SkImageInfo infoTransparent = SkImageInfo::MakeN32Premul(5, 5);
    SkAutoTUnref<SkSurface> surfaceTransparent(SkSurface::NewRaster(infoTransparent));
    check_isopaque(reporter, surfaceTransparent, false);

    SkImageInfo infoOpaque = SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType);
    SkAutoTUnref<SkSurface> surfaceOpaque(SkSurface::NewRaster(infoOpaque));
    check_isopaque(reporter, surfaceOpaque, true);
}

#if SK_SUPPORT_GPU

DEF_GPUTEST(ImageIsOpaqueTest_GPU, reporter, factory) {
    for (int i = 0; i < GrContextFactory::kGLContextTypeCnt; ++i) {
        GrContextFactory::GLContextType glCtxType = (GrContextFactory::GLContextType) i;

        if (!GrContextFactory::IsRenderingGLContext(glCtxType)) {
            continue;
        }

        GrContext* context = factory->get(glCtxType);

        if (NULL == context) {
            continue;
        }

        SkImageInfo infoTransparent = SkImageInfo::MakeN32Premul(5, 5);
        SkAutoTUnref<SkSurface> surfaceTransparent(
            SkSurface::NewRenderTarget(context,SkSurface::kNo_Budgeted, infoTransparent));
        check_isopaque(reporter, surfaceTransparent, false);

        SkImageInfo infoOpaque = SkImageInfo::MakeN32(5, 5, kOpaque_SkAlphaType);
        SkAutoTUnref<SkSurface> surfaceOpaque(
            SkSurface::NewRenderTarget(context,SkSurface::kNo_Budgeted, infoOpaque));
#if 0
        // this is failing right now : TODO fix me
        check_isopaque(reporter, surfaceOpaque, true);
#endif
    }
}

#endif
