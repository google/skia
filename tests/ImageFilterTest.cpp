/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBicubicImageFilter.h"
#include "SkBitmap.h"
#include "SkBitmapDevice.h"
#include "SkBitmapSource.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkDeviceImageFilterProxy.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkFlattenableBuffers.h"
#include "SkLightingImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPicture.h"
#include "SkRect.h"
#include "SkTileImageFilter.h"
#include "SkXfermodeImageFilter.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContextFactory.h"
#include "SkGpuDevice.h"
#endif

static const int kBitmapSize = 4;

namespace {

class MatrixTestImageFilter : public SkImageFilter {
public:
    MatrixTestImageFilter(skiatest::Reporter* reporter, const SkMatrix& expectedMatrix)
      : SkImageFilter(0), fReporter(reporter), fExpectedMatrix(expectedMatrix) {
    }

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context& ctx,
                               SkBitmap* result, SkIPoint* offset) const SK_OVERRIDE {
        REPORTER_ASSERT(fReporter, ctx.ctm() == fExpectedMatrix);
        return true;
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(MatrixTestImageFilter)

protected:
    explicit MatrixTestImageFilter(SkReadBuffer& buffer) : SkImageFilter(0) {
        fReporter = static_cast<skiatest::Reporter*>(buffer.readFunctionPtr());
        buffer.readMatrix(&fExpectedMatrix);
    }

    virtual void flatten(SkWriteBuffer& buffer) const SK_OVERRIDE {
        buffer.writeFunctionPtr(fReporter);
        buffer.writeMatrix(fExpectedMatrix);
    }

private:
    skiatest::Reporter* fReporter;
    SkMatrix fExpectedMatrix;
};

}

static void make_small_bitmap(SkBitmap& bitmap) {
    bitmap.allocN32Pixels(kBitmapSize, kBitmapSize);
    SkCanvas canvas(bitmap);
    canvas.clear(0x00000000);
    SkPaint darkPaint;
    darkPaint.setColor(0xFF804020);
    SkPaint lightPaint;
    lightPaint.setColor(0xFF244484);
    const int i = kBitmapSize / 4;
    for (int y = 0; y < kBitmapSize; y += i) {
        for (int x = 0; x < kBitmapSize; x += i) {
            canvas.save();
            canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas.drawRect(SkRect::MakeXYWH(0, 0,
                                             SkIntToScalar(i),
                                             SkIntToScalar(i)), darkPaint);
            canvas.drawRect(SkRect::MakeXYWH(SkIntToScalar(i),
                                             0,
                                             SkIntToScalar(i),
                                             SkIntToScalar(i)), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(0,
                                             SkIntToScalar(i),
                                             SkIntToScalar(i),
                                             SkIntToScalar(i)), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(SkIntToScalar(i),
                                             SkIntToScalar(i),
                                             SkIntToScalar(i),
                                             SkIntToScalar(i)), darkPaint);
            canvas.restore();
        }
    }
}

static SkImageFilter* make_scale(float amount, SkImageFilter* input = NULL) {
    SkScalar s = amount;
    SkScalar matrix[20] = { s, 0, 0, 0, 0,
                            0, s, 0, 0, 0,
                            0, 0, s, 0, 0,
                            0, 0, 0, s, 0 };
    SkAutoTUnref<SkColorFilter> filter(SkColorMatrixFilter::Create(matrix));
    return SkColorFilterImageFilter::Create(filter, input);
}

static SkImageFilter* make_grayscale(SkImageFilter* input = NULL, const SkImageFilter::CropRect* cropRect = NULL) {
    SkScalar matrix[20];
    memset(matrix, 0, 20 * sizeof(SkScalar));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    SkAutoTUnref<SkColorFilter> filter(SkColorMatrixFilter::Create(matrix));
    return SkColorFilterImageFilter::Create(filter, input, cropRect);
}

DEF_TEST(ImageFilter, reporter) {
    {
        // Check that two non-clipping color matrices concatenate into a single filter.
        SkAutoTUnref<SkImageFilter> halfBrightness(make_scale(0.5f));
        SkAutoTUnref<SkImageFilter> quarterBrightness(make_scale(0.5f, halfBrightness));
        REPORTER_ASSERT(reporter, NULL == quarterBrightness->getInput(0));
    }

    {
        // Check that a clipping color matrix followed by a grayscale does not concatenate into a single filter.
        SkAutoTUnref<SkImageFilter> doubleBrightness(make_scale(2.0f));
        SkAutoTUnref<SkImageFilter> halfBrightness(make_scale(0.5f, doubleBrightness));
        REPORTER_ASSERT(reporter, NULL != halfBrightness->getInput(0));
    }

    {
        // Check that a color filter image filter without a crop rect can be
        // expressed as a color filter.
        SkAutoTUnref<SkImageFilter> gray(make_grayscale());
        REPORTER_ASSERT(reporter, true == gray->asColorFilter(NULL));
    }

    {
        // Check that a color filter image filter with a crop rect cannot
        // be expressed as a color filter.
        SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(0, 0, 100, 100));
        SkAutoTUnref<SkImageFilter> grayWithCrop(make_grayscale(NULL, &cropRect));
        REPORTER_ASSERT(reporter, false == grayWithCrop->asColorFilter(NULL));
    }

    {
        // Tests pass by not asserting
        SkBitmap bitmap, result;
        make_small_bitmap(bitmap);
        result.allocN32Pixels(kBitmapSize, kBitmapSize);

        {
            // This tests for :
            // 1 ) location at (0,0,1)
            SkPoint3 location(0, 0, SK_Scalar1);
            // 2 ) location and target at same value
            SkPoint3 target(location.fX, location.fY, location.fZ);
            // 3 ) large negative specular exponent value
            SkScalar specularExponent = -1000;

            SkAutoTUnref<SkImageFilter> bmSrc(SkBitmapSource::Create(bitmap));
            SkPaint paint;
            paint.setImageFilter(SkLightingImageFilter::CreateSpotLitSpecular(
                    location, target, specularExponent, 180,
                    0xFFFFFFFF, SK_Scalar1, SK_Scalar1, SK_Scalar1,
                    bmSrc))->unref();
            SkCanvas canvas(result);
            SkRect r = SkRect::MakeWH(SkIntToScalar(kBitmapSize),
                                      SkIntToScalar(kBitmapSize));
            canvas.drawRect(r, paint);
        }

        {
            // This tests for scale bringing width to 0
            SkSize scale = SkSize::Make(-0.001f, SK_Scalar1);
            SkAutoTUnref<SkImageFilter> bmSrc(SkBitmapSource::Create(bitmap));
            SkAutoTUnref<SkBicubicImageFilter> bicubic(
                SkBicubicImageFilter::CreateMitchell(scale, bmSrc));
            SkBitmapDevice device(bitmap);
            SkDeviceImageFilterProxy proxy(&device);
            SkIPoint loc = SkIPoint::Make(0, 0);
            // An empty input should early return and return false
            SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeEmpty());
            REPORTER_ASSERT(reporter,
                            !bicubic->filterImage(&proxy, bitmap, ctx, &result, &loc));
        }
    }
}

static void test_crop_rects(SkBaseDevice* device, skiatest::Reporter* reporter) {
    // Check that all filters offset to their absolute crop rect,
    // unaffected by the input crop rect.
    // Tests pass by not asserting.
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    bitmap.eraseARGB(0, 0, 0, 0);
    SkDeviceImageFilterProxy proxy(device);

    SkImageFilter::CropRect inputCropRect(SkRect::MakeXYWH(8, 13, 80, 80));
    SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(20, 30, 60, 60));
    SkAutoTUnref<SkImageFilter> input(make_grayscale(NULL, &inputCropRect));

    SkAutoTUnref<SkColorFilter> cf(SkColorFilter::CreateModeFilter(SK_ColorRED, SkXfermode::kSrcIn_Mode));
    SkPoint3 location(0, 0, SK_Scalar1);
    SkPoint3 target(SK_Scalar1, SK_Scalar1, SK_Scalar1);
    SkScalar kernel[9] = {
        SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
        SkIntToScalar( 1), SkIntToScalar(-7), SkIntToScalar( 1),
        SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
    };
    SkISize kernelSize = SkISize::Make(3, 3);
    SkScalar gain = SK_Scalar1, bias = 0;

    SkImageFilter* filters[] = {
        SkColorFilterImageFilter::Create(cf.get(), input.get(), &cropRect),
        SkDisplacementMapEffect::Create(SkDisplacementMapEffect::kR_ChannelSelectorType,
                                        SkDisplacementMapEffect::kB_ChannelSelectorType,
                                        40.0f, input.get(), input.get(), &cropRect),
        SkBlurImageFilter::Create(SK_Scalar1, SK_Scalar1, input.get(), &cropRect),
        SkDropShadowImageFilter::Create(SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_ColorGREEN, input.get(), &cropRect),
        SkLightingImageFilter::CreatePointLitDiffuse(location, SK_ColorGREEN, 0, 0, input.get(), &cropRect),
        SkLightingImageFilter::CreatePointLitSpecular(location, SK_ColorGREEN, 0, 0, 0, input.get(), &cropRect),
        SkMatrixConvolutionImageFilter::Create(kernelSize, kernel, gain, bias, SkIPoint::Make(1, 1), SkMatrixConvolutionImageFilter::kRepeat_TileMode, false, input.get(), &cropRect),
        SkMergeImageFilter::Create(input.get(), input.get(), SkXfermode::kSrcOver_Mode, &cropRect),
        SkOffsetImageFilter::Create(SK_Scalar1, SK_Scalar1, input.get(), &cropRect),
        SkOffsetImageFilter::Create(SK_Scalar1, SK_Scalar1, input.get(), &cropRect),
        SkDilateImageFilter::Create(3, 2, input.get(), &cropRect),
        SkErodeImageFilter::Create(2, 3, input.get(), &cropRect),
        SkTileImageFilter::Create(inputCropRect.rect(), cropRect.rect(), input.get()),
        SkXfermodeImageFilter::Create(SkXfermode::Create(SkXfermode::kSrcOver_Mode), input.get(), input.get(), &cropRect),
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
        SkImageFilter* filter = filters[i];
        SkBitmap result;
        SkIPoint offset;
        SkString str;
        str.printf("filter %d", static_cast<int>(i));
        SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeLargest());
        REPORTER_ASSERT_MESSAGE(reporter, filter->filterImage(&proxy, bitmap, ctx, &result, &offset), str.c_str());
        REPORTER_ASSERT_MESSAGE(reporter, offset.fX == 20 && offset.fY == 30, str.c_str());
    }

    for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
        SkSafeUnref(filters[i]);
    }
}

DEF_TEST(ImageFilterCropRect, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(100, 100);
    SkBitmapDevice device(temp);
    test_crop_rects(&device, reporter);
}

DEF_TEST(ImageFilterMatrixTest, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(100, 100);
    SkBitmapDevice device(temp);
    SkCanvas canvas(&device);
    canvas.scale(SkIntToScalar(2), SkIntToScalar(2));

    SkMatrix expectedMatrix = canvas.getTotalMatrix();

    SkPicture picture;
    SkCanvas* recordingCanvas = picture.beginRecording(100, 100,
        SkPicture::kOptimizeForClippedPlayback_RecordingFlag);

    SkPaint paint;
    SkAutoTUnref<MatrixTestImageFilter> imageFilter(
        new MatrixTestImageFilter(reporter, expectedMatrix));
    paint.setImageFilter(imageFilter.get());
    SkCanvas::SaveFlags saveFlags = static_cast<SkCanvas::SaveFlags>(
        SkCanvas::kHasAlphaLayer_SaveFlag | SkCanvas::kFullColorLayer_SaveFlag);
    recordingCanvas->saveLayer(NULL, &paint, saveFlags);
    SkPaint solidPaint;
    solidPaint.setColor(0xFFFFFFFF);
    recordingCanvas->save();
    recordingCanvas->scale(SkIntToScalar(10), SkIntToScalar(10));
    recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(100, 100)), solidPaint);
    recordingCanvas->restore(); // scale
    recordingCanvas->restore(); // saveLayer
    picture.endRecording();

    canvas.drawPicture(picture);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST(ImageFilterCropRectGPU, reporter, factory) {
    GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(0));
    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(context,
                                                         SkImageInfo::MakeN32Premul(100, 100),
                                                         0));
    test_crop_rects(device, reporter);
}
#endif
