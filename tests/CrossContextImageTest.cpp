/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrContextFactory.h"
#include "Resources.h"
#include "SkAutoPixmapStorage.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkCrossContextImageData.h"
#include "SkSemaphore.h"
#include "SkSurface.h"
#include "SkThreadUtils.h"
#include "Test.h"

using namespace sk_gpu_test;

static SkImageInfo read_pixels_info(SkImage* image) {
    return SkImageInfo::MakeN32(image->width(), image->height(), image->alphaType());
}

static bool colors_are_close(SkColor a, SkColor b, int error) {
    return SkTAbs((int)SkColorGetR(a) - (int)SkColorGetR(b)) <= error &&
           SkTAbs((int)SkColorGetG(a) - (int)SkColorGetG(b)) <= error &&
           SkTAbs((int)SkColorGetB(a) - (int)SkColorGetB(b)) <= error;
}

static void assert_equal(skiatest::Reporter* reporter, SkImage* a, SkImage* b, int error) {
    REPORTER_ASSERT(reporter, a->width() == b->width());
    REPORTER_ASSERT(reporter, a->height() == b->height());

    SkAutoPixmapStorage pmapA, pmapB;
    pmapA.alloc(read_pixels_info(a));
    pmapB.alloc(read_pixels_info(b));

    REPORTER_ASSERT(reporter, a->readPixels(pmapA, 0, 0));
    REPORTER_ASSERT(reporter, b->readPixels(pmapB, 0, 0));

    for (int y = 0; y < a->height(); ++y) {
        for (int x = 0; x < a->width(); ++x) {
            SkColor ca = pmapA.getColor(x, y);
            SkColor cb = pmapB.getColor(x, y);
            if (!error) {
                if (ca != cb) {
                    ERRORF(reporter, "Expected 0x%08x but got 0x%08x at (%d, %d)", ca, cb, x, y);
                    return;
                }
            } else {
                if (!colors_are_close(ca, cb, error)) {
                    ERRORF(reporter, "Expected 0x%08x +-%d but got 0x%08x at (%d, %d)",
                           ca, error, cb, x, y);
                    return;
                }
            }
        }
    }
}

static void draw_image_test_pattern(SkCanvas* canvas) {
    canvas->clear(SK_ColorWHITE);
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    canvas->drawRect(SkRect::MakeXYWH(5, 5, 10, 10), paint);
}

static sk_sp<SkImage> create_test_image() {
    SkBitmap bm;
    bm.allocN32Pixels(20, 20, true);
    SkCanvas canvas(bm);
    draw_image_test_pattern(&canvas);

    return SkImage::MakeFromBitmap(bm);
}

static sk_sp<SkData> create_test_data(SkEncodedImageFormat format) {
    auto image = create_test_image();
    return sk_sp<SkData>(image->encode(format, 100));
}

DEF_GPUTEST(CrossContextImage_SameContext, reporter, /*factory*/) {
    GrContextFactory factory;
    sk_sp<SkImage> testImage = create_test_image();

    // Test both PNG and JPG, to exercise GPU YUV conversion
    for (auto format : { SkEncodedImageFormat::kPNG, SkEncodedImageFormat::kJPEG }) {
        sk_sp<SkData> encoded = create_test_data(format);

        for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
            GrContextFactory::ContextType ctxType = static_cast<GrContextFactory::ContextType>(i);
            if (!sk_gpu_test::GrContextFactory::IsRenderingContext(ctxType)) {
                continue;
            }

            ContextInfo info = factory.getContextInfo(ctxType);
            if (!info.grContext()) {
                continue;
            }

            auto ccid = SkCrossContextImageData::MakeFromEncoded(info.grContext(), encoded,
                                                                 nullptr);
            REPORTER_ASSERT(reporter, ccid != nullptr);

            auto image = SkImage::MakeFromCrossContextImageData(info.grContext(), std::move(ccid));
            REPORTER_ASSERT(reporter, image != nullptr);

            // JPEG encode -> decode won't round trip the image perfectly
            assert_equal(reporter, testImage.get(), image.get(),
                         SkEncodedImageFormat::kJPEG == format ? 2 : 0);
        }
    }
}

DEF_GPUTEST(CrossContextImage_SharedContextSameThread, reporter, /*factory*/) {
    GrContextFactory factory;
    sk_sp<SkImage> testImage = create_test_image();

    // Test both PNG and JPG, to exercise GPU YUV conversion
    for (auto format : { SkEncodedImageFormat::kPNG, SkEncodedImageFormat::kJPEG }) {
        sk_sp<SkData> encoded = create_test_data(format);

        for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
            GrContextFactory::ContextType ctxType = static_cast<GrContextFactory::ContextType>(i);
            if (!sk_gpu_test::GrContextFactory::IsRenderingContext(ctxType)) {
                continue;
            }

            ContextInfo info = factory.getContextInfo(ctxType);
            if (!info.grContext()) {
                continue;
            }
            auto ccid = SkCrossContextImageData::MakeFromEncoded(info.grContext(), encoded,
                                                                 nullptr);
            REPORTER_ASSERT(reporter, ccid != nullptr);

            ContextInfo info2 = factory.getSharedContextInfo(info.grContext());
            GrContext* ctx2 = info2.grContext();
            int resourceCountBefore = 0, resourceCountAfter = 0;
            size_t resourceBytesBefore = 0, resourceBytesAfter = 0;
            if (ctx2 && info.grContext()->caps()->crossContextTextureSupport()) {
                ctx2->getResourceCacheUsage(&resourceCountBefore, &resourceBytesBefore);
            }

            auto image = SkImage::MakeFromCrossContextImageData(ctx2, std::move(ccid));
            REPORTER_ASSERT(reporter, image != nullptr);

            if (ctx2 && info.grContext()->caps()->crossContextTextureSupport()) {
                // MakeFromCrossContextImageData should have imported the texture back into our
                // cache, so we should see an uptick. (If we have crossContextTextureSupport,
                // otherwise we're just handing around a CPU or codec-backed image, so no cache
                // impact will occur).
                ctx2->getResourceCacheUsage(&resourceCountAfter, &resourceBytesAfter);
                REPORTER_ASSERT(reporter, resourceCountAfter == resourceCountBefore + 1);
                REPORTER_ASSERT(reporter, resourceBytesAfter > resourceBytesBefore);
            }

            // JPEG encode -> decode won't round trip the image perfectly
            assert_equal(reporter, testImage.get(), image.get(),
                         SkEncodedImageFormat::kJPEG == format ? 2 : 0);
        }
    }
}

namespace {
struct CrossContextImage_ThreadContext {
    GrContext* fGrContext;
    sk_gpu_test::TestContext* fTestContext;
    SkSemaphore fSemaphore;
    std::unique_ptr<SkCrossContextImageData> fCCID;
    sk_sp<SkData> fEncoded;
};
}

static void upload_image_thread_proc(void* data) {
    CrossContextImage_ThreadContext* ctx = static_cast<CrossContextImage_ThreadContext*>(data);
    ctx->fTestContext->makeCurrent();
    ctx->fCCID = SkCrossContextImageData::MakeFromEncoded(ctx->fGrContext, ctx->fEncoded, nullptr);
    ctx->fSemaphore.signal();
}

DEF_GPUTEST(CrossContextImage_SharedContextOtherThread, reporter, /*factory*/) {
    sk_sp<SkImage> testImage = create_test_image();

    // Test both PNG and JPG, to exercise GPU YUV conversion
    for (auto format : { SkEncodedImageFormat::kPNG, SkEncodedImageFormat::kJPEG }) {
        // Use a new factory for each batch of tests. Otherwise the shared context will still be
        // current on the upload thread when we do the second iteration, and we get undefined
        // behavior.
        GrContextFactory factory;
        sk_sp<SkData> encoded = create_test_data(format);

        for (int i = 0; i < GrContextFactory::kContextTypeCnt; ++i) {
            GrContextFactory::ContextType ctxType = static_cast<GrContextFactory::ContextType>(i);
            if (!sk_gpu_test::GrContextFactory::IsRenderingContext(ctxType)) {
                continue;
            }

            // Create two GrContexts in a share group
            ContextInfo info = factory.getContextInfo(ctxType);
            if (!info.grContext()) {
                continue;
            }
            ContextInfo info2 = factory.getSharedContextInfo(info.grContext());
            if (!info2.grContext()) {
                continue;
            }

            // Make the first one current (on this thread) again
            info.testContext()->makeCurrent();

            // Bundle up data for the worker thread
            CrossContextImage_ThreadContext ctx;
            ctx.fGrContext = info2.grContext();
            ctx.fTestContext = info2.testContext();
            ctx.fEncoded = encoded;

            SkThread uploadThread(upload_image_thread_proc, &ctx);
            SkAssertResult(uploadThread.start());

            ctx.fSemaphore.wait();
            auto image = SkImage::MakeFromCrossContextImageData(info.grContext(),
                                                                std::move(ctx.fCCID));
            REPORTER_ASSERT(reporter, image != nullptr);

            // JPEG encode -> decode won't round trip the image perfectly
            assert_equal(reporter, testImage.get(), image.get(),
                         SkEncodedImageFormat::kJPEG == format ? 2 : 0);

            uploadThread.join();
        }
    }
}

#endif
