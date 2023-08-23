/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "tests/Test.h"
#include "tools/ToolUtils.h"

using namespace skgpu::graphite;

namespace {

sk_sp<SkPicture> create_picture(int width, int height) {
    const SkRect bounds = SkRect::MakeWH(width, height);

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(bounds);

    SkPaint paint;
    paint.setColor(SK_ColorMAGENTA);

    canvas->drawRect(bounds, paint);

    return recorder.finishRecordingAsPicture();
}


SkBitmap create_bitmap(int width, int height) {
    SkImageInfo ii = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkBitmap bm;
    bm.allocPixels(ii);

    bm.eraseColor(SK_ColorMAGENTA);

    bm.setImmutable();
    return bm;
}

} // anonymous namespace

// In this test we just iterate through the cases we expect to work and verify that rewrapping the
// base SkPicture doesn't block finding the earlier cached image.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(CacheKeyTest_Picture, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    RecorderOptions options = ToolUtils::CreateTestingRecorderOptions();

    sk_sp<ImageProvider> provider = options.fImageProvider;

    sk_sp<SkPicture> picture = create_picture(128, 128);
    const SkMatrix xlate = SkMatrix::Translate(10.0f, -10.0f);
    sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();

    for (auto bitDepth : { SkImages::BitDepth::kU8, SkImages::BitDepth::kF16 }) {
        for (const SkMatrix* mat : { static_cast<const SkMatrix*>(nullptr), &xlate }) {
            for (bool mipmapped : { false, true }) {
                for (uint32_t flags : { 0, int(SkSurfaceProps::kAlwaysDither_Flag) } ) {
                    for (SkPixelGeometry geometry : { kUnknown_SkPixelGeometry,
                                                      kRGB_H_SkPixelGeometry } ) {
                        sk_sp<SkImage> image1 = SkImages::DeferredFromPicture(picture, {128, 128},
                                                                              mat, nullptr,
                                                                              bitDepth, srgb,
                                                                              { flags, geometry });

                        sk_sp<SkImage> result1 = provider->findOrCreate(recorder.get(),
                                                                        image1.get(),
                                                                        {mipmapped});

                        sk_sp<SkImage> image2 = SkImages::DeferredFromPicture(picture, {128, 128},
                                                                              mat, nullptr,
                                                                              bitDepth, srgb,
                                                                              { flags, geometry });
                        REPORTER_ASSERT(reporter, image1->uniqueID() != image2->uniqueID());

                        sk_sp<SkImage> result2 = provider->findOrCreate(recorder.get(),
                                                                        image2.get(),
                                                                        {mipmapped});
                        REPORTER_ASSERT(reporter, result1 == result2);
                    }
                }
            }
        }
    }
}

// In this test we just iterate through the cases we expect to work and verify that rewrapping the
// subsetted SkBitmap doesn't block finding the earlier cached image.
DEF_GRAPHITE_TEST_FOR_ALL_CONTEXTS(CacheKeyTest_Bitmap, reporter, context,
                                   CtsEnforcement::kNextRelease) {
    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    RecorderOptions options = ToolUtils::CreateTestingRecorderOptions();

    sk_sp<ImageProvider> provider = options.fImageProvider;

    SkBitmap orig = create_bitmap(128, 128);
    SkBitmap subset;
    orig.extractSubset(&subset, SkIRect::MakeXYWH(32, 32, 64, 64));
    SkASSERT(orig.getGenerationID() == subset.getGenerationID());

    for (bool mipmapped : { false, true }) {
        sk_sp<SkImage> image1 = SkImages::RasterFromBitmap(subset);

        sk_sp<SkImage> result1 = provider->findOrCreate(recorder.get(),
                                                        image1.get(),
                                                        {mipmapped});

        sk_sp<SkImage> image2 = SkImages::RasterFromBitmap(subset);
        REPORTER_ASSERT(reporter, image1->uniqueID() != image2->uniqueID());

        sk_sp<SkImage> result2 = provider->findOrCreate(recorder.get(),
                                                        image2.get(),
                                                        {mipmapped});
        REPORTER_ASSERT(reporter, result1 == result2);
    }
}
