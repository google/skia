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
#include "SkFlattenableSerialization.h"
#include "SkGradientShader.h"
#include "SkLightingImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMatrixImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPicture.h"
#include "SkPictureImageFilter.h"
#include "SkPictureRecorder.h"
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
        // Check that two non-commutative matrices are concatenated in
        // the correct order.
        SkScalar blueToRedMatrix[20] = { 0 };
        blueToRedMatrix[2] = blueToRedMatrix[18] = SK_Scalar1;
        SkScalar redToGreenMatrix[20] = { 0 };
        redToGreenMatrix[5] = redToGreenMatrix[18] = SK_Scalar1;
        SkAutoTUnref<SkColorFilter> blueToRed(SkColorMatrixFilter::Create(blueToRedMatrix));
        SkAutoTUnref<SkImageFilter> filter1(SkColorFilterImageFilter::Create(blueToRed.get()));
        SkAutoTUnref<SkColorFilter> redToGreen(SkColorMatrixFilter::Create(redToGreenMatrix));
        SkAutoTUnref<SkImageFilter> filter2(SkColorFilterImageFilter::Create(redToGreen.get(), filter1.get()));

        SkBitmap result;
        result.allocN32Pixels(kBitmapSize, kBitmapSize);

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setImageFilter(filter2.get());
        SkCanvas canvas(result);
        canvas.clear(0x0);
        SkRect rect = SkRect::Make(SkIRect::MakeWH(kBitmapSize, kBitmapSize));
        canvas.drawRect(rect, paint);
        uint32_t pixel = *result.getAddr32(0, 0);
        // The result here should be green, since we have effectively shifted blue to green.
        REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
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
            SkAutoTUnref<SkImageFilter::Cache> cache(SkImageFilter::Cache::Create(2));
            SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeEmpty(), cache.get());
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
        SkAutoTUnref<SkImageFilter::Cache> cache(SkImageFilter::Cache::Create(2));
        SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeLargest(), cache.get());
        REPORTER_ASSERT_MESSAGE(reporter, filter->filterImage(&proxy, bitmap, ctx,
                                &result, &offset), str.c_str());
        REPORTER_ASSERT_MESSAGE(reporter, offset.fX == 20 && offset.fY == 30, str.c_str());
    }

    for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
        SkSafeUnref(filters[i]);
    }
}

static SkBitmap make_gradient_circle(int width, int height) {
    SkBitmap bitmap;
    SkScalar x = SkIntToScalar(width / 2);
    SkScalar y = SkIntToScalar(height / 2);
    SkScalar radius = SkMinScalar(x, y) * 0.8f;
    bitmap.allocN32Pixels(width, height);
    SkCanvas canvas(bitmap);
    canvas.clear(0x00000000);
    SkColor colors[2];
    colors[0] = SK_ColorWHITE;
    colors[1] = SK_ColorBLACK;
    SkAutoTUnref<SkShader> shader(
        SkGradientShader::CreateRadial(SkPoint::Make(x, y), radius, colors, NULL, 2,
                                       SkShader::kClamp_TileMode)
    );
    SkPaint paint;
    paint.setShader(shader);
    canvas.drawCircle(x, y, radius, paint);
    return bitmap;
}

DEF_TEST(ImageFilterDrawTiled, reporter) {
    // Check that all filters when drawn tiled (with subsequent clip rects) exactly
    // match the same filters drawn with a single full-canvas bitmap draw.
    // Tests pass by not asserting.

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
    SkScalar five = SkIntToScalar(5);

    SkAutoTUnref<SkImageFilter> gradient_source(SkBitmapSource::Create(make_gradient_circle(64, 64)));
    SkAutoTUnref<SkImageFilter> blur(SkBlurImageFilter::Create(five, five));
    SkMatrix matrix;

    matrix.setTranslate(SK_Scalar1, SK_Scalar1);
    matrix.postRotate(SkIntToScalar(45), SK_Scalar1, SK_Scalar1);

    SkRTreeFactory factory;
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(64, 64, &factory, 0);

    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeXYWH(10, 10, 30, 20)), greenPaint);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());
    SkAutoTUnref<SkImageFilter> pictureFilter(SkPictureImageFilter::Create(picture.get()));

    struct {
        const char*    fName;
        SkImageFilter* fFilter;
    } filters[] = {
        { "color filter", SkColorFilterImageFilter::Create(cf.get()) },
        { "displacement map", SkDisplacementMapEffect::Create(
              SkDisplacementMapEffect::kR_ChannelSelectorType,
              SkDisplacementMapEffect::kB_ChannelSelectorType,
              20.0f, gradient_source.get()) },
        { "blur", SkBlurImageFilter::Create(SK_Scalar1, SK_Scalar1) },
        { "drop shadow", SkDropShadowImageFilter::Create(
              SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_ColorGREEN) },
        { "diffuse lighting", SkLightingImageFilter::CreatePointLitDiffuse(
              location, SK_ColorGREEN, 0, 0) },
        { "specular lighting",
              SkLightingImageFilter::CreatePointLitSpecular(location, SK_ColorGREEN, 0, 0, 0) },
        { "matrix convolution",
              SkMatrixConvolutionImageFilter::Create(
                  kernelSize, kernel, gain, bias, SkIPoint::Make(1, 1),
                  SkMatrixConvolutionImageFilter::kRepeat_TileMode, false) },
        { "merge", SkMergeImageFilter::Create(NULL, NULL, SkXfermode::kSrcOver_Mode) },
        { "offset", SkOffsetImageFilter::Create(SK_Scalar1, SK_Scalar1) },
        { "dilate", SkDilateImageFilter::Create(3, 2) },
        { "erode", SkErodeImageFilter::Create(2, 3) },
        { "tile", SkTileImageFilter::Create(SkRect::MakeXYWH(0, 0, 50, 50),
                                            SkRect::MakeXYWH(0, 0, 100, 100), NULL) },
        { "matrix", SkMatrixImageFilter::Create(matrix, SkPaint::kLow_FilterLevel) },
        { "blur and offset", SkOffsetImageFilter::Create(five, five, blur.get()) },
        { "picture and blur", SkBlurImageFilter::Create(five, five, pictureFilter.get()) },
    };

    SkBitmap untiledResult, tiledResult;
    int width = 64, height = 64;
    untiledResult.allocN32Pixels(width, height);
    tiledResult.allocN32Pixels(width, height);
    SkCanvas tiledCanvas(tiledResult);
    SkCanvas untiledCanvas(untiledResult);
    int tileSize = 8;

    for (int scale = 1; scale <= 2; ++scale) {
        for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
            tiledCanvas.clear(0);
            untiledCanvas.clear(0);
            SkPaint paint;
            paint.setImageFilter(filters[i].fFilter);
            paint.setTextSize(SkIntToScalar(height));
            paint.setColor(SK_ColorWHITE);
            SkString str;
            const char* text = "ABC";
            SkScalar ypos = SkIntToScalar(height);
            untiledCanvas.save();
            untiledCanvas.scale(SkIntToScalar(scale), SkIntToScalar(scale));
            untiledCanvas.drawText(text, strlen(text), 0, ypos, paint);
            untiledCanvas.restore();
            for (int y = 0; y < height; y += tileSize) {
                for (int x = 0; x < width; x += tileSize) {
                    tiledCanvas.save();
                    tiledCanvas.clipRect(SkRect::Make(SkIRect::MakeXYWH(x, y, tileSize, tileSize)));
                    tiledCanvas.scale(SkIntToScalar(scale), SkIntToScalar(scale));
                    tiledCanvas.drawText(text, strlen(text), 0, ypos, paint);
                    tiledCanvas.restore();
                }
            }
            untiledCanvas.flush();
            tiledCanvas.flush();
            for (int y = 0; y < height; y++) {
                int diffs = memcmp(untiledResult.getAddr32(0, y), tiledResult.getAddr32(0, y), untiledResult.rowBytes());
                REPORTER_ASSERT_MESSAGE(reporter, !diffs, filters[i].fName);
                if (diffs) {
                    break;
                }
            }
        }
    }

    for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
        SkSafeUnref(filters[i].fFilter);
    }
}

DEF_TEST(ImageFilterMatrixConvolution, reporter) {
    // Check that a 1x3 filter does not cause a spurious assert.
    SkScalar kernel[3] = {
        SkIntToScalar( 1), SkIntToScalar( 1), SkIntToScalar( 1),
    };
    SkISize kernelSize = SkISize::Make(1, 3);
    SkScalar gain = SK_Scalar1, bias = 0;
    SkIPoint kernelOffset = SkIPoint::Make(0, 0);

    SkAutoTUnref<SkImageFilter> filter(
        SkMatrixConvolutionImageFilter::Create(
            kernelSize, kernel, gain, bias, kernelOffset,
            SkMatrixConvolutionImageFilter::kRepeat_TileMode, false));

    SkBitmap result;
    int width = 16, height = 16;
    result.allocN32Pixels(width, height);
    SkCanvas canvas(result);
    canvas.clear(0);

    SkPaint paint;
    paint.setImageFilter(filter);
    SkRect rect = SkRect::Make(SkIRect::MakeWH(width, height));
    canvas.drawRect(rect, paint);
}

DEF_TEST(ImageFilterMatrixConvolutionBorder, reporter) {
    // Check that a filter with borders outside the target bounds
    // does not crash.
    SkScalar kernel[3] = {
        0, 0, 0,
    };
    SkISize kernelSize = SkISize::Make(3, 1);
    SkScalar gain = SK_Scalar1, bias = 0;
    SkIPoint kernelOffset = SkIPoint::Make(2, 0);

    SkAutoTUnref<SkImageFilter> filter(
        SkMatrixConvolutionImageFilter::Create(
            kernelSize, kernel, gain, bias, kernelOffset,
            SkMatrixConvolutionImageFilter::kClamp_TileMode, true));

    SkBitmap result;

    int width = 10, height = 10;
    result.allocN32Pixels(width, height);
    SkCanvas canvas(result);
    canvas.clear(0);

    SkPaint filterPaint;
    filterPaint.setImageFilter(filter);
    SkRect bounds = SkRect::MakeWH(1, 10);
    SkRect rect = SkRect::Make(SkIRect::MakeWH(width, height));
    SkPaint rectPaint;
    canvas.saveLayer(&bounds, &filterPaint);
    canvas.drawRect(rect, rectPaint);
    canvas.restore();
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

    SkRTreeFactory factory;
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(100, 100, &factory, 0);

    SkPaint paint;
    SkAutoTUnref<MatrixTestImageFilter> imageFilter(
        new MatrixTestImageFilter(reporter, expectedMatrix));
    paint.setImageFilter(imageFilter.get());
    recordingCanvas->saveLayer(NULL, &paint);
    SkPaint solidPaint;
    solidPaint.setColor(0xFFFFFFFF);
    recordingCanvas->save();
    recordingCanvas->scale(SkIntToScalar(10), SkIntToScalar(10));
    recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(100, 100)), solidPaint);
    recordingCanvas->restore(); // scale
    recordingCanvas->restore(); // saveLayer
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());

    canvas.drawPicture(picture);
}

DEF_TEST(ImageFilterPictureImageFilterTest, reporter) {

    SkRTreeFactory factory;
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(1, 1, &factory, 0);

    // Create an SkPicture which simply draws a green 1x1 rectangle.
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(1, 1)), greenPaint);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());

    // Wrap that SkPicture in an SkPictureImageFilter.
    SkAutoTUnref<SkImageFilter> imageFilter(
        SkPictureImageFilter::Create(picture.get()));

    // Check that SkPictureImageFilter successfully serializes its contained
    // SkPicture when not in cross-process mode.
    SkPaint paint;
    paint.setImageFilter(imageFilter.get());
    SkPictureRecorder outerRecorder;
    SkCanvas* outerCanvas = outerRecorder.beginRecording(1, 1, &factory, 0);
    SkPaint redPaintWithFilter;
    redPaintWithFilter.setColor(SK_ColorRED);
    redPaintWithFilter.setImageFilter(imageFilter.get());
    outerCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(1, 1)), redPaintWithFilter);
    SkAutoTUnref<SkPicture> outerPicture(outerRecorder.endRecording());

    SkBitmap bitmap;
    bitmap.allocN32Pixels(1, 1);
    SkBitmapDevice device(bitmap);
    SkCanvas canvas(&device);

    // The result here should be green, since the filter replaces the primitive's red interior.
    canvas.clear(0x0);
    canvas.drawPicture(outerPicture);
    uint32_t pixel = *bitmap.getAddr32(0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    // Check that, for now, SkPictureImageFilter does not serialize or
    // deserialize its contained picture when the filter is serialized
    // cross-process. Do this by "laundering" it through SkValidatingReadBuffer.
    SkAutoTUnref<SkData> data(SkValidatingSerializeFlattenable(imageFilter.get()));
    SkAutoTUnref<SkFlattenable> flattenable(SkValidatingDeserializeFlattenable(
        data->data(), data->size(), SkImageFilter::GetFlattenableType()));
    SkImageFilter* unflattenedFilter = static_cast<SkImageFilter*>(flattenable.get());

    redPaintWithFilter.setImageFilter(unflattenedFilter);
    SkPictureRecorder crossProcessRecorder;
    SkCanvas* crossProcessCanvas = crossProcessRecorder.beginRecording(1, 1, &factory, 0);
    crossProcessCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(1, 1)), redPaintWithFilter);
    SkAutoTUnref<SkPicture> crossProcessPicture(crossProcessRecorder.endRecording());

    canvas.clear(0x0);
    canvas.drawPicture(crossProcessPicture);
    pixel = *bitmap.getAddr32(0, 0);
    // The result here should not be green, since the filter draws nothing.
    REPORTER_ASSERT(reporter, pixel != SK_ColorGREEN);
}

DEF_TEST(ImageFilterEmptySaveLayerTest, reporter) {

    // Even when there's an empty saveLayer()/restore(), ensure that an image
    // filter or color filter which affects transparent black still draws.

    SkBitmap bitmap;
    bitmap.allocN32Pixels(10, 10);
    SkBitmapDevice device(bitmap);
    SkCanvas canvas(&device);

    SkRTreeFactory factory;
    SkPictureRecorder recorder;

    SkAutoTUnref<SkColorFilter> green(
        SkColorFilter::CreateModeFilter(SK_ColorGREEN, SkXfermode::kSrc_Mode));
    SkAutoTUnref<SkColorFilterImageFilter> imageFilter(
        SkColorFilterImageFilter::Create(green.get()));
    SkPaint imageFilterPaint;
    imageFilterPaint.setImageFilter(imageFilter.get());
    SkPaint colorFilterPaint;
    colorFilterPaint.setColorFilter(green.get());

    SkRect bounds = SkRect::MakeWH(10, 10);

    SkCanvas* recordingCanvas = recorder.beginRecording(10, 10, &factory, 0);
    recordingCanvas->saveLayer(&bounds, &imageFilterPaint);
    recordingCanvas->restore();
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());

    canvas.clear(0);
    canvas.drawPicture(picture);
    uint32_t pixel = *bitmap.getAddr32(0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    recordingCanvas = recorder.beginRecording(10, 10, &factory, 0);
    recordingCanvas->saveLayer(NULL, &imageFilterPaint);
    recordingCanvas->restore();
    SkAutoTUnref<SkPicture> picture2(recorder.endRecording());

    canvas.clear(0);
    canvas.drawPicture(picture2);
    pixel = *bitmap.getAddr32(0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    recordingCanvas = recorder.beginRecording(10, 10, &factory, 0);
    recordingCanvas->saveLayer(&bounds, &colorFilterPaint);
    recordingCanvas->restore();
    SkAutoTUnref<SkPicture> picture3(recorder.endRecording());

    canvas.clear(0);
    canvas.drawPicture(picture3);
    pixel = *bitmap.getAddr32(0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
}

static void test_huge_blur(SkBaseDevice* device, skiatest::Reporter* reporter) {
    SkCanvas canvas(device);

    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    bitmap.eraseARGB(0, 0, 0, 0);

    // Check that a blur with an insane radius does not crash or assert.
    SkAutoTUnref<SkImageFilter> blur(SkBlurImageFilter::Create(SkIntToScalar(1<<30), SkIntToScalar(1<<30)));

    SkPaint paint;
    paint.setImageFilter(blur);
    canvas.drawSprite(bitmap, 0, 0, &paint);
}

DEF_TEST(HugeBlurImageFilter, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(100, 100);
    SkBitmapDevice device(temp);
    test_huge_blur(&device, reporter);
}

static void test_xfermode_cropped_input(SkBaseDevice* device, skiatest::Reporter* reporter) {
    SkCanvas canvas(device);
    canvas.clear(0);

    SkBitmap bitmap;
    bitmap.allocN32Pixels(1, 1);
    bitmap.eraseARGB(255, 255, 255, 255);

    SkAutoTUnref<SkColorFilter> green(
        SkColorFilter::CreateModeFilter(SK_ColorGREEN, SkXfermode::kSrcIn_Mode));
    SkAutoTUnref<SkColorFilterImageFilter> greenFilter(
        SkColorFilterImageFilter::Create(green.get()));
    SkImageFilter::CropRect cropRect(SkRect::MakeEmpty());
    SkAutoTUnref<SkColorFilterImageFilter> croppedOut(
        SkColorFilterImageFilter::Create(green.get(), NULL, &cropRect));

    // Check that an xfermode image filter whose input has been cropped out still draws the other
    // input. Also check that drawing with both inputs cropped out doesn't cause a GPU warning.
    SkXfermode* mode = SkXfermode::Create(SkXfermode::kSrcOver_Mode);
    SkAutoTUnref<SkImageFilter> xfermodeNoFg(
        SkXfermodeImageFilter::Create(mode, greenFilter, croppedOut));
    SkAutoTUnref<SkImageFilter> xfermodeNoBg(
        SkXfermodeImageFilter::Create(mode, croppedOut, greenFilter));
    SkAutoTUnref<SkImageFilter> xfermodeNoFgNoBg(
        SkXfermodeImageFilter::Create(mode, croppedOut, croppedOut));

    SkPaint paint;
    paint.setImageFilter(xfermodeNoFg);
    canvas.drawSprite(bitmap, 0, 0, &paint);

    uint32_t pixel;
    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    canvas.readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    paint.setImageFilter(xfermodeNoBg);
    canvas.drawSprite(bitmap, 0, 0, &paint);
    canvas.readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    paint.setImageFilter(xfermodeNoFgNoBg);
    canvas.drawSprite(bitmap, 0, 0, &paint);
    canvas.readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
}

DEF_TEST(ImageFilterNestedSaveLayer, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(50, 50);
    SkBitmapDevice device(temp);
    SkCanvas canvas(&device);
    canvas.clear(0x0);

    SkBitmap bitmap;
    bitmap.allocN32Pixels(10, 10);
    bitmap.eraseColor(SK_ColorGREEN);

    SkMatrix matrix;
    matrix.setScale(SkIntToScalar(2), SkIntToScalar(2));
    matrix.postTranslate(SkIntToScalar(-20), SkIntToScalar(-20));
    SkAutoTUnref<SkImageFilter> matrixFilter(
        SkMatrixImageFilter::Create(matrix, SkPaint::kLow_FilterLevel));

    // Test that saveLayer() with a filter nested inside another saveLayer() applies the
    // correct offset to the filter matrix.
    SkRect bounds1 = SkRect::MakeXYWH(10, 10, 30, 30);
    canvas.saveLayer(&bounds1, NULL);
    SkPaint filterPaint;
    filterPaint.setImageFilter(matrixFilter);
    SkRect bounds2 = SkRect::MakeXYWH(20, 20, 10, 10);
    canvas.saveLayer(&bounds2, &filterPaint);
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    canvas.drawRect(bounds2, greenPaint);
    canvas.restore();
    canvas.restore();
    SkPaint strokePaint;
    strokePaint.setStyle(SkPaint::kStroke_Style);
    strokePaint.setColor(SK_ColorRED);

    SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    uint32_t pixel;
    canvas.readPixels(info, &pixel, 4, 25, 25);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    // Test that drawSprite() with a filter nested inside a saveLayer() applies the
    // correct offset to the filter matrix.
    canvas.clear(0x0);
    canvas.readPixels(info, &pixel, 4, 25, 25);
    canvas.saveLayer(&bounds1, NULL);
    canvas.drawSprite(bitmap, 20, 20, &filterPaint);
    canvas.restore();

    canvas.readPixels(info, &pixel, 4, 25, 25);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
}

DEF_TEST(XfermodeImageFilterCroppedInput, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(100, 100);
    SkBitmapDevice device(temp);
    test_xfermode_cropped_input(&device, reporter);
}

#if SK_SUPPORT_GPU
DEF_GPUTEST(ImageFilterCropRectGPU, reporter, factory) {
    GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(0));
    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(context,
                                                         SkImageInfo::MakeN32Premul(100, 100),
                                                         0));
    test_crop_rects(device, reporter);
}

DEF_GPUTEST(HugeBlurImageFilterGPU, reporter, factory) {
    GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(0));
    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(context,
                                                         SkImageInfo::MakeN32Premul(100, 100),
                                                         0));
    test_huge_blur(device, reporter);
}

DEF_GPUTEST(XfermodeImageFilterCroppedInputGPU, reporter, factory) {
    GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(0));
    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(context,
                                                         SkImageInfo::MakeN32Premul(1, 1),
                                                         0));
    test_xfermode_cropped_input(device, reporter);
}
#endif
