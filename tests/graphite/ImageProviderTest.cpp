/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSpan.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recording.h"
#include "src/core/SkMipmapBuilder.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

using namespace skgpu::graphite;

namespace {

const SkISize kSurfaceSize = { 16, 16 };
const SkISize kImageSize = { 32, 32 };

constexpr SkColor4f kBaseImageColor = SkColors::kYellow;
constexpr SkColor4f kFirstMipLevelColor = SkColors::kRed;
constexpr SkColor4f kBackgroundColor = SkColors::kBlue;

sk_sp<SkImage> create_and_attach_mipmaps(sk_sp<SkImage> img) {
    constexpr SkColor4f mipLevelColors[] = {
            kFirstMipLevelColor,
            SkColors::kGreen,
            SkColors::kMagenta,
            SkColors::kCyan,
            SkColors::kWhite,
    };

    SkMipmapBuilder builder(img->imageInfo());

    int count = builder.countLevels();

    SkASSERT_RELEASE(count == std::size(mipLevelColors));

    for (int i = 0; i < count; ++i) {
        SkPixmap pm = builder.level(i);
        pm.erase(mipLevelColors[i]);
    }

    return builder.attachTo(img);
}

sk_sp<SkImage> create_raster(Mipmapped mipmapped) {
    SkImageInfo ii = SkImageInfo::Make(kImageSize.width(),
                                       kImageSize.height(),
                                       kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType);
    SkBitmap bm;
    if (!bm.tryAllocPixels(ii)) {
        return nullptr;
    }

    bm.eraseColor(kBaseImageColor);

    sk_sp<SkImage> img = SkImage::MakeFromBitmap(bm);

    if (mipmapped == Mipmapped::kYes) {
        img = create_and_attach_mipmaps(std::move(img));
    }

    return img;
}

/* 0 */
sk_sp<SkImage> create_raster_backed_image_no_mipmaps(Recorder*) {
    return create_raster(Mipmapped::kNo);
}

/* 1 */
sk_sp<SkImage> create_raster_backed_image_with_mipmaps(Recorder*) {
    return create_raster(Mipmapped::kYes);
}

/* 2 */
sk_sp<SkImage> create_gpu_backed_image_no_mipmaps(Recorder* recorder) {
    sk_sp<SkImage> raster = create_raster(Mipmapped::kNo);
    return raster->makeTextureImage(recorder, { Mipmapped::kNo });
}

/* 3 */
sk_sp<SkImage> create_gpu_backed_image_with_mipmaps(Recorder* recorder) {
    sk_sp<SkImage> raster = create_raster(Mipmapped::kYes);
    return raster->makeTextureImage(recorder, { Mipmapped::kYes });
}

/* 4 */
sk_sp<SkImage> create_picture_backed_image(Recorder*) {
    SkIRect r = SkIRect::MakeWH(kImageSize.width(), kImageSize.height());
    SkPaint paint;
    paint.setColor(kBaseImageColor);

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(SkRect::Make(r));
    canvas->drawIRect(r, paint);
    sk_sp<SkPicture> picture = recorder.finishRecordingAsPicture();

    return SkImage::MakeFromPicture(std::move(picture),
                                    r.size(),
                                    /* matrix= */ nullptr,
                                    /* paint= */ nullptr,
                                    SkImage::BitDepth::kU8,
                                    SkColorSpace::MakeSRGB());
}

/* 5 */
sk_sp<SkImage> create_bitmap_generator_backed_image(Recorder*) {

    class BitmapBackedGenerator final : public SkImageGenerator {
    public:
        BitmapBackedGenerator()
                : SkImageGenerator(SkImageInfo::Make(kImageSize.width(),
                                                     kImageSize.height(),
                                                     kRGBA_8888_SkColorType,
                                                     kPremul_SkAlphaType)) {
        }

        bool onGetPixels(const SkImageInfo& dstInfo,
                         void* pixels,
                         size_t rowBytes,
                         const Options&) override {

            if (dstInfo.dimensions() != kImageSize) {
                return false;
            }

            SkBitmap bm;
            if (!bm.tryAllocPixels(dstInfo)) {
                return false;
            }

            bm.eraseColor(kBaseImageColor);

            return bm.readPixels(dstInfo, pixels, rowBytes, 0, 0);
        }
    };

    std::unique_ptr<SkImageGenerator> gen(new BitmapBackedGenerator());

    return SkImage::MakeFromGenerator(std::move(gen));
}

bool check_img(skiatest::Reporter* reporter,
               Context* context,
               Recorder* recorder,
               SkImage* imageToDraw,
               Mipmapped mipmapped,
               const char* testcase,
               const SkColor4f& expectedColor) {
    SkImageInfo ii = SkImageInfo::Make(kSurfaceSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkBitmap result;
    result.allocPixels(ii);
    SkPixmap pm;

    SkAssertResult(result.peekPixels(&pm));

    {
        sk_sp<SkSurface> surface = SkSurface::MakeGraphite(recorder, ii);
        if (!surface) {
            ERRORF(reporter, "Surface creation failed");
            return false;
        }

        SkCanvas* canvas = surface->getCanvas();

        canvas->clear(kBackgroundColor);

        SkSamplingOptions sampling = (mipmapped == Mipmapped::kYes)
                ? SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNearest)
                : SkSamplingOptions(SkFilterMode::kLinear);

        canvas->drawImageRect(imageToDraw,
                              SkRect::MakeWH(kSurfaceSize.width(), kSurfaceSize.height()),
                              sampling);

        // For now, we cast and call directly into Surface. Once we have a better idea of
        // what the public API for synchronous graphite readPixels we can update this call to use
        // that instead.
        if (!static_cast<skgpu::graphite::Surface*>(surface.get())->onReadPixels(context,
                                                                                 recorder,
                                                                                 pm,
                                                                                 0,
                                                                                 0)) {
            ERRORF(reporter, "readPixels failed");
            return false;
        }
    }

    auto error = std::function<ComparePixmapsErrorReporter>(
            [&](int x, int y, const float diffs[4]) {
                ERRORF(reporter,
                       "case %s %s: expected (%.1f %.1f %.1f %.1f) got (%.1f, %.1f, %.1f, %.1f)",
                       testcase,
                       (mipmapped == Mipmapped::kYes) ? "w/ mipmaps" : "w/o mipmaps",
                       expectedColor.fR, expectedColor.fG, expectedColor.fB, expectedColor.fA,
                       expectedColor.fR-diffs[0], expectedColor.fG-diffs[1],
                       expectedColor.fB-diffs[2], expectedColor.fA-diffs[3]);
            });
    static constexpr float kTol[] = {0, 0, 0, 0};
    CheckSolidPixels(expectedColor, pm, kTol, error);

    return true;
}

using FactoryT = sk_sp<SkImage> (*)(Recorder*);

struct TestCase {
    const char* fTestCase;
    FactoryT    fFactory;
    SkColor4f   fExpectedColors[2];   /* [ w/o mipmaps, w/ mipmaps ] */
};

void run_test(skiatest::Reporter* reporter,
              Context* context,
              Recorder* recorder,
              SkSpan<TestCase> testcases) {

    for (auto t : testcases) {
        for (auto mm : { Mipmapped::kNo, Mipmapped::kYes }) {
            sk_sp<SkImage> image = t.fFactory(recorder);

            check_img(reporter, context, recorder, image.get(), mm,
                      t.fTestCase, t.fExpectedColors[static_cast<int>(mm)]);
        }
    }
}

} // anonymous namespace

// This test creates a bunch of solid yellow images in different ways and then draws them into a
// smaller surface (w/ src mode) that has been initialized to solid blue. When mipmap levels
// are possible to be specified the first mipmap level is made red. Thus, when mipmapping
// is allowed and it is specified as the sample mode, the drawn image will be red.

// For the Default ImageProvider (which does _no_ caching and conversion) the expectations are:
//
//    0) raster-backed image w/o mipmaps
//                    drawn w/o mipmapping    --> dropped draw (blue)
//                    drawn w/ mipmapping     --> dropped draw (blue)
//
//    1) raster-backed image w/ mipmaps
//                    drawn w/o mipmapping    --> dropped draw (blue)
//                    drawn w/ mipmapping     --> dropped draw (blue)
//
//    2) Graphite-backed w/o mipmaps
//                    drawn w/o mipmapping    --> drawn (yellow)
//                    drawn w/ mipmapping     --> drawn (yellow) - mipmap filtering is dropped
//
//    3) Graphite-backed w/ mipmaps
//                    drawn w/o mipmapping    --> drawn (yellow)
//                    drawn w/ mipmapping     --> drawn (red)
//
//    4) picture-backed image
//                    drawn w/o mipmapping    --> dropped draw (blue)
//                    drawn w/ mipmapping     --> dropped draw (blue)
//
//    5) bitmap-backed-generator based image
//                    drawn w/o mipmapping    --> dropped draw (blue)
//                    drawn w/ mipmapping     --> dropped draw (blue)
//
DEF_GRAPHITE_TEST_FOR_CONTEXTS(ImageProviderTest_Graphite_Default, reporter, context) {
    TestCase testcases[] = {
        { "0", create_raster_backed_image_no_mipmaps,   { kBackgroundColor, kBackgroundColor } },
        { "1", create_raster_backed_image_with_mipmaps, { kBackgroundColor, kBackgroundColor } },
        { "2", create_gpu_backed_image_no_mipmaps,      { kBaseImageColor,  kBaseImageColor } },
        { "3", create_gpu_backed_image_with_mipmaps,    { kBaseImageColor,  kFirstMipLevelColor } },
        { "4", create_picture_backed_image,             { kBackgroundColor, kBackgroundColor } },
        { "5", create_bitmap_generator_backed_image,    { kBackgroundColor, kBackgroundColor }  },
    };

    std::unique_ptr<Recorder> recorder = context->makeRecorder();

    run_test(reporter, context, recorder.get(), testcases);
}

// For the Testing ImageProvider (which does some caching and conversion) the expectations are:
//
//    0) raster-backed image w/o mipmaps
//                    drawn w/o mipmapping    --> drawn (yellow) - auto-converted
//                    drawn w/ mipmapping     --> drawn (yellow) - auto-converted
//
//    1) raster-backed image w/ mipmaps
//                    drawn w/o mipmapping    --> drawn (yellow) - auto-converted
//                    drawn w/ mipmapping     --> drawn (red) - auto-converted
//
//    2) Graphite-backed w/o mipmaps
//                    drawn w/o mipmapping    --> drawn (yellow)
//                    drawn w/ mipmapping     --> drawn (yellow) - mipmap filtering is dropped
//
//    3) Graphite-backed w/ mipmaps
//                    drawn w/o mipmapping    --> drawn (yellow)
//                    drawn w/ mipmapping     --> drawn (red)
//
//    4) picture-backed image
//                    drawn w/o mipmapping    --> drawn (yellow) - auto-converted
//                    drawn w/ mipmapping     --> drawn (yellow) - auto-converted
//
//    5) bitmap-backed-generator based image
//                    drawn w/o mipmapping    --> drawn (yellow) - auto-converted
//                    drawn w/ mipmapping     --> drawn (yellow) - auto-converted
//
DEF_GRAPHITE_TEST_FOR_CONTEXTS(ImageProviderTest_Graphite_Testing, reporter, context) {
    TestCase testcases[] = {
        { "0", create_raster_backed_image_no_mipmaps,   { kBaseImageColor, kBaseImageColor } },
        { "1", create_raster_backed_image_with_mipmaps, { kBaseImageColor, kFirstMipLevelColor } },
        { "2", create_gpu_backed_image_no_mipmaps,      { kBaseImageColor, kBaseImageColor } },
        { "3", create_gpu_backed_image_with_mipmaps,    { kBaseImageColor, kFirstMipLevelColor } },
        { "4", create_picture_backed_image,             { kBaseImageColor, kBaseImageColor } },
        { "5", create_bitmap_generator_backed_image,    { kBaseImageColor, kBaseImageColor } },
    };

    RecorderOptions options = ToolUtils::CreateTestingRecorderOptions();
    std::unique_ptr<skgpu::graphite::Recorder> recorder = context->makeRecorder(options);

    run_test(reporter, context, recorder.get(), testcases);
}
