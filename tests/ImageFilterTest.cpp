/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapDevice.h"
#include "SkBitmapSource.h"
#include "SkBlurImageFilter.h"
#include "SkCanvas.h"
#include "SkColorFilterImageFilter.h"
#include "SkColorMatrixFilter.h"
#include "SkComposeImageFilter.h"
#include "SkDisplacementMapEffect.h"
#include "SkDropShadowImageFilter.h"
#include "SkFlattenableSerialization.h"
#include "SkGradientShader.h"
#include "SkLightingImageFilter.h"
#include "SkMatrixConvolutionImageFilter.h"
#include "SkMergeImageFilter.h"
#include "SkMorphologyImageFilter.h"
#include "SkOffsetImageFilter.h"
#include "SkPerlinNoiseShader.h"
#include "SkPicture.h"
#include "SkPictureImageFilter.h"
#include "SkPictureRecorder.h"
#include "SkPoint3.h"
#include "SkReadBuffer.h"
#include "SkRect.h"
#include "SkRectShaderImageFilter.h"
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
      : SkImageFilter(0, NULL), fReporter(reporter), fExpectedMatrix(expectedMatrix) {
    }

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context& ctx,
                               SkBitmap* result, SkIPoint* offset) const override {
        REPORTER_ASSERT(fReporter, ctx.ctm() == fExpectedMatrix);
        return true;
    }

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(MatrixTestImageFilter)

protected:
    void flatten(SkWriteBuffer& buffer) const override {
        this->INHERITED::flatten(buffer);
        buffer.writeFunctionPtr(fReporter);
        buffer.writeMatrix(fExpectedMatrix);
    }

private:
    skiatest::Reporter* fReporter;
    SkMatrix fExpectedMatrix;

    typedef SkImageFilter INHERITED;
};

}

SkFlattenable* MatrixTestImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    skiatest::Reporter* reporter = (skiatest::Reporter*)buffer.readFunctionPtr();
    SkMatrix matrix;
    buffer.readMatrix(&matrix);
    return SkNEW_ARGS(MatrixTestImageFilter, (reporter, matrix));
}

#ifndef SK_IGNORE_TO_STRING
void MatrixTestImageFilter::toString(SkString* str) const {
    str->appendf("MatrixTestImageFilter: (");
    str->append(")");
}
#endif

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

static SkImageFilter* make_grayscale(SkImageFilter* input, const SkImageFilter::CropRect* cropRect) {
    SkScalar matrix[20];
    memset(matrix, 0, 20 * sizeof(SkScalar));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    SkAutoTUnref<SkColorFilter> filter(SkColorMatrixFilter::Create(matrix));
    return SkColorFilterImageFilter::Create(filter, input, cropRect);
}

static SkImageFilter* make_blue(SkImageFilter* input, const SkImageFilter::CropRect* cropRect) {
    SkAutoTUnref<SkColorFilter> filter(SkColorFilter::CreateModeFilter(SK_ColorBLUE,
                                                                       SkXfermode::kSrcIn_Mode));
    return SkColorFilterImageFilter::Create(filter, input, cropRect);
}

DEF_TEST(ImageFilter, reporter) {
    {
        // Check that two non-clipping color-matrice-filters concatenate into a single filter.
        SkAutoTUnref<SkImageFilter> halfBrightness(make_scale(0.5f));
        SkAutoTUnref<SkImageFilter> quarterBrightness(make_scale(0.5f, halfBrightness));
        REPORTER_ASSERT(reporter, NULL == quarterBrightness->getInput(0));
        SkColorFilter* cf;
        REPORTER_ASSERT(reporter, quarterBrightness->asColorFilter(&cf));
        REPORTER_ASSERT(reporter, cf->asColorMatrix(NULL));
        cf->unref();
    }

    {
        // Check that a clipping color-matrice-filter followed by a color-matrice-filters
        // concatenates into a single filter, but not a matrixfilter (due to clamping).
        SkAutoTUnref<SkImageFilter> doubleBrightness(make_scale(2.0f));
        SkAutoTUnref<SkImageFilter> halfBrightness(make_scale(0.5f, doubleBrightness));
        REPORTER_ASSERT(reporter, NULL == halfBrightness->getInput(0));
        SkColorFilter* cf;
        REPORTER_ASSERT(reporter, halfBrightness->asColorFilter(&cf));
        REPORTER_ASSERT(reporter, !cf->asColorMatrix(NULL));
        cf->unref();
    }

    {
        // Check that a color filter image filter without a crop rect can be
        // expressed as a color filter.
        SkAutoTUnref<SkImageFilter> gray(make_grayscale(NULL, NULL));
        REPORTER_ASSERT(reporter, true == gray->asColorFilter(NULL));
    }
    
    {
        // Check that a colorfilterimage filter without a crop rect but with an input
        // that is another colorfilterimage can be expressed as a colorfilter (composed).
        SkAutoTUnref<SkImageFilter> mode(make_blue(NULL, NULL));
        SkAutoTUnref<SkImageFilter> gray(make_grayscale(mode, NULL));
        REPORTER_ASSERT(reporter, true == gray->asColorFilter(NULL));
    }

    {
        // Test that if we exceed the limit of what ComposeColorFilter can combine, we still
        // can build the DAG and won't assert if we call asColorFilter.
        SkAutoTUnref<SkImageFilter> filter(make_blue(NULL, NULL));
        const int kWayTooManyForComposeColorFilter = 100;
        for (int i = 0; i < kWayTooManyForComposeColorFilter; ++i) {
            filter.reset(make_blue(filter, NULL));
            // the first few of these will succeed, but after we hit the internal limit,
            // it will then return false.
            (void)filter->asColorFilter(NULL);
        }
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
            SkPoint3 location = SkPoint3::Make(0, 0, SK_Scalar1);
            // 2 ) location and target at same value
            SkPoint3 target = SkPoint3::Make(location.fX, location.fY, location.fZ);
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
    }
}

static void test_crop_rects(SkImageFilter::Proxy* proxy, skiatest::Reporter* reporter) {
    // Check that all filters offset to their absolute crop rect,
    // unaffected by the input crop rect.
    // Tests pass by not asserting.
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    bitmap.eraseARGB(0, 0, 0, 0);

    SkImageFilter::CropRect inputCropRect(SkRect::MakeXYWH(8, 13, 80, 80));
    SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(20, 30, 60, 60));
    SkAutoTUnref<SkImageFilter> input(make_grayscale(NULL, &inputCropRect));

    SkAutoTUnref<SkColorFilter> cf(SkColorFilter::CreateModeFilter(SK_ColorRED, SkXfermode::kSrcIn_Mode));
    SkPoint3 location = SkPoint3::Make(0, 0, SK_Scalar1);
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
        SkDropShadowImageFilter::Create(SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1,
            SK_ColorGREEN, SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
            input.get(), &cropRect),
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
        SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeLargest(), NULL);
        REPORTER_ASSERT_MESSAGE(reporter, filter->filterImage(proxy, bitmap, ctx,
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

static void test_negative_blur_sigma(SkImageFilter::Proxy* proxy, skiatest::Reporter* reporter) {
    // Check that SkBlurImageFilter will accept a negative sigma, either in
    // the given arguments or after CTM application.
    int width = 32, height = 32;
    SkScalar five = SkIntToScalar(5);

    SkAutoTUnref<SkBlurImageFilter> positiveFilter(
        SkBlurImageFilter::Create(five, five)
    );

    SkAutoTUnref<SkBlurImageFilter> negativeFilter(
        SkBlurImageFilter::Create(-five, five)
    );

    SkBitmap gradient = make_gradient_circle(width, height);
    SkBitmap positiveResult1, negativeResult1;
    SkBitmap positiveResult2, negativeResult2;
    SkIPoint offset;
    SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeLargest(), NULL);
    positiveFilter->filterImage(proxy, gradient, ctx, &positiveResult1, &offset);
    negativeFilter->filterImage(proxy, gradient, ctx, &negativeResult1, &offset);
    SkMatrix negativeScale;
    negativeScale.setScale(-SK_Scalar1, SK_Scalar1);
    SkImageFilter::Context negativeCTX(negativeScale, SkIRect::MakeLargest(), NULL);
    positiveFilter->filterImage(proxy, gradient, negativeCTX, &negativeResult2, &offset);
    negativeFilter->filterImage(proxy, gradient, negativeCTX, &positiveResult2, &offset);
    SkAutoLockPixels lockP1(positiveResult1);
    SkAutoLockPixels lockP2(positiveResult2);
    SkAutoLockPixels lockN1(negativeResult1);
    SkAutoLockPixels lockN2(negativeResult2);
    for (int y = 0; y < height; y++) {
        int diffs = memcmp(positiveResult1.getAddr32(0, y), negativeResult1.getAddr32(0, y), positiveResult1.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
        diffs = memcmp(positiveResult1.getAddr32(0, y), negativeResult2.getAddr32(0, y), positiveResult1.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
        diffs = memcmp(positiveResult1.getAddr32(0, y), positiveResult2.getAddr32(0, y), positiveResult1.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
    }
}

DEF_TEST(TestNegativeBlurSigma, reporter) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    SkAutoTUnref<SkBaseDevice> device(SkBitmapDevice::Create(info, props));
    SkImageFilter::Proxy proxy(device);

    test_negative_blur_sigma(&proxy, reporter);
}

DEF_TEST(ImageFilterDrawTiled, reporter) {
    // Check that all filters when drawn tiled (with subsequent clip rects) exactly
    // match the same filters drawn with a single full-canvas bitmap draw.
    // Tests pass by not asserting.

    SkAutoTUnref<SkColorFilter> cf(SkColorFilter::CreateModeFilter(SK_ColorRED, SkXfermode::kSrcIn_Mode));
    SkPoint3 location = SkPoint3::Make(0, 0, SK_Scalar1);
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
    SkAutoTUnref<SkShader> shader(SkPerlinNoiseShader::CreateTurbulence(SK_Scalar1, SK_Scalar1, 1, 0));

    SkAutoTUnref<SkImageFilter> rectShaderFilter(SkRectShaderImageFilter::Create(shader.get()));

    SkAutoTUnref<SkShader> greenColorShader(SkShader::CreateColorShader(SK_ColorGREEN));
    SkImageFilter::CropRect leftSideCropRect(SkRect::MakeXYWH(0, 0, 32, 64));
    SkAutoTUnref<SkImageFilter> rectShaderFilterLeft(SkRectShaderImageFilter::Create(greenColorShader.get(), &leftSideCropRect));
    SkImageFilter::CropRect rightSideCropRect(SkRect::MakeXYWH(32, 0, 32, 64));
    SkAutoTUnref<SkImageFilter> rectShaderFilterRight(SkRectShaderImageFilter::Create(greenColorShader.get(), &rightSideCropRect));

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
              SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_ColorGREEN,
              SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode) },
        { "diffuse lighting", SkLightingImageFilter::CreatePointLitDiffuse(
              location, SK_ColorGREEN, 0, 0) },
        { "specular lighting",
              SkLightingImageFilter::CreatePointLitSpecular(location, SK_ColorGREEN, 0, 0, 0) },
        { "matrix convolution",
              SkMatrixConvolutionImageFilter::Create(
                  kernelSize, kernel, gain, bias, SkIPoint::Make(1, 1),
                  SkMatrixConvolutionImageFilter::kRepeat_TileMode, false) },
        { "merge", SkMergeImageFilter::Create(NULL, NULL, SkXfermode::kSrcOver_Mode) },
        { "merge with disjoint inputs", SkMergeImageFilter::Create(
              rectShaderFilterLeft, rectShaderFilterRight, SkXfermode::kSrcOver_Mode) },
        { "offset", SkOffsetImageFilter::Create(SK_Scalar1, SK_Scalar1) },
        { "dilate", SkDilateImageFilter::Create(3, 2) },
        { "erode", SkErodeImageFilter::Create(2, 3) },
        { "tile", SkTileImageFilter::Create(SkRect::MakeXYWH(0, 0, 50, 50),
                                            SkRect::MakeXYWH(0, 0, 100, 100), NULL) },
        { "matrix", SkImageFilter::CreateMatrixFilter(matrix, kLow_SkFilterQuality) },
        { "blur and offset", SkOffsetImageFilter::Create(five, five, blur.get()) },
        { "picture and blur", SkBlurImageFilter::Create(five, five, pictureFilter.get()) },
        { "rect shader and blur", SkBlurImageFilter::Create(five, five, rectShaderFilter.get()) },
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

static void draw_saveLayer_picture(int width, int height, int tileSize,
                                   SkBBHFactory* factory, SkBitmap* result) {

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(50), 0);

    SkAutoTUnref<SkColorFilter> cf(SkColorFilter::CreateModeFilter(SK_ColorWHITE, SkXfermode::kSrc_Mode));
    SkAutoTUnref<SkImageFilter> cfif(SkColorFilterImageFilter::Create(cf.get()));
    SkAutoTUnref<SkImageFilter> imageFilter(SkImageFilter::CreateMatrixFilter(matrix, kNone_SkFilterQuality, cfif.get()));

    SkPaint paint;
    paint.setImageFilter(imageFilter.get());
    SkPictureRecorder recorder;
    SkRect bounds = SkRect::Make(SkIRect::MakeXYWH(0, 0, 50, 50));
    SkCanvas* recordingCanvas = recorder.beginRecording(SkIntToScalar(width),
                                                        SkIntToScalar(height),
                                                        factory, 0);
    recordingCanvas->translate(-55, 0);
    recordingCanvas->saveLayer(&bounds, &paint);
    recordingCanvas->restore();
    SkAutoTUnref<SkPicture> picture1(recorder.endRecording());

    result->allocN32Pixels(width, height);
    SkCanvas canvas(*result);
    canvas.clear(0);
    canvas.clipRect(SkRect::Make(SkIRect::MakeWH(tileSize, tileSize)));
    canvas.drawPicture(picture1.get());
}

DEF_TEST(ImageFilterDrawMatrixBBH, reporter) {
    // Check that matrix filter when drawn tiled with BBH exactly
    // matches the same thing drawn without BBH.
    // Tests pass by not asserting.

    const int width = 200, height = 200;
    const int tileSize = 100;
    SkBitmap result1, result2;
    SkRTreeFactory factory;

    draw_saveLayer_picture(width, height, tileSize, &factory, &result1);
    draw_saveLayer_picture(width, height, tileSize, NULL, &result2);

    for (int y = 0; y < height; y++) {
        int diffs = memcmp(result1.getAddr32(0, y), result2.getAddr32(0, y), result1.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
    }
}

static SkImageFilter* makeBlur(SkImageFilter* input = NULL) {
    return SkBlurImageFilter::Create(SK_Scalar1, SK_Scalar1, input);
}

static SkImageFilter* makeDropShadow(SkImageFilter* input = NULL) {
    return SkDropShadowImageFilter::Create(
        SkIntToScalar(100), SkIntToScalar(100),
        SkIntToScalar(10), SkIntToScalar(10),
        SK_ColorBLUE, SkDropShadowImageFilter::kDrawShadowAndForeground_ShadowMode,
        input, NULL);
}

DEF_TEST(ImageFilterBlurThenShadowBounds, reporter) {
    SkAutoTUnref<SkImageFilter> filter1(makeBlur());
    SkAutoTUnref<SkImageFilter> filter2(makeDropShadow(filter1.get()));

    SkIRect bounds = SkIRect::MakeXYWH(0, 0, 100, 100);
    SkIRect expectedBounds = SkIRect::MakeXYWH(-133, -133, 236, 236);
    filter2->filterBounds(bounds, SkMatrix::I(), &bounds);

    REPORTER_ASSERT(reporter, bounds == expectedBounds);
}

DEF_TEST(ImageFilterShadowThenBlurBounds, reporter) {
    SkAutoTUnref<SkImageFilter> filter1(makeDropShadow());
    SkAutoTUnref<SkImageFilter> filter2(makeBlur(filter1.get()));

    SkIRect bounds = SkIRect::MakeXYWH(0, 0, 100, 100);
    SkIRect expectedBounds = SkIRect::MakeXYWH(-133, -133, 236, 236);
    filter2->filterBounds(bounds, SkMatrix::I(), &bounds);

    REPORTER_ASSERT(reporter, bounds == expectedBounds);
}

DEF_TEST(ImageFilterDilateThenBlurBounds, reporter) {
    SkAutoTUnref<SkImageFilter> filter1(SkDilateImageFilter::Create(2, 2));
    SkAutoTUnref<SkImageFilter> filter2(makeDropShadow(filter1.get()));

    SkIRect bounds = SkIRect::MakeXYWH(0, 0, 100, 100);
    SkIRect expectedBounds = SkIRect::MakeXYWH(-132, -132, 234, 234);
    filter2->filterBounds(bounds, SkMatrix::I(), &bounds);

    REPORTER_ASSERT(reporter, bounds == expectedBounds);
}

DEF_TEST(ImageFilterComposedBlurFastBounds, reporter) {
    SkAutoTUnref<SkImageFilter> filter1(makeBlur());
    SkAutoTUnref<SkImageFilter> filter2(makeBlur());
    SkAutoTUnref<SkImageFilter> composedFilter(SkComposeImageFilter::Create(filter1.get(), filter2.get()));

    SkRect boundsSrc = SkRect::MakeWH(SkIntToScalar(100), SkIntToScalar(100));
    SkRect expectedBounds = SkRect::MakeXYWH(
        SkIntToScalar(-6), SkIntToScalar(-6), SkIntToScalar(112), SkIntToScalar(112));
    SkRect boundsDst = SkRect::MakeEmpty();
    composedFilter->computeFastBounds(boundsSrc, &boundsDst);

    REPORTER_ASSERT(reporter, boundsDst == expectedBounds);
}

static void draw_blurred_rect(SkCanvas* canvas) {
    SkAutoTUnref<SkImageFilter> filter(SkBlurImageFilter::Create(SkIntToScalar(8), 0));
    SkPaint filterPaint;
    filterPaint.setColor(SK_ColorWHITE);
    filterPaint.setImageFilter(filter);
    canvas->saveLayer(NULL, &filterPaint);
    SkPaint whitePaint;
    whitePaint.setColor(SK_ColorWHITE);
    canvas->drawRect(SkRect::Make(SkIRect::MakeWH(4, 4)), whitePaint);
    canvas->restore();
}

static void draw_picture_clipped(SkCanvas* canvas, const SkRect& clipRect, const SkPicture* picture) {
    canvas->save();
    canvas->clipRect(clipRect);
    canvas->drawPicture(picture);
    canvas->restore();
}

DEF_TEST(ImageFilterDrawTiledBlurRTree, reporter) {
    // Check that the blur filter when recorded with RTree acceleration,
    // and drawn tiled (with subsequent clip rects) exactly
    // matches the same filter drawn with without RTree acceleration.
    // This tests that the "bleed" from the blur into the otherwise-blank
    // tiles is correctly rendered.
    // Tests pass by not asserting.

    int width = 16, height = 8;
    SkBitmap result1, result2;
    result1.allocN32Pixels(width, height);
    result2.allocN32Pixels(width, height);
    SkCanvas canvas1(result1);
    SkCanvas canvas2(result2);
    int tileSize = 8;

    canvas1.clear(0);
    canvas2.clear(0);

    SkRTreeFactory factory;

    SkPictureRecorder recorder1, recorder2;
    // The only difference between these two pictures is that one has RTree aceleration.
    SkCanvas* recordingCanvas1 = recorder1.beginRecording(SkIntToScalar(width),
                                                          SkIntToScalar(height),
                                                          NULL, 0);
    SkCanvas* recordingCanvas2 = recorder2.beginRecording(SkIntToScalar(width),
                                                          SkIntToScalar(height),
                                                          &factory, 0);
    draw_blurred_rect(recordingCanvas1);
    draw_blurred_rect(recordingCanvas2);
    SkAutoTUnref<SkPicture> picture1(recorder1.endRecording());
    SkAutoTUnref<SkPicture> picture2(recorder2.endRecording());
    for (int y = 0; y < height; y += tileSize) {
        for (int x = 0; x < width; x += tileSize) {
            SkRect tileRect = SkRect::Make(SkIRect::MakeXYWH(x, y, tileSize, tileSize));
            draw_picture_clipped(&canvas1, tileRect, picture1);
            draw_picture_clipped(&canvas2, tileRect, picture2);
        }
    }
    for (int y = 0; y < height; y++) {
        int diffs = memcmp(result1.getAddr32(0, y), result2.getAddr32(0, y), result1.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
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
    const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    SkAutoTUnref<SkBaseDevice> device(SkBitmapDevice::Create(info, props));
    SkImageFilter::Proxy proxy(device);

    test_crop_rects(&proxy, reporter);
}

DEF_TEST(ImageFilterMatrix, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(100, 100);
    SkCanvas canvas(temp);
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

DEF_TEST(ImageFilterCrossProcessPictureImageFilter, reporter) {
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
    SkCanvas canvas(bitmap);

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
    // If the security precautions are enabled, the result here should not be green, since the
    // filter draws nothing.
    REPORTER_ASSERT(reporter, SkPicture::PictureIOSecurityPrecautionsEnabled() 
        ? pixel != SK_ColorGREEN : pixel == SK_ColorGREEN);
}

DEF_TEST(ImageFilterClippedPictureImageFilter, reporter) {
    SkRTreeFactory factory;
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(1, 1, &factory, 0);

    // Create an SkPicture which simply draws a green 1x1 rectangle.
    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(1, 1)), greenPaint);
    SkAutoTUnref<SkPicture> picture(recorder.endRecording());

    SkAutoTUnref<SkImageFilter> imageFilter(SkPictureImageFilter::Create(picture.get()));

    SkBitmap result;
    SkIPoint offset;
    SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeXYWH(1, 1, 1, 1), NULL);
    SkBitmap bitmap;
    bitmap.allocN32Pixels(2, 2);
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkBitmapDevice device(bitmap, props);
    SkImageFilter::Proxy proxy(&device);
    REPORTER_ASSERT(reporter, !imageFilter->filterImage(&proxy, bitmap, ctx, &result, &offset));
}

DEF_TEST(ImageFilterEmptySaveLayer, reporter) {
    // Even when there's an empty saveLayer()/restore(), ensure that an image
    // filter or color filter which affects transparent black still draws.

    SkBitmap bitmap;
    bitmap.allocN32Pixels(10, 10);
    SkCanvas canvas(bitmap);

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

static void test_huge_blur(SkCanvas* canvas, skiatest::Reporter* reporter) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    bitmap.eraseARGB(0, 0, 0, 0);

    // Check that a blur with an insane radius does not crash or assert.
    SkAutoTUnref<SkImageFilter> blur(SkBlurImageFilter::Create(SkIntToScalar(1<<30), SkIntToScalar(1<<30)));

    SkPaint paint;
    paint.setImageFilter(blur);
    canvas->drawSprite(bitmap, 0, 0, &paint);
}

DEF_TEST(HugeBlurImageFilter, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(100, 100);
    SkCanvas canvas(temp);
    test_huge_blur(&canvas, reporter);
}

DEF_TEST(MatrixConvolutionSanityTest, reporter) {
    SkScalar kernel[1] = { 0 };
    SkScalar gain = SK_Scalar1, bias = 0;
    SkIPoint kernelOffset = SkIPoint::Make(1, 1);

    // Check that an enormous (non-allocatable) kernel gives a NULL filter.
    SkAutoTUnref<SkImageFilter> conv(SkMatrixConvolutionImageFilter::Create(
        SkISize::Make(1<<30, 1<<30),
        kernel,
        gain,
        bias,
        kernelOffset,
        SkMatrixConvolutionImageFilter::kRepeat_TileMode,
        false));

    REPORTER_ASSERT(reporter, NULL == conv.get());

    // Check that a NULL kernel gives a NULL filter.
    conv.reset(SkMatrixConvolutionImageFilter::Create(
        SkISize::Make(1, 1),
        NULL,
        gain,
        bias,
        kernelOffset,
        SkMatrixConvolutionImageFilter::kRepeat_TileMode,
        false));

    REPORTER_ASSERT(reporter, NULL == conv.get());

    // Check that a kernel width < 1 gives a NULL filter.
    conv.reset(SkMatrixConvolutionImageFilter::Create(
        SkISize::Make(0, 1),
        kernel,
        gain,
        bias,
        kernelOffset,
        SkMatrixConvolutionImageFilter::kRepeat_TileMode,
        false));

    REPORTER_ASSERT(reporter, NULL == conv.get());

    // Check that kernel height < 1 gives a NULL filter.
    conv.reset(SkMatrixConvolutionImageFilter::Create(
        SkISize::Make(1, -1),
        kernel,
        gain,
        bias,
        kernelOffset,
        SkMatrixConvolutionImageFilter::kRepeat_TileMode,
        false));

    REPORTER_ASSERT(reporter, NULL == conv.get());
}

static void test_xfermode_cropped_input(SkCanvas* canvas, skiatest::Reporter* reporter) {
    canvas->clear(0);

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
    canvas->drawSprite(bitmap, 0, 0, &paint);

    uint32_t pixel;
    SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType);
    canvas->readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    paint.setImageFilter(xfermodeNoBg);
    canvas->drawSprite(bitmap, 0, 0, &paint);
    canvas->readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    paint.setImageFilter(xfermodeNoFgNoBg);
    canvas->drawSprite(bitmap, 0, 0, &paint);
    canvas->readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
}

DEF_TEST(ImageFilterNestedSaveLayer, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(50, 50);
    SkCanvas canvas(temp);
    canvas.clear(0x0);

    SkBitmap bitmap;
    bitmap.allocN32Pixels(10, 10);
    bitmap.eraseColor(SK_ColorGREEN);

    SkMatrix matrix;
    matrix.setScale(SkIntToScalar(2), SkIntToScalar(2));
    matrix.postTranslate(SkIntToScalar(-20), SkIntToScalar(-20));
    SkAutoTUnref<SkImageFilter> matrixFilter(
        SkImageFilter::CreateMatrixFilter(matrix, kLow_SkFilterQuality));

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

    SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType);
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
    SkCanvas canvas(temp);
    test_xfermode_cropped_input(&canvas, reporter);
}

DEF_TEST(ComposedImageFilterOffset, reporter) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    bitmap.eraseARGB(0, 0, 0, 0);
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkBitmapDevice device(bitmap, props);
    SkImageFilter::Proxy proxy(&device);

    SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(1, 0, 20, 20));
    SkAutoTUnref<SkImageFilter> offsetFilter(SkOffsetImageFilter::Create(0, 0, NULL, &cropRect));
    SkAutoTUnref<SkImageFilter> blurFilter(makeBlur());
    SkAutoTUnref<SkImageFilter> composedFilter(SkComposeImageFilter::Create(blurFilter, offsetFilter.get()));
    SkBitmap result;
    SkIPoint offset;
    SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeLargest(), NULL);
    REPORTER_ASSERT(reporter, composedFilter->filterImage(&proxy, bitmap, ctx, &result, &offset));
    REPORTER_ASSERT(reporter, offset.fX == 1 && offset.fY == 0);
}

DEF_TEST(PartialCropRect, reporter) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    bitmap.eraseARGB(0, 0, 0, 0);
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
    SkBitmapDevice device(bitmap, props);
    SkImageFilter::Proxy proxy(&device);

    SkImageFilter::CropRect cropRect(SkRect::MakeXYWH(100, 0, 20, 30),
        SkImageFilter::CropRect::kHasWidth_CropEdge | SkImageFilter::CropRect::kHasHeight_CropEdge);
    SkAutoTUnref<SkImageFilter> filter(make_grayscale(NULL, &cropRect));
    SkBitmap result;
    SkIPoint offset;
    SkImageFilter::Context ctx(SkMatrix::I(), SkIRect::MakeLargest(), NULL);
    REPORTER_ASSERT(reporter, filter->filterImage(&proxy, bitmap, ctx, &result, &offset));
    REPORTER_ASSERT(reporter, offset.fX == 0);
    REPORTER_ASSERT(reporter, offset.fY == 0);
    REPORTER_ASSERT(reporter, result.width() == 20);
    REPORTER_ASSERT(reporter, result.height() == 30);
}

#if SK_SUPPORT_GPU

DEF_GPUTEST(ImageFilterCropRectGPU, reporter, factory) {
    GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(0));
    if (NULL == context) {
        return;
    }
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(context,
                                                         SkSurface::kNo_Budgeted,
                                                         SkImageInfo::MakeN32Premul(100, 100),
                                                         0,
                                                         &props,
                                                         SkGpuDevice::kUninit_InitContents));
    SkImageFilter::Proxy proxy(device);

    test_crop_rects(&proxy, reporter);
}

DEF_GPUTEST(HugeBlurImageFilterGPU, reporter, factory) {
    GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(0));
    if (NULL == context) {
        return;
    }
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(context,
                                                         SkSurface::kNo_Budgeted,
                                                         SkImageInfo::MakeN32Premul(100, 100),
                                                         0,
                                                         &props,
                                                         SkGpuDevice::kUninit_InitContents));
    SkCanvas canvas(device);

    test_huge_blur(&canvas, reporter);
}

DEF_GPUTEST(XfermodeImageFilterCroppedInputGPU, reporter, factory) {
    GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(0));
    if (NULL == context) {
        return;
    }
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(context,
                                                         SkSurface::kNo_Budgeted,
                                                         SkImageInfo::MakeN32Premul(1, 1),
                                                         0,
                                                         &props,
                                                         SkGpuDevice::kUninit_InitContents));
    SkCanvas canvas(device);

    test_xfermode_cropped_input(&canvas, reporter);
}

DEF_GPUTEST(TestNegativeBlurSigmaGPU, reporter, factory) {
    GrContext* context = factory->get(static_cast<GrContextFactory::GLContextType>(0));
    if (NULL == context) {
        return;
    }
    const SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    SkAutoTUnref<SkGpuDevice> device(SkGpuDevice::Create(context,
                                                         SkSurface::kNo_Budgeted,
                                                         SkImageInfo::MakeN32Premul(1, 1),
                                                         0,
                                                         &props,
                                                         SkGpuDevice::kUninit_InitContents));
    SkImageFilter::Proxy proxy(device);

    test_negative_blur_sigma(&proxy, reporter);
}
#endif
