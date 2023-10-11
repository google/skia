/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkAlphaType.h"
#include "include/core/SkBBHFactory.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSerialProcs.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceProps.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkPerlinNoiseShader.h"
#include "include/encode/SkPngEncoder.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkBitmapDevice.h"
#include "src/core/SkDevice.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkSpecialImage.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/image/SkImage_Base.h"
#include "tests/CtsEnforcement.h"
#include "tests/Test.h"
#include "tools/EncodeUtils.h"
#include "tools/GpuToolUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#if defined(SK_GANESH)
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/gpu/ganesh/image/SkImage_GaneshBase.h"
#include "src/gpu/ganesh/image/SkSpecialImage_Ganesh.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Surface.h"
#endif

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <utility>
#include <limits>

using namespace skia_private;

class SkReadBuffer;
class SkWriteBuffer;
struct GrContextOptions;

static const int kBitmapSize = 4;

namespace {

static constexpr GrSurfaceOrigin kTestSurfaceOrigin = kTopLeft_GrSurfaceOrigin;

class MatrixTestImageFilter : public SkImageFilter_Base {
public:
    MatrixTestImageFilter(skiatest::Reporter* reporter, const SkMatrix& expectedMatrix)
            : SkImageFilter_Base(nullptr, 0)
            , fReporter(reporter)
            , fExpectedMatrix(expectedMatrix) {}

private:
    Factory getFactory() const override {
        SK_ABORT("Does not participate in serialization");
        return nullptr;
    }
    const char* getTypeName() const override { return "MatrixTestImageFilter"; }

    skif::FilterResult onFilterImage(const skif::Context& ctx) const override {
        REPORTER_ASSERT(fReporter, ctx.mapping().layerMatrix() == fExpectedMatrix);
        return ctx.source();
    }

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override {
        return desiredOutput;
    }

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override {
        return contentBounds;
    }

    skiatest::Reporter* fReporter;
    SkMatrix fExpectedMatrix;
};

void draw_gradient_circle(SkCanvas* canvas, int width, int height) {
    SkScalar x = SkIntToScalar(width / 2);
    SkScalar y = SkIntToScalar(height / 2);
    SkScalar radius = std::min(x, y) * 0.8f;
    canvas->clear(0x00000000);
    SkColor colors[2];
    colors[0] = SK_ColorWHITE;
    colors[1] = SK_ColorBLACK;
    sk_sp<SkShader> shader(
        SkGradientShader::MakeRadial(SkPoint::Make(x, y), radius, colors, nullptr, 2,
                                       SkTileMode::kClamp)
    );
    SkPaint paint;
    paint.setShader(shader);
    canvas->drawCircle(x, y, radius, paint);
}

SkBitmap make_gradient_circle(int width, int height) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(width, height);
    SkCanvas canvas(bitmap);
    draw_gradient_circle(&canvas, width, height);
    return bitmap;
}

class FilterList {
public:
    FilterList(sk_sp<SkImageFilter> input, const SkIRect* cropRect = nullptr) {
        static const SkScalar kBlurSigma = SkIntToScalar(5);

        SkPoint3 location = SkPoint3::Make(0, 0, SK_Scalar1);
        {
            sk_sp<SkColorFilter> cf(SkColorFilters::Blend(SK_ColorRED, SkBlendMode::kSrcIn));

            this->addFilter("color filter",
                    SkImageFilters::ColorFilter(std::move(cf), input, cropRect));
        }
        {
            sk_sp<SkImage> gradientImage(make_gradient_circle(64, 64).asImage());
            sk_sp<SkImageFilter> gradientSource(SkImageFilters::Image(std::move(gradientImage),
                                                                      SkFilterMode::kNearest));

            this->addFilter("displacement map",
                    SkImageFilters::DisplacementMap(SkColorChannel::kR, SkColorChannel::kB, 20.0f,
                                                    std::move(gradientSource), input, cropRect));
        }
        this->addFilter("blur", SkImageFilters::Blur(SK_Scalar1, SK_Scalar1, input, cropRect));
        this->addFilter("drop shadow", SkImageFilters::DropShadow(
                SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_Scalar1, SK_ColorGREEN, input, cropRect));
        this->addFilter("diffuse lighting",
                SkImageFilters::PointLitDiffuse(location, SK_ColorGREEN, 0, 0, input, cropRect));
        this->addFilter("specular lighting",
                SkImageFilters::PointLitSpecular(location, SK_ColorGREEN, 0, 0, 0, input,
                                                   cropRect));
        {
            SkScalar kernel[9] = {
                SkIntToScalar(1), SkIntToScalar(1), SkIntToScalar(1),
                SkIntToScalar(1), SkIntToScalar(-7), SkIntToScalar(1),
                SkIntToScalar(1), SkIntToScalar(1), SkIntToScalar(1),
            };
            const SkISize kernelSize = SkISize::Make(3, 3);
            const SkScalar gain = SK_Scalar1, bias = 0;

            // This filter needs a saveLayer bc it is in repeat mode
            this->addFilter("matrix convolution",
                            SkImageFilters::MatrixConvolution(
                                    kernelSize, kernel, gain, bias, SkIPoint::Make(1, 1),
                                    SkTileMode::kRepeat, false, input, cropRect),
                            true);
        }
        this->addFilter("merge", SkImageFilters::Merge(input, input, cropRect));

        {
            sk_sp<SkShader> greenColorShader = SkShaders::Color(SK_ColorGREEN);

            SkIRect leftSideCropRect = SkIRect::MakeXYWH(0, 0, 32, 64);
            sk_sp<SkImageFilter> shaderFilterLeft(SkImageFilters::Shader(greenColorShader,
                                                                         &leftSideCropRect));
            SkIRect rightSideCropRect = SkIRect::MakeXYWH(32, 0, 32, 64);
            sk_sp<SkImageFilter> shaderFilterRight(SkImageFilters::Shader(greenColorShader,
                                                                          &rightSideCropRect));


            this->addFilter("merge with disjoint inputs", SkImageFilters::Merge(
                    std::move(shaderFilterLeft), std::move(shaderFilterRight), cropRect));
        }

        this->addFilter("offset", SkImageFilters::Offset(SK_Scalar1, SK_Scalar1, input, cropRect));
        this->addFilter("dilate", SkImageFilters::Dilate(3, 2, input, cropRect));
        this->addFilter("erode", SkImageFilters::Erode(2, 3, input, cropRect));
        this->addFilter("tile", SkImageFilters::Tile(SkRect::MakeXYWH(0, 0, 50, 50),
                                                     cropRect ? SkRect::Make(*cropRect)
                                                              : SkRect::MakeXYWH(0, 0, 100, 100),
                                                     input));

        if (!cropRect) {
            SkMatrix matrix;

            matrix.setTranslate(SK_Scalar1, SK_Scalar1);
            matrix.postRotate(SkIntToScalar(45), SK_Scalar1, SK_Scalar1);

            this->addFilter("matrix",
                    SkImageFilters::MatrixTransform(matrix,
                                                    SkSamplingOptions(SkFilterMode::kLinear),
                                                    input));
        }
        {
            sk_sp<SkImageFilter> blur(SkImageFilters::Blur(kBlurSigma, kBlurSigma, input));

            this->addFilter("blur and offset", SkImageFilters::Offset(
                    kBlurSigma, kBlurSigma, std::move(blur), cropRect));
        }
        {
            SkPictureRecorder recorder;
            SkCanvas* recordingCanvas = recorder.beginRecording(64, 64);

            SkPaint greenPaint;
            greenPaint.setColor(SK_ColorGREEN);
            recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeXYWH(10, 10, 30, 20)), greenPaint);
            sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
            sk_sp<SkImageFilter> pictureFilter(SkImageFilters::Picture(std::move(picture)));

            this->addFilter("picture and blur", SkImageFilters::Blur(
                    kBlurSigma, kBlurSigma, std::move(pictureFilter), cropRect));
        }
        {
            sk_sp<SkImageFilter> paintFilter(SkImageFilters::Shader(
                    SkShaders::MakeTurbulence(SK_Scalar1, SK_Scalar1, 1, 0)));

            this->addFilter("paint and blur", SkImageFilters::Blur(
                    kBlurSigma, kBlurSigma,  std::move(paintFilter), cropRect));
        }
        this->addFilter("blend", SkImageFilters::Blend(
                SkBlendMode::kSrc, input, input, cropRect));
    }
    int count() const { return fFilters.size(); }
    SkImageFilter* getFilter(int index) const { return fFilters[index].fFilter.get(); }
    const char* getName(int index) const { return fFilters[index].fName; }
    bool needsSaveLayer(int index) const { return fFilters[index].fNeedsSaveLayer; }
private:
    struct Filter {
        Filter() : fName(nullptr), fNeedsSaveLayer(false) {}
        Filter(const char* name, sk_sp<SkImageFilter> filter, bool needsSaveLayer)
            : fName(name)
            , fFilter(std::move(filter))
            , fNeedsSaveLayer(needsSaveLayer) {
        }
        const char*                 fName;
        sk_sp<SkImageFilter>        fFilter;
        bool                        fNeedsSaveLayer;
    };
    void addFilter(const char* name, sk_sp<SkImageFilter> filter, bool needsSaveLayer = false) {
        fFilters.push_back(Filter(name, std::move(filter), needsSaveLayer));
    }

    TArray<Filter> fFilters;
};

}  // namespace

static skif::Context make_context(const SkIRect& out, const SkSpecialImage* src) {
    sk_sp<skif::Backend> backend;
    if (src->isGaneshBacked()) {
        backend = skif::MakeGaneshBackend(sk_ref_sp(src->getContext()), kTestSurfaceOrigin,
                                          src->props(), src->colorType());
    } else {
        backend = skif::MakeRasterBackend(src->props(), src->colorType());
    }

    return skif::Context{std::move(backend),
                         skif::Mapping{SkMatrix::I()},
                         skif::LayerSpace<SkIRect>{out},
                         skif::FilterResult{sk_ref_sp(src)},
                         src->getColorSpace()};
}
static skif::Context make_context(int outWidth, int outHeight, const SkSpecialImage* src) {
    return make_context(SkIRect::MakeWH(outWidth, outHeight), src);
}

static sk_sp<SkImage> make_small_image() {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(kBitmapSize, kBitmapSize)));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(0x00000000);
    SkPaint darkPaint;
    darkPaint.setColor(0xFF804020);
    SkPaint lightPaint;
    lightPaint.setColor(0xFF244484);
    const int kRectSize = kBitmapSize / 4;
    static_assert(kBitmapSize % 4 == 0, "bitmap size not multiple of 4");

    for (int y = 0; y < kBitmapSize; y += kRectSize) {
        for (int x = 0; x < kBitmapSize; x += kRectSize) {
            canvas->save();
            canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas->drawRect(
                    SkRect::MakeXYWH(0,         0,         kRectSize, kRectSize), darkPaint);
            canvas->drawRect(
                    SkRect::MakeXYWH(kRectSize, 0,         kRectSize, kRectSize), lightPaint);
            canvas->drawRect(
                    SkRect::MakeXYWH(0,         kRectSize, kRectSize, kRectSize), lightPaint);
            canvas->drawRect(
                    SkRect::MakeXYWH(kRectSize, kRectSize, kRectSize, kRectSize), darkPaint);
            canvas->restore();
        }
    }

    return surface->makeImageSnapshot();
}

static sk_sp<SkImageFilter> make_scale(float amount, sk_sp<SkImageFilter> input) {
    float s = amount;
    float matrix[20] = { s, 0, 0, 0, 0,
                         0, s, 0, 0, 0,
                         0, 0, s, 0, 0,
                         0, 0, 0, s, 0 };
    sk_sp<SkColorFilter> filter(SkColorFilters::Matrix(matrix));
    return SkImageFilters::ColorFilter(std::move(filter), std::move(input));
}

static sk_sp<SkImageFilter> make_grayscale(sk_sp<SkImageFilter> input,
                                           const SkIRect* cropRect) {
    float matrix[20];
    memset(matrix, 0, 20 * sizeof(float));
    matrix[0] = matrix[5] = matrix[10] = 0.2126f;
    matrix[1] = matrix[6] = matrix[11] = 0.7152f;
    matrix[2] = matrix[7] = matrix[12] = 0.0722f;
    matrix[18] = 1.0f;
    sk_sp<SkColorFilter> filter(SkColorFilters::Matrix(matrix));
    return SkImageFilters::ColorFilter(std::move(filter), std::move(input), cropRect);
}

static sk_sp<SkImageFilter> make_blue(sk_sp<SkImageFilter> input, const SkIRect* cropRect) {
    sk_sp<SkColorFilter> filter(SkColorFilters::Blend(SK_ColorBLUE, SkBlendMode::kSrcIn));
    return SkImageFilters::ColorFilter(std::move(filter), std::move(input), cropRect);
}


static sk_sp<SkDevice> create_empty_device(GrRecordingContext* rContext, int widthHeight) {

    const SkImageInfo ii = SkImageInfo::Make({ widthHeight, widthHeight },
                                             kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);

    if (rContext) {
        return rContext->priv().createDevice(skgpu::Budgeted::kNo, ii, SkBackingFit::kApprox, 1,
                                             skgpu::Mipmapped::kNo, skgpu::Protected::kNo,
                                             kTestSurfaceOrigin, {},
                                             skgpu::ganesh::Device::InitContents::kUninit);
    } else {
        SkBitmap bm;
        SkAssertResult(bm.tryAllocPixels(ii));
        return sk_make_sp<SkBitmapDevice>(bm, SkSurfaceProps());
    }
}

static sk_sp<SkSpecialImage> create_empty_special_image(GrRecordingContext* rContext,
                                                        int widthHeight,
                                                        SkColor4f color = SkColors::kTransparent) {
    sk_sp<SkDevice> device = create_empty_device(rContext, widthHeight);

    SkASSERT(device);

    SkPaint p;
    p.setColor4f(color, /*colorSpace=*/nullptr);
    p.setBlendMode(SkBlendMode::kSrc);
    device->drawPaint(p);
    return device->snapSpecial(SkIRect::MakeWH(widthHeight, widthHeight));
}


DEF_TEST(ImageFilter, reporter) {
    {
        // Check that a color matrix filter followed by a color matrix filter
        // concatenates into a single filter.
        sk_sp<SkImageFilter> doubleBrightness(make_scale(2.0f, nullptr));
        sk_sp<SkImageFilter> halfBrightness(make_scale(0.5f, std::move(doubleBrightness)));
        REPORTER_ASSERT(reporter, nullptr == halfBrightness->getInput(0));
        SkColorFilter* cf;
        REPORTER_ASSERT(reporter, halfBrightness->asColorFilter(&cf));
        cf->unref();
    }

    {
        // Check that a color filter image filter without a crop rect can be
        // expressed as a color filter.
        sk_sp<SkImageFilter> gray(make_grayscale(nullptr, nullptr));
        REPORTER_ASSERT(reporter, true == gray->asColorFilter(nullptr));
    }

    {
        // Check that a colorfilterimage filter without a crop rect but with an input
        // that is another colorfilterimage can be expressed as a colorfilter (composed).
        sk_sp<SkImageFilter> mode(make_blue(nullptr, nullptr));
        sk_sp<SkImageFilter> gray(make_grayscale(std::move(mode), nullptr));
        REPORTER_ASSERT(reporter, true == gray->asColorFilter(nullptr));
    }

    {
        // Test that if we exceed the limit of what ComposeColorFilter can combine, we still
        // can build the DAG and won't assert if we call asColorFilter.
        sk_sp<SkImageFilter> filter(make_blue(nullptr, nullptr));
        const int kWayTooManyForComposeColorFilter = 100;
        for (int i = 0; i < kWayTooManyForComposeColorFilter; ++i) {
            filter = make_blue(filter, nullptr);
            // the first few of these will succeed, but after we hit the internal limit,
            // it will then return false.
            (void)filter->asColorFilter(nullptr);
        }
    }

    {
        // Check that a color filter image filter with a crop rect cannot
        // be expressed as a color filter.
        SkIRect cropRect = SkIRect::MakeWH(100, 100);
        sk_sp<SkImageFilter> grayWithCrop(make_grayscale(nullptr, &cropRect));
        REPORTER_ASSERT(reporter, false == grayWithCrop->asColorFilter(nullptr));
    }

    {
        // Check that two non-commutative matrices are concatenated in
        // the correct order.
        float blueToRedMatrix[20] = { 0 };
        blueToRedMatrix[2] = blueToRedMatrix[18] = 1;
        float redToGreenMatrix[20] = { 0 };
        redToGreenMatrix[5] = redToGreenMatrix[18] = 1;
        sk_sp<SkColorFilter> blueToRed(SkColorFilters::Matrix(blueToRedMatrix));
        sk_sp<SkImageFilter> filter1(SkImageFilters::ColorFilter(std::move(blueToRed), nullptr));
        sk_sp<SkColorFilter> redToGreen(SkColorFilters::Matrix(redToGreenMatrix));
        sk_sp<SkImageFilter> filter2(SkImageFilters::ColorFilter(std::move(redToGreen),
                                                                 std::move(filter1)));

        SkBitmap result;
        result.allocN32Pixels(kBitmapSize, kBitmapSize);

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setImageFilter(std::move(filter2));
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
        sk_sp<SkImage> image(make_small_image());
        SkBitmap result;
        result.allocN32Pixels(kBitmapSize, kBitmapSize);

        {
            // This tests for :
            // 1 ) location at (0,0,1)
            SkPoint3 location = SkPoint3::Make(0, 0, SK_Scalar1);
            // 2 ) location and target at same value
            SkPoint3 target = SkPoint3::Make(location.fX, location.fY, location.fZ);
            // 3 ) large negative specular exponent value
            SkScalar specularExponent = -1000;

            sk_sp<SkImageFilter> bmSrc(SkImageFilters::Image(std::move(image), {}));
            SkPaint paint;
            paint.setImageFilter(SkImageFilters::SpotLitSpecular(
                    location, target, specularExponent, 180,
                    0xFFFFFFFF, SK_Scalar1, SK_Scalar1, SK_Scalar1,
                    std::move(bmSrc)));
            SkCanvas canvas(result);
            SkRect r = SkRect::MakeIWH(kBitmapSize, kBitmapSize);
            canvas.drawRect(r, paint);
        }
    }
}

static void test_cropRects(skiatest::Reporter* reporter, GrRecordingContext* rContext) {
    // Check that all filters offset to their absolute crop rect,
    // unaffected by the input crop rect.
    // Tests pass by not asserting.
    sk_sp<SkSpecialImage> srcImg(create_empty_special_image(rContext, 100));
    SkASSERT(srcImg);

    SkIRect inputCropRect = SkIRect::MakeXYWH(8, 13, 80, 80);
    SkIRect cropRect = SkIRect::MakeXYWH(20, 30, 60, 60);
    sk_sp<SkImageFilter> input(make_grayscale(nullptr, &inputCropRect));

    FilterList filters(input, &cropRect);

    for (int i = 0; i < filters.count(); ++i) {
        SkImageFilter* filter = filters.getFilter(i);
        SkIPoint offset;
        skif::Context ctx = make_context(100, 100, srcImg.get());
        sk_sp<SkSpecialImage> resultImg(as_IFB(filter)->filterImage(ctx)
                                                       .imageAndOffset(ctx, &offset));
        REPORTER_ASSERT(reporter, resultImg, "%s", filters.getName(i));
        REPORTER_ASSERT(reporter, offset.fX == 20 && offset.fY == 30, "%s", filters.getName(i));
    }
}

static bool special_image_to_bitmap(GrDirectContext* dContext, const SkSpecialImage* src,
                                    SkBitmap* dst) {
    sk_sp<SkImage> img = src->asImage();
    if (!img) {
        return false;
    }

    if (!dst->tryAllocN32Pixels(src->width(), src->height())) {
        return false;
    }

    return img->readPixels(dContext, dst->pixmap(), src->subset().fLeft, src->subset().fTop);
}

static void test_negative_blur_sigma(skiatest::Reporter* reporter,
                                     GrDirectContext* dContext) {
    // Check that SkBlurImageFilter will reject a negative sigma on creation, but properly uses the
    // absolute value of the mapped sigma after CTM application.
    static const int kWidth = 32, kHeight = 32;
    static const SkScalar kBlurSigma = SkIntToScalar(5);

    sk_sp<SkImageFilter> positiveFilter(SkImageFilters::Blur(kBlurSigma, kBlurSigma, nullptr));
    sk_sp<SkImageFilter> negativeFilter(SkImageFilters::Blur(-kBlurSigma, kBlurSigma, nullptr));
    REPORTER_ASSERT(reporter, !negativeFilter);

    sk_sp<SkImage> gradient = make_gradient_circle(kWidth, kHeight).asImage();
    sk_sp<SkSpecialImage> imgSrc;
    if (dContext) {
        imgSrc = SkSpecialImages::MakeFromTextureImage(
                dContext, SkIRect::MakeWH(kWidth, kHeight), gradient, SkSurfaceProps());
    } else {
        imgSrc = SkSpecialImages::MakeFromRaster(SkIRect::MakeWH(kWidth, kHeight), gradient, {});
    }

    SkIPoint offset;
    skif::Context ctx = make_context(32, 32, imgSrc.get());

    sk_sp<SkSpecialImage> positiveResult(
            as_IFB(positiveFilter)->filterImage(ctx).imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, positiveResult);

    SkMatrix negativeScale;
    negativeScale.setScale(-SK_Scalar1, SK_Scalar1);
    skif::Context negativeCTX = ctx.withNewMapping(skif::Mapping(negativeScale));

    sk_sp<SkSpecialImage> negativeResult(
            as_IFB(positiveFilter)->filterImage(negativeCTX).imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, negativeResult);


    SkBitmap positiveResultBM;
    SkBitmap negativeResultBM;

    REPORTER_ASSERT(reporter, special_image_to_bitmap(dContext, positiveResult.get(),
                                                      &positiveResultBM));
    REPORTER_ASSERT(reporter, special_image_to_bitmap(dContext, negativeResult.get(),
                                                      &negativeResultBM));

    for (int y = 0; y < kHeight; y++) {
        int diffs = memcmp(positiveResultBM.getAddr32(0, y),
                           negativeResultBM.getAddr32(0, y),
                           positiveResultBM.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
    }
}

DEF_TEST(ImageFilterNegativeBlurSigma, reporter) {
    test_negative_blur_sigma(reporter, nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageFilterNegativeBlurSigma_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    test_negative_blur_sigma(reporter, ctxInfo.directContext());
}

static void test_morphology_radius_with_mirror_ctm(skiatest::Reporter* reporter,
                                                   GrDirectContext* dContext) {
    // Check that SkMorphologyImageFilter maps the radius correctly when the
    // CTM contains a mirroring transform.
    static const int kWidth = 32, kHeight = 32;
    static const int kRadius = 8;

    sk_sp<SkImageFilter> filter(SkImageFilters::Dilate(kRadius, kRadius, nullptr));

    SkBitmap bitmap;
    bitmap.allocN32Pixels(kWidth, kHeight);
    SkCanvas canvas(bitmap);
    canvas.clear(SK_ColorTRANSPARENT);
    SkPaint paint;
    paint.setColor(SK_ColorWHITE);
    canvas.drawRect(SkRect::MakeXYWH(kWidth / 4, kHeight / 4, kWidth / 2, kHeight / 2),
                    paint);
    sk_sp<SkImage> image = bitmap.asImage();
    sk_sp<SkSpecialImage> imgSrc;
    if (dContext) {
        imgSrc = SkSpecialImages::MakeFromTextureImage(
                dContext, SkIRect::MakeWH(kWidth, kHeight), image, SkSurfaceProps());
    } else {
        imgSrc = SkSpecialImages::MakeFromRaster(SkIRect::MakeWH(kWidth, kHeight), image, {});
    }

    SkIPoint offset;
    skif::Context ctx = make_context(32, 32, imgSrc.get());

    sk_sp<SkSpecialImage> normalResult(
            as_IFB(filter)->filterImage(ctx).imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, normalResult);

    SkMatrix mirrorX;
    mirrorX.setTranslate(0, SkIntToScalar(32));
    mirrorX.preScale(SK_Scalar1, -SK_Scalar1);
    skif::Context mirrorXCTX = ctx.withNewMapping(skif::Mapping(mirrorX));

    sk_sp<SkSpecialImage> mirrorXResult(
            as_IFB(filter)->filterImage(mirrorXCTX).imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, mirrorXResult);

    SkMatrix mirrorY;
    mirrorY.setTranslate(SkIntToScalar(32), 0);
    mirrorY.preScale(-SK_Scalar1, SK_Scalar1);
    skif::Context mirrorYCTX = ctx.withNewMapping(skif::Mapping(mirrorY));

    sk_sp<SkSpecialImage> mirrorYResult(
            as_IFB(filter)->filterImage(mirrorYCTX).imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, mirrorYResult);

    SkBitmap normalResultBM, mirrorXResultBM, mirrorYResultBM;

    REPORTER_ASSERT(reporter, special_image_to_bitmap(dContext, normalResult.get(),
                                                      &normalResultBM));
    REPORTER_ASSERT(reporter, special_image_to_bitmap(dContext, mirrorXResult.get(),
                                                      &mirrorXResultBM));
    REPORTER_ASSERT(reporter, special_image_to_bitmap(dContext, mirrorYResult.get(),
                                                      &mirrorYResultBM));

    for (int y = 0; y < kHeight; y++) {
        int diffs = memcmp(normalResultBM.getAddr32(0, y),
                           mirrorXResultBM.getAddr32(0, y),
                           normalResultBM.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
        diffs = memcmp(normalResultBM.getAddr32(0, y),
                       mirrorYResultBM.getAddr32(0, y),
                       normalResultBM.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
    }
}

DEF_TEST(MorphologyFilterRadiusWithMirrorCTM, reporter) {
    test_morphology_radius_with_mirror_ctm(reporter, nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(MorphologyFilterRadiusWithMirrorCTM_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    test_morphology_radius_with_mirror_ctm(reporter, ctxInfo.directContext());
}

static void test_zero_blur_sigma(skiatest::Reporter* reporter, GrDirectContext* dContext) {
    // Check that SkBlurImageFilter with a zero sigma and a non-zero srcOffset works correctly.
    SkIRect cropRect = SkIRect::MakeXYWH(5, 0, 5, 10);
    sk_sp<SkImageFilter> input(SkImageFilters::Offset(0, 0, nullptr, &cropRect));
    sk_sp<SkImageFilter> filter(SkImageFilters::Blur(0, 0, std::move(input), &cropRect));

    sk_sp<SkSpecialImage> image = create_empty_special_image(dContext, 10, SkColors::kGreen);

    SkIPoint offset;
    skif::Context ctx = make_context(32, 32, image.get());


    sk_sp<SkSpecialImage> result(as_IFB(filter)->filterImage(ctx).imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, offset.fX == 5 && offset.fY == 0);
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, result->width() == 5 && result->height() == 10);

    SkBitmap resultBM;

    REPORTER_ASSERT(reporter, special_image_to_bitmap(dContext, result.get(), &resultBM));

    for (int y = 0; y < resultBM.height(); y++) {
        for (int x = 0; x < resultBM.width(); x++) {
            bool diff = *resultBM.getAddr32(x, y) != SK_ColorGREEN;
            REPORTER_ASSERT(reporter, !diff);
            if (diff) {
                break;
            }
        }
    }
}

DEF_TEST(ImageFilterZeroBlurSigma, reporter) {
    test_zero_blur_sigma(reporter, nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageFilterZeroBlurSigma_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    test_zero_blur_sigma(reporter, ctxInfo.directContext());
}

// Tests that, even when an upstream filter has returned null (due to failure or clipping), a
// downstream filter that affects transparent black still does so even with a nullptr input.
static void test_fail_affects_transparent_black(skiatest::Reporter* reporter,
                                                GrDirectContext* dContext) {
    sk_sp<SkImageFilter> failFilter = SkImageFilters::Empty();
    sk_sp<SkSpecialImage> source(create_empty_special_image(dContext, 5));
    skif::Context ctx = make_context(1, 1, source.get());

    sk_sp<SkColorFilter> green(SkColorFilters::Blend(SK_ColorGREEN, SkBlendMode::kSrc));
    SkASSERT(as_CFB(green)->affectsTransparentBlack());
    sk_sp<SkImageFilter> greenFilter(SkImageFilters::ColorFilter(std::move(green),
                                                                 std::move(failFilter)));
    SkIPoint offset;
    sk_sp<SkSpecialImage> result(as_IFB(greenFilter)->filterImage(ctx)
                                                     .imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, nullptr != result.get());
    if (result) {
        SkBitmap resultBM;
        REPORTER_ASSERT(reporter, special_image_to_bitmap(dContext, result.get(), &resultBM));
        REPORTER_ASSERT(reporter, *resultBM.getAddr32(0, 0) == SK_ColorGREEN);
    }
}

DEF_TEST(ImageFilterFailAffectsTransparentBlack, reporter) {
    test_fail_affects_transparent_black(reporter, nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageFilterFailAffectsTransparentBlack_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    test_fail_affects_transparent_black(reporter, ctxInfo.directContext());
}

DEF_TEST(ImageFilterDrawTiled, reporter) {
    // Check that all filters when drawn tiled (with subsequent clip rects) exactly
    // match the same filters drawn with a single full-canvas bitmap draw.
    // Tests pass by not asserting.

    FilterList filters(nullptr);

    SkBitmap untiledResult, tiledResult;
    const int width = 64, height = 64;
    untiledResult.allocN32Pixels(width, height);
    tiledResult.allocN32Pixels(width, height);
    SkCanvas tiledCanvas(tiledResult);
    SkCanvas untiledCanvas(untiledResult);
    const int tileSize = 8;

    SkPaint textPaint;
    textPaint.setColor(SK_ColorWHITE);
    SkFont font(ToolUtils::create_portable_typeface(), height);

    const char* text = "ABC";
    const SkScalar yPos = SkIntToScalar(height);

    for (int scale = 1; scale <= 2; ++scale) {
        for (int i = 0; i < filters.count(); ++i) {
            SkPaint combinedPaint;
            combinedPaint.setColor(SK_ColorWHITE);
            combinedPaint.setImageFilter(sk_ref_sp(filters.getFilter(i)));

            untiledCanvas.clear(SK_ColorTRANSPARENT);
            untiledCanvas.save();
            untiledCanvas.scale(SkIntToScalar(scale), SkIntToScalar(scale));
            untiledCanvas.drawString(text, 0, yPos, font, combinedPaint);
            untiledCanvas.restore();

            tiledCanvas.clear(SK_ColorTRANSPARENT);
            for (int y = 0; y < height; y += tileSize) {
                for (int x = 0; x < width; x += tileSize) {
                    tiledCanvas.save();
                    const SkRect clipRect = SkRect::MakeXYWH(x, y, tileSize, tileSize);
                    tiledCanvas.clipRect(clipRect);
                    if (filters.needsSaveLayer(i)) {
                        tiledCanvas.saveLayer(nullptr, &combinedPaint);
                            tiledCanvas.scale(SkIntToScalar(scale), SkIntToScalar(scale));
                            tiledCanvas.drawString(text, 0, yPos, font, textPaint);
                        tiledCanvas.restore();
                    } else {
                        tiledCanvas.scale(SkIntToScalar(scale), SkIntToScalar(scale));
                        tiledCanvas.drawString(text, 0, yPos, font, combinedPaint);
                    }

                    tiledCanvas.restore();
                }
            }

            if (!ToolUtils::equal_pixels(untiledResult, tiledResult)) {
                SkString encoded;
                SkString errString("Tiled image filter doesn't match untiled reference");
                errString.append("\nExpected: ");
                if (ToolUtils::BitmapToBase64DataURI(untiledResult, &encoded)) {
                    errString.append(encoded);
                } else {
                    errString.append("failed to encode");
                }

                errString.append("\nActual: ");
                if (ToolUtils::BitmapToBase64DataURI(tiledResult, &encoded)) {
                    errString.append(encoded);
                } else {
                    errString.append("failed to encode");
                }

                ERRORF(reporter, "%s\n%s", filters.getName(i), errString.c_str());
            }
        }
    }
}

static void draw_saveLayer_picture(int width, int height, int tileSize,
                                   SkBBHFactory* factory, SkBitmap* result) {

    SkMatrix matrix;
    matrix.setTranslate(SkIntToScalar(50), 0);

    sk_sp<SkColorFilter> cf(SkColorFilters::Blend(SK_ColorWHITE, SkBlendMode::kSrc));
    sk_sp<SkImageFilter> cfif(SkImageFilters::ColorFilter(std::move(cf), nullptr));
    sk_sp<SkImageFilter> imageFilter(SkImageFilters::MatrixTransform(matrix,
                                                                     SkSamplingOptions(),
                                                                     std::move(cfif)));

    SkPaint paint;
    paint.setImageFilter(std::move(imageFilter));
    SkPictureRecorder recorder;
    SkRect bounds = SkRect::Make(SkIRect::MakeXYWH(0, 0, 50, 50));
    SkCanvas* recordingCanvas = recorder.beginRecording(SkIntToScalar(width),
                                                        SkIntToScalar(height),
                                                        factory);
    recordingCanvas->translate(-55, 0);
    recordingCanvas->saveLayer(&bounds, &paint);
    recordingCanvas->restore();
    sk_sp<SkPicture> picture1(recorder.finishRecordingAsPicture());

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
    draw_saveLayer_picture(width, height, tileSize, nullptr, &result2);

    for (int y = 0; y < height; y++) {
        int diffs = memcmp(result1.getAddr32(0, y), result2.getAddr32(0, y), result1.rowBytes());
        REPORTER_ASSERT(reporter, !diffs);
        if (diffs) {
            break;
        }
    }
}

static sk_sp<SkImageFilter> make_blur(sk_sp<SkImageFilter> input) {
    return SkImageFilters::Blur(SK_Scalar1, SK_Scalar1, std::move(input));
}

static sk_sp<SkImageFilter> make_drop_shadow(sk_sp<SkImageFilter> input) {
    return SkImageFilters::DropShadow(100, 100, 10, 10, SK_ColorBLUE, std::move(input));
}

DEF_TEST(ImageFilterBlurThenShadowBounds, reporter) {
    sk_sp<SkImageFilter> filter1(make_blur(nullptr));
    sk_sp<SkImageFilter> filter2(make_drop_shadow(std::move(filter1)));

    static const SkIRect kContentBounds = SkIRect::MakeXYWH(0, 0, 100, 100);

    // For output, the [0,0,100,100] source is expanded to [-3,-3,103,103] by the initial blur.
    // The drop shadow is translated by [100,100] and further outset by 30px -> [67,67,233,233],
    // The blend unions the inner blur result with the drop shadow to get [-3,-3,233,233].
    static const SkIRect kExpectedOutputBounds = SkIRect::MakeLTRB(-3, -3, 233, 233);
    SkIRect outputBounds = filter2->filterBounds(kContentBounds,
                                                 SkMatrix::I(),
                                                 SkImageFilter::kForward_MapDirection);
    REPORTER_ASSERT(reporter, outputBounds == kExpectedOutputBounds);

    // For input, it should be able to restrict itself to the source content.
    SkIRect inputBounds = filter2->filterBounds(kExpectedOutputBounds,
                                                SkMatrix::I(),
                                                SkImageFilter::kReverse_MapDirection,
                                                &kContentBounds);

    REPORTER_ASSERT(reporter, inputBounds == kContentBounds);
}

DEF_TEST(ImageFilterShadowThenBlurBounds, reporter) {
    sk_sp<SkImageFilter> filter1(make_drop_shadow(nullptr));
    sk_sp<SkImageFilter> filter2(make_blur(std::move(filter1)));

    static const SkIRect kContentBounds = SkIRect::MakeXYWH(0, 0, 100, 100);
    // For output, the [0,0,100,100] source is translated by 100px and outset by 30px for the drop
    // shadow = [70,70,230,230], then blended back with its original to get [0,0,230,230]. This is
    // then outset by 3px for the outer blur to get [-3,-3,233,233].
    static const SkIRect kExpectedOutputBounds = SkIRect::MakeLTRB(-3, -3, 233, 233);
    SkIRect outputBounds = filter2->filterBounds(kContentBounds,
                                                 SkMatrix::I(),
                                                 SkImageFilter::kForward_MapDirection);
    REPORTER_ASSERT(reporter, outputBounds == kExpectedOutputBounds);

    // For input, it should be able to restrict itself to the source content.
    SkIRect inputBounds = filter2->filterBounds(kExpectedOutputBounds,
                                                SkMatrix::I(),
                                                SkImageFilter::kReverse_MapDirection,
                                                &kContentBounds);
    REPORTER_ASSERT(reporter, inputBounds == kContentBounds);
}

DEF_TEST(ImageFilterDilateThenBlurBounds, reporter) {
    sk_sp<SkImageFilter> filter1(SkImageFilters::Dilate(2, 2, nullptr));
    sk_sp<SkImageFilter> filter2(make_drop_shadow(std::move(filter1)));

    static const SkIRect kContentBounds = SkIRect::MakeXYWH(0, 0, 100, 100);
    // For output, the [0,0,100,100] source is outset by dilate radius (2px) to [-2,-2,102,102].
    // This is then translated by 100px and outset by 30px for the drop shadow = [68,68,232,232].
    // Finally this is joined with the original dilate result to get [-2,-2,232,232].
    static const SkIRect kExpectedOutputBounds = SkIRect::MakeLTRB(-2, -2, 232, 232);
    SkIRect outputBounds = filter2->filterBounds(kContentBounds,
                                                 SkMatrix::I(),
                                                 SkImageFilter::kForward_MapDirection);
    REPORTER_ASSERT(reporter, outputBounds == kExpectedOutputBounds);

    // For input, it should be able to restrict itself to the source content.
    SkIRect inputBounds = filter2->filterBounds(kExpectedOutputBounds,
                                                SkMatrix::I(),
                                                SkImageFilter::kReverse_MapDirection,
                                                &kContentBounds);
    REPORTER_ASSERT(reporter, inputBounds == kContentBounds);
}

DEF_TEST(ImageFilterScaledBlurRadius, reporter) {
    // Each blur should spread 3*sigma, so 3 for the blur and 30 for the shadow
    // (before the CTM). Bounds should be computed correctly in the presence of
    // a (possibly negative) scale.
    sk_sp<SkImageFilter> blur(make_blur(nullptr));
    sk_sp<SkImageFilter> dropShadow(make_drop_shadow(nullptr));
    {
        // Uniform scale by 2.
        SkMatrix scaleMatrix;
        scaleMatrix.setScale(2, 2);
        static const SkIRect kBounds = SkIRect::MakeLTRB(0, 0, 200, 200);

        static const SkIRect kExpectedBlurBounds = SkIRect::MakeLTRB(-6, -6, 206, 206);
        SkIRect blurBounds = blur->filterBounds(
                kBounds, scaleMatrix, SkImageFilter::kForward_MapDirection, nullptr);
        REPORTER_ASSERT(reporter, blurBounds == kExpectedBlurBounds);
        SkIRect reverseBlurBounds = blur->filterBounds(
                kExpectedBlurBounds, scaleMatrix, SkImageFilter::kReverse_MapDirection, &kBounds);
        REPORTER_ASSERT(reporter, reverseBlurBounds == kBounds);

        static const SkIRect kExpectedShadowBounds = SkIRect::MakeLTRB(0, 0, 460, 460);
        SkIRect shadowBounds = dropShadow->filterBounds(
                kBounds, scaleMatrix, SkImageFilter::kForward_MapDirection, nullptr);
        REPORTER_ASSERT(reporter, shadowBounds == kExpectedShadowBounds);

        SkIRect reverseShadowBounds = dropShadow->filterBounds(
                kExpectedShadowBounds, scaleMatrix, SkImageFilter::kReverse_MapDirection, &kBounds);
        REPORTER_ASSERT(reporter, reverseShadowBounds == kBounds);
    }
    {
        // Vertical flip.
        SkMatrix scaleMatrix;
        scaleMatrix.setScale(1, -1);
        static const SkIRect kBounds = SkIRect::MakeLTRB(0, -100, 100, 0);

        static const SkIRect kExpectedBlurBounds = SkIRect::MakeLTRB(-3, -103, 103, 3);
        SkIRect blurBounds = blur->filterBounds(
                kBounds, scaleMatrix, SkImageFilter::kForward_MapDirection, nullptr);
        REPORTER_ASSERT(reporter, blurBounds == kExpectedBlurBounds);
        SkIRect reverseBlurBounds = blur->filterBounds(
                kExpectedBlurBounds, scaleMatrix, SkImageFilter::kReverse_MapDirection, &kBounds);
        REPORTER_ASSERT(reporter, reverseBlurBounds == kBounds);

        SkIRect kExpectedShadowBounds = SkIRect::MakeLTRB(0, -230, 230, 0);
        SkIRect shadowBounds = dropShadow->filterBounds(
                kBounds, scaleMatrix, SkImageFilter::kForward_MapDirection, nullptr);
        REPORTER_ASSERT(reporter, shadowBounds == kExpectedShadowBounds);
        SkIRect reverseShadowBounds = dropShadow->filterBounds(
                kExpectedShadowBounds, scaleMatrix, SkImageFilter::kReverse_MapDirection, &kBounds);
        REPORTER_ASSERT(reporter, reverseShadowBounds == kBounds);
    }
}

DEF_TEST(ImageFilterComposedBlurFastBounds, reporter) {
    sk_sp<SkImageFilter> filter1(make_blur(nullptr));
    sk_sp<SkImageFilter> filter2(make_blur(nullptr));
    sk_sp<SkImageFilter> composedFilter(SkImageFilters::Compose(std::move(filter1),
                                                                std::move(filter2)));

    static const SkRect kBoundsSrc = SkRect::MakeIWH(100, 100);
    static const SkRect kExpectedBounds = SkRect::MakeXYWH(-6, -6, 112, 112);
    SkRect boundsDst = composedFilter->computeFastBounds(kBoundsSrc);

    REPORTER_ASSERT(reporter, boundsDst == kExpectedBounds);
}

DEF_TEST(ImageFilterUnionBounds, reporter) {
    sk_sp<SkImageFilter> offset(SkImageFilters::Offset(50, 0, nullptr));
    // Regardless of which order they appear in, the image filter bounds should
    // be combined correctly.
    {
        sk_sp<SkImageFilter> composite(SkImageFilters::Blend(SkBlendMode::kSrcOver, offset));
        SkRect bounds = SkRect::MakeIWH(100, 100);
        // Intentionally aliasing here, as that's what the real callers do.
        bounds = composite->computeFastBounds(bounds);
        REPORTER_ASSERT(reporter, bounds == SkRect::MakeIWH(150, 100));
    }
    {
        sk_sp<SkImageFilter> composite(SkImageFilters::Blend(SkBlendMode::kSrcOver, nullptr,
                                                             offset, nullptr));
        SkRect bounds = SkRect::MakeIWH(100, 100);
        // Intentionally aliasing here, as that's what the real callers do.
        bounds = composite->computeFastBounds(bounds);
        REPORTER_ASSERT(reporter, bounds == SkRect::MakeIWH(150, 100));
    }
}

static void test_imagefilter_merge_result_size(skiatest::Reporter* reporter,
                                               GrRecordingContext* rContext) {
    SkBitmap greenBM;
    greenBM.allocN32Pixels(20, 20);
    greenBM.eraseColor(SK_ColorGREEN);
    sk_sp<SkImage> greenImage(greenBM.asImage());
    sk_sp<SkImageFilter> source(SkImageFilters::Image(std::move(greenImage), {}));
    sk_sp<SkImageFilter> merge(SkImageFilters::Merge(source, source));

    sk_sp<SkSpecialImage> srcImg(create_empty_special_image(rContext, 1));

    skif::Context ctx = make_context(100, 100, srcImg.get());
    SkIPoint offset;

    sk_sp<SkSpecialImage> resultImg(as_IFB(merge)->filterImage(ctx).imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, resultImg);

    REPORTER_ASSERT(reporter, resultImg->width() == 20 && resultImg->height() == 20);
}

DEF_TEST(ImageFilterMergeResultSize, reporter) {
    test_imagefilter_merge_result_size(reporter, nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageFilterMergeResultSize_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    test_imagefilter_merge_result_size(reporter, ctxInfo.directContext());
}

static void draw_blurred_rect(SkCanvas* canvas) {
    SkPaint filterPaint;
    filterPaint.setColor(SK_ColorWHITE);
    filterPaint.setImageFilter(SkImageFilters::Blur(SkIntToScalar(8), 0, nullptr));
    canvas->saveLayer(nullptr, &filterPaint);
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
    SkCanvas* recordingCanvas1 = recorder1.beginRecording(width, height);
    SkCanvas* recordingCanvas2 = recorder2.beginRecording(width, height, &factory);

    draw_blurred_rect(recordingCanvas1);
    draw_blurred_rect(recordingCanvas2);
    sk_sp<SkPicture> picture1(recorder1.finishRecordingAsPicture());
    sk_sp<SkPicture> picture2(recorder2.finishRecordingAsPicture());
    for (int y = 0; y < height; y += tileSize) {
        for (int x = 0; x < width; x += tileSize) {
            SkRect tileRect = SkRect::Make(SkIRect::MakeXYWH(x, y, tileSize, tileSize));
            draw_picture_clipped(&canvas1, tileRect, picture1.get());
            draw_picture_clipped(&canvas2, tileRect, picture2.get());
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

    sk_sp<SkImageFilter> filter(SkImageFilters::MatrixConvolution(
            kernelSize, kernel, gain, bias, kernelOffset, SkTileMode::kRepeat, false, nullptr));

    SkBitmap result;
    int width = 16, height = 16;
    result.allocN32Pixels(width, height);
    SkCanvas canvas(result);
    canvas.clear(0);

    SkPaint paint;
    paint.setImageFilter(std::move(filter));
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

    sk_sp<SkImageFilter> filter(SkImageFilters::MatrixConvolution(
            kernelSize, kernel, gain, bias, kernelOffset, SkTileMode::kClamp, true, nullptr));

    SkBitmap result;

    int width = 10, height = 10;
    result.allocN32Pixels(width, height);
    SkCanvas canvas(result);
    canvas.clear(0);

    SkPaint filterPaint;
    filterPaint.setImageFilter(std::move(filter));
    SkRect bounds = SkRect::MakeIWH(1, 10);
    SkRect rect = SkRect::Make(SkIRect::MakeWH(width, height));
    SkPaint rectPaint;
    canvas.saveLayer(&bounds, &filterPaint);
    canvas.drawRect(rect, rectPaint);
    canvas.restore();
}

static void test_big_kernel(skiatest::Reporter* reporter, GrRecordingContext* rContext) {
    // Check that a kernel that is too big for the GPU still works
    SkScalar identityKernel[49] = {
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0
    };
    SkISize kernelSize = SkISize::Make(7, 7);
    SkScalar gain = SK_Scalar1, bias = 0;
    SkIPoint kernelOffset = SkIPoint::Make(0, 0);

    sk_sp<SkImageFilter> filter(SkImageFilters::MatrixConvolution(
            kernelSize, identityKernel, gain, bias, kernelOffset,
            SkTileMode::kClamp, true, nullptr));

    sk_sp<SkSpecialImage> srcImg(create_empty_special_image(rContext, 100));
    SkASSERT(srcImg);

    SkIPoint offset;
    skif::Context ctx = make_context(100, 100, srcImg.get());
    sk_sp<SkSpecialImage> resultImg(as_IFB(filter)->filterImage(ctx).imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, resultImg);
    REPORTER_ASSERT(reporter, SkToBool(rContext) == resultImg->isGaneshBacked());
    REPORTER_ASSERT(reporter, resultImg->width() == 100 && resultImg->height() == 100);
    REPORTER_ASSERT(reporter, offset.fX == 0 && offset.fY == 0);
}

DEF_TEST(ImageFilterMatrixConvolutionBigKernel, reporter) {
    test_big_kernel(reporter, nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageFilterMatrixConvolutionBigKernel_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    test_big_kernel(reporter, ctxInfo.directContext());
}

DEF_TEST(ImageFilterCropRect, reporter) {
    test_cropRects(reporter, nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageFilterCropRect_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    test_cropRects(reporter, ctxInfo.directContext());
}

DEF_TEST(ImageFilterMatrix, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(100, 100);
    SkCanvas canvas(temp);
    canvas.scale(SkIntToScalar(2), SkIntToScalar(2));

    SkMatrix expectedMatrix = canvas.getTotalMatrix();

    SkRTreeFactory factory;
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(100, 100, &factory);

    SkPaint paint;
    paint.setImageFilter(sk_sp<SkImageFilter>(new MatrixTestImageFilter(reporter, expectedMatrix)));
    recordingCanvas->saveLayer(nullptr, &paint);
    SkPaint solidPaint;
    solidPaint.setColor(0xFFFFFFFF);
    recordingCanvas->save();
    recordingCanvas->scale(SkIntToScalar(10), SkIntToScalar(10));
    recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(100, 100)), solidPaint);
    recordingCanvas->restore(); // scale
    recordingCanvas->restore(); // saveLayer

    canvas.drawPicture(recorder.finishRecordingAsPicture());
}

static void test_clipped_picture_imagefilter(skiatest::Reporter* reporter,
                                             GrRecordingContext* rContext) {
    sk_sp<SkPicture> picture;

    {
        SkRTreeFactory factory;
        SkPictureRecorder recorder;
        SkCanvas* recordingCanvas = recorder.beginRecording(1, 1, &factory);

        // Create an SkPicture which simply draws a green 1x1 rectangle.
        SkPaint greenPaint;
        greenPaint.setColor(SK_ColorGREEN);
        recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeWH(1, 1)), greenPaint);
        picture = recorder.finishRecordingAsPicture();
    }

    sk_sp<SkSpecialImage> srcImg(create_empty_special_image(rContext, 2));

    sk_sp<SkImageFilter> imageFilter(SkImageFilters::Picture(picture));

    SkIPoint offset;
    skif::Context ctx = make_context(SkIRect::MakeXYWH(1,1,1,1), srcImg.get());

    sk_sp<SkSpecialImage> resultImage(
            as_IFB(imageFilter)->filterImage(ctx).imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, !resultImage);
}

DEF_TEST(ImageFilterClippedPictureImageFilter, reporter) {
    test_clipped_picture_imagefilter(reporter, nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageFilterClippedPictureImageFilter_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    test_clipped_picture_imagefilter(reporter, ctxInfo.directContext());
}

DEF_TEST(ImageFilterEmptySaveLayer, reporter) {
    // Even when there's an empty saveLayer()/restore(), ensure that an image
    // filter or color filter which affects transparent black still draws.

    SkBitmap bitmap;
    bitmap.allocN32Pixels(10, 10);
    SkCanvas canvas(bitmap);

    SkRTreeFactory factory;
    SkPictureRecorder recorder;

    sk_sp<SkColorFilter> green(SkColorFilters::Blend(SK_ColorGREEN, SkBlendMode::kSrc));
    sk_sp<SkImageFilter> imageFilter(SkImageFilters::ColorFilter(green, nullptr));
    SkPaint imageFilterPaint;
    imageFilterPaint.setImageFilter(std::move(imageFilter));
    SkPaint colorFilterPaint;
    colorFilterPaint.setColorFilter(green);

    SkRect bounds = SkRect::MakeIWH(10, 10);

    SkCanvas* recordingCanvas = recorder.beginRecording(10, 10, &factory);
    recordingCanvas->saveLayer(&bounds, &imageFilterPaint);
    recordingCanvas->restore();
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    canvas.clear(0);
    canvas.drawPicture(picture);
    uint32_t pixel = *bitmap.getAddr32(0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    recordingCanvas = recorder.beginRecording(10, 10, &factory);
    recordingCanvas->saveLayer(nullptr, &imageFilterPaint);
    recordingCanvas->restore();
    sk_sp<SkPicture> picture2(recorder.finishRecordingAsPicture());

    canvas.clear(0);
    canvas.drawPicture(picture2);
    pixel = *bitmap.getAddr32(0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    recordingCanvas = recorder.beginRecording(10, 10, &factory);
    recordingCanvas->saveLayer(&bounds, &colorFilterPaint);
    recordingCanvas->restore();
    sk_sp<SkPicture> picture3(recorder.finishRecordingAsPicture());

    canvas.clear(0);
    canvas.drawPicture(picture3);
    pixel = *bitmap.getAddr32(0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
}

static void test_huge_blur(SkCanvas* canvas, skiatest::Reporter* reporter) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(100, 100);
    bitmap.eraseARGB(0, 0, 0, 0);

    // Check that a blur with a very large radius does not crash or assert.
    SkPaint paint;
    paint.setImageFilter(SkImageFilters::Blur(SkIntToScalar(1<<30), SkIntToScalar(1<<30), nullptr));
    canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);
}

DEF_TEST(HugeBlurImageFilter, reporter) {
    SkBitmap temp;
    temp.allocN32Pixels(100, 100);
    SkCanvas canvas(temp);
    test_huge_blur(&canvas, reporter);
}

DEF_TEST(ImageFilterMatrixConvolutionTest, reporter) {
    SkScalar kernel[1] = { 0 };
    SkScalar gain = SK_Scalar1, bias = 0;
    SkIPoint kernelOffset = SkIPoint::Make(1, 1);

    // Check that an enormous (non-allocatable) kernel gives a nullptr filter.
    sk_sp<SkImageFilter> conv(SkImageFilters::MatrixConvolution(
            SkISize::Make(1<<30, 1<<30), kernel, gain, bias, kernelOffset,
            SkTileMode::kRepeat, false, nullptr));

    REPORTER_ASSERT(reporter, nullptr == conv.get());

    // Check that a nullptr kernel gives a nullptr filter.
    conv = SkImageFilters::MatrixConvolution(
            SkISize::Make(1, 1), nullptr, gain, bias, kernelOffset,
            SkTileMode::kRepeat, false, nullptr);

    REPORTER_ASSERT(reporter, nullptr == conv.get());

    // Check that a kernel width < 1 gives a nullptr filter.
    conv = SkImageFilters::MatrixConvolution(
            SkISize::Make(0, 1), kernel, gain, bias, kernelOffset,
            SkTileMode::kRepeat, false, nullptr);

    REPORTER_ASSERT(reporter, nullptr == conv.get());

    // Check that kernel height < 1 gives a nullptr filter.
    conv = SkImageFilters::MatrixConvolution(
            SkISize::Make(1, -1), kernel, gain, bias, kernelOffset,
            SkTileMode::kRepeat, false, nullptr);

    REPORTER_ASSERT(reporter, nullptr == conv.get());
}

static void test_xfermode_cropped_input(SkSurface* surf, skiatest::Reporter* reporter) {
    auto canvas = surf->getCanvas();
    canvas->clear(SK_ColorRED);

    SkBitmap bitmap;
    bitmap.allocN32Pixels(1, 1);
    bitmap.eraseARGB(255, 255, 255, 255);

    sk_sp<SkColorFilter> green(SkColorFilters::Blend(SK_ColorGREEN, SkBlendMode::kSrcIn));
    sk_sp<SkImageFilter> greenFilter(SkImageFilters::ColorFilter(green, nullptr));
    SkIRect cropRect = SkIRect::MakeEmpty();
    sk_sp<SkImageFilter> croppedOut(SkImageFilters::ColorFilter(green, nullptr, &cropRect));

    // Check that an blend image filter whose input has been cropped out still draws the other
    // input. Also check that drawing with both inputs cropped out doesn't cause a GPU warning.
    SkBlendMode mode = SkBlendMode::kSrcOver;
    sk_sp<SkImageFilter> xfermodeNoFg(SkImageFilters::Blend(
            mode, greenFilter, croppedOut, nullptr));
    sk_sp<SkImageFilter> xfermodeNoBg(SkImageFilters::Blend(
            mode, croppedOut, greenFilter, nullptr));
    sk_sp<SkImageFilter> xfermodeNoFgNoBg(SkImageFilters::Blend(
            mode, croppedOut,  croppedOut, nullptr));

    SkPaint paint;
    paint.setImageFilter(std::move(xfermodeNoFg));
    canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);   // drawSprite

    // xfermodeNoFg is a src-over blend between a green image and a transparent black image,
    // so should just be green.
    uint32_t pixel;
    SkImageInfo info = SkImageInfo::Make(1, 1, kBGRA_8888_SkColorType, kUnpremul_SkAlphaType);
    surf->readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    // xfermodeNoBg is the reverse of the above, but because it's src-over the final blend
    // between transparent black and green is still green.
    canvas->clear(SK_ColorRED); // should be overwritten
    paint.setImageFilter(std::move(xfermodeNoBg));
    canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);   // drawSprite
    surf->readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    // xfermodeNoFgNoBg is a src-over blend of two empty images, so should produce no change
    // to the image.
    canvas->clear(SK_ColorRED); // should not be overwritten
    paint.setImageFilter(std::move(xfermodeNoFgNoBg));
    canvas->drawImage(bitmap.asImage(), 0, 0, SkSamplingOptions(), &paint);   // drawSprite
    surf->readPixels(info, &pixel, 4, 0, 0);
    REPORTER_ASSERT(reporter, pixel == SK_ColorRED);
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
    sk_sp<SkImageFilter> matrixFilter(
        SkImageFilters::MatrixTransform(matrix, SkSamplingOptions(SkFilterMode::kLinear), nullptr));

    // Test that saveLayer() with a filter nested inside another saveLayer() applies the
    // correct offset to the filter matrix.
    SkRect bounds1 = SkRect::MakeXYWH(10, 10, 30, 30);
    canvas.saveLayer(&bounds1, nullptr);
    SkPaint filterPaint;
    filterPaint.setImageFilter(std::move(matrixFilter));
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
    temp.readPixels(info, &pixel, 4, 25, 25);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);

    // Test that drawSprite() with a filter nested inside a saveLayer() applies the
    // correct offset to the filter matrix.
    canvas.clear(0x0);
    temp.readPixels(info, &pixel, 4, 25, 25);
    canvas.saveLayer(&bounds1, nullptr);
    canvas.drawImage(bitmap.asImage(), 20, 20, SkSamplingOptions(), &filterPaint); // drawSprite
    canvas.restore();

    temp.readPixels(info, &pixel, 4, 25, 25);
    REPORTER_ASSERT(reporter, pixel == SK_ColorGREEN);
}

DEF_TEST(XfermodeImageFilterCroppedInput, reporter) {
    test_xfermode_cropped_input(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100)).get(),
                                reporter);
}

static void test_composed_imagefilter_offset(skiatest::Reporter* reporter,
                                             GrRecordingContext* rContext) {
    sk_sp<SkSpecialImage> srcImg(create_empty_special_image(rContext, 100));

    SkIRect cropRect = SkIRect::MakeXYWH(1, 0, 20, 20);
    sk_sp<SkImageFilter> offsetFilter(SkImageFilters::Offset(0, 0, nullptr, &cropRect));
    sk_sp<SkImageFilter> blurFilter(SkImageFilters::Blur(SK_Scalar1, SK_Scalar1,
                                                            nullptr, &cropRect));
    sk_sp<SkImageFilter> composedFilter(SkImageFilters::Compose(std::move(blurFilter),
                                                                std::move(offsetFilter)));
    SkIPoint offset;
    skif::Context ctx = make_context(100, 100, srcImg.get());

    sk_sp<SkSpecialImage> resultImg(
            as_IFB(composedFilter)->filterImage(ctx).imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, resultImg);
    REPORTER_ASSERT(reporter, offset.fX == 1 && offset.fY == 0);
}

DEF_TEST(ComposedImageFilterOffset, reporter) {
    test_composed_imagefilter_offset(reporter, nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ComposedImageFilterOffset_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    test_composed_imagefilter_offset(reporter, ctxInfo.directContext());
}

static void test_composed_imagefilter_bounds(skiatest::Reporter* reporter,
                                             GrDirectContext* dContext) {
    // The bounds passed to the inner filter must be filtered by the outer
    // filter, so that the inner filter produces the pixels that the outer
    // filter requires as input. This matters if the outer filter moves pixels.
    // Here, accounting for the outer offset is necessary so that the green
    // pixels of the picture are not clipped.

    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(SkRect::MakeIWH(200, 100));
    recordingCanvas->clipRect(SkRect::MakeXYWH(100, 0, 100, 100));
    recordingCanvas->clear(SK_ColorGREEN);
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());
    sk_sp<SkImageFilter> pictureFilter(SkImageFilters::Picture(picture));
    SkIRect cropRect = SkIRect::MakeWH(100, 100);
    sk_sp<SkImageFilter> offsetFilter(SkImageFilters::Offset(-100, 0, nullptr, &cropRect));
    sk_sp<SkImageFilter> composedFilter(SkImageFilters::Compose(std::move(offsetFilter),
                                                                std::move(pictureFilter)));

    sk_sp<SkSpecialImage> sourceImage(create_empty_special_image(dContext, 100));
    skif::Context ctx = make_context(100, 100, sourceImage.get());

    SkIPoint offset;
    sk_sp<SkSpecialImage> result(
            as_IFB(composedFilter)->filterImage(ctx).imageAndOffset(ctx, &offset));
    REPORTER_ASSERT(reporter, offset.isZero());
    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, result->subset().size() == SkISize::Make(100, 100));

    SkBitmap resultBM;
    REPORTER_ASSERT(reporter, special_image_to_bitmap(dContext, result.get(), &resultBM));
    REPORTER_ASSERT(reporter, resultBM.getColor(50, 50) == SK_ColorGREEN);
}

DEF_TEST(ComposedImageFilterBounds, reporter) {
    test_composed_imagefilter_bounds(reporter, nullptr);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ComposedImageFilterBounds_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    test_composed_imagefilter_bounds(reporter, ctxInfo.directContext());
}

DEF_TEST(ImageFilterCanComputeFastBounds, reporter) {

    {
        SkPoint3 location = SkPoint3::Make(0, 0, SK_Scalar1);
        sk_sp<SkImageFilter> lighting(SkImageFilters::PointLitDiffuse(
                location,  SK_ColorGREEN, 0, 0, nullptr));
        REPORTER_ASSERT(reporter, !lighting->canComputeFastBounds());
    }

    {
        sk_sp<SkImageFilter> gray(make_grayscale(nullptr, nullptr));
        REPORTER_ASSERT(reporter, gray->canComputeFastBounds());
        {
            SkColorFilter* grayCF;
            REPORTER_ASSERT(reporter, gray->asAColorFilter(&grayCF));
            REPORTER_ASSERT(reporter, !as_CFB(grayCF)->affectsTransparentBlack());
            grayCF->unref();
        }
        REPORTER_ASSERT(reporter, gray->canComputeFastBounds());

        sk_sp<SkImageFilter> grayBlur(SkImageFilters::Blur(
                SK_Scalar1, SK_Scalar1, std::move(gray)));
        REPORTER_ASSERT(reporter, grayBlur->canComputeFastBounds());
    }

    {
        float greenMatrix[20] = { 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 1.0f/255,
                                  0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 1.0f/255
        };
        sk_sp<SkColorFilter> greenCF(SkColorFilters::Matrix(greenMatrix));
        sk_sp<SkImageFilter> green(SkImageFilters::ColorFilter(greenCF, nullptr));

        REPORTER_ASSERT(reporter, as_CFB(greenCF)->affectsTransparentBlack());
        REPORTER_ASSERT(reporter, !green->canComputeFastBounds());

        sk_sp<SkImageFilter> greenBlur(SkImageFilters::Blur(SK_Scalar1, SK_Scalar1,
                                                               std::move(green)));
        REPORTER_ASSERT(reporter, !greenBlur->canComputeFastBounds());
    }

    uint8_t allOne[256], identity[256];
    for (int i = 0; i < 256; ++i) {
        identity[i] = i;
        allOne[i] = 255;
    }

    sk_sp<SkColorFilter> identityCF(SkColorFilters::TableARGB(identity, identity,
                                                              identity, allOne));
    sk_sp<SkImageFilter> identityFilter(SkImageFilters::ColorFilter(identityCF, nullptr));
    REPORTER_ASSERT(reporter, !as_CFB(identityCF)->affectsTransparentBlack());
    REPORTER_ASSERT(reporter, identityFilter->canComputeFastBounds());

    sk_sp<SkColorFilter> forceOpaqueCF(SkColorFilters::TableARGB(allOne, identity,
                                                                 identity, identity));
    sk_sp<SkImageFilter> forceOpaque(SkImageFilters::ColorFilter(forceOpaqueCF, nullptr));
    REPORTER_ASSERT(reporter, as_CFB(forceOpaqueCF)->affectsTransparentBlack());
    REPORTER_ASSERT(reporter, !forceOpaque->canComputeFastBounds());
}

// Verify that SkImageSource survives serialization
DEF_TEST(ImageFilterImageSourceSerialization, reporter) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(10, 10)));
    surface->getCanvas()->clear(SK_ColorGREEN);
    sk_sp<SkImage> image(surface->makeImageSnapshot());
    sk_sp<SkImageFilter> filter(SkImageFilters::Image(std::move(image), SkFilterMode::kNearest));

    SkSerialProcs sProcs;
    sProcs.fImageProc = [](SkImage* img, void*) -> sk_sp<SkData> {
        return SkPngEncoder::Encode(as_IB(img)->directContext(), img, SkPngEncoder::Options{});
    };
    sk_sp<SkData> data(filter->serialize(&sProcs));
    sk_sp<SkImageFilter> unflattenedFilter = SkImageFilter::Deserialize(data->data(), data->size());
    REPORTER_ASSERT(reporter, unflattenedFilter);

    SkBitmap bm;
    bm.allocN32Pixels(10, 10);
    bm.eraseColor(SK_ColorBLUE);
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setImageFilter(unflattenedFilter);

    SkCanvas canvas(bm);
    canvas.drawRect(SkRect::MakeIWH(10, 10), paint);
    REPORTER_ASSERT(reporter, *bm.getAddr32(0, 0) == SkPreMultiplyColor(SK_ColorGREEN));
}

DEF_TEST(ImageFilterImageSourceUninitialized, r) {
    sk_sp<SkData> data(GetResourceAsData("crbug769134.fil"));
    if (!data) {
        return;
    }
    sk_sp<SkImageFilter> unflattenedFilter = SkImageFilter::Deserialize(data->data(), data->size());
    // This will fail. More importantly, msan will verify that we did not
    // compare against uninitialized memory.
    REPORTER_ASSERT(r, !unflattenedFilter);
}

static void test_large_blur_input(skiatest::Reporter* reporter, SkCanvas* canvas) {
    SkBitmap largeBmp;
    int largeW = 5000;
    int largeH = 5000;
    // If we're GPU-backed make the bitmap too large to be converted into a texture.
    if (auto ctx = canvas->recordingContext()) {
        largeW = ctx->priv().caps()->maxTextureSize() + 1;
    }

    largeBmp.allocN32Pixels(largeW, largeH);
    largeBmp.eraseColor(0);
    if (!largeBmp.getPixels()) {
        ERRORF(reporter, "Failed to allocate large bmp.");
        return;
    }

    sk_sp<SkImage> largeImage(largeBmp.asImage());
    if (!largeImage) {
        ERRORF(reporter, "Failed to create large image.");
        return;
    }

    sk_sp<SkImageFilter> largeSource(SkImageFilters::Image(std::move(largeImage), {}));
    if (!largeSource) {
        ERRORF(reporter, "Failed to create large SkImageSource.");
        return;
    }

    sk_sp<SkImageFilter> blur(SkImageFilters::Blur(10.f, 10.f, std::move(largeSource)));
    if (!blur) {
        ERRORF(reporter, "Failed to create SkBlurImageFilter.");
        return;
    }

    SkPaint paint;
    paint.setImageFilter(std::move(blur));

    // This should not crash (http://crbug.com/570479).
    canvas->drawRect(SkRect::MakeIWH(largeW, largeH), paint);
}

DEF_TEST(ImageFilterBlurLargeImage, reporter) {
    auto surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(100, 100)));
    test_large_blur_input(reporter, surface->getCanvas());
}

static void test_make_with_filter(
        skiatest::Reporter* reporter,
        const std::function<sk_sp<SkSurface>(int width, int height)>& createSurface,
        const std::function<sk_sp<SkImage>(sk_sp<SkImage> src,
                                           const SkImageFilter* filter,
                                           const SkIRect& subset,
                                           const SkIRect& clipBounds,
                                           SkIRect* outSubset,
                                           SkIPoint* offset)>& makeWithFilter) {
    sk_sp<SkSurface> surface(createSurface(192, 128));
    surface->getCanvas()->clear(SK_ColorRED);
    SkPaint bluePaint;
    bluePaint.setColor(SK_ColorBLUE);
    SkIRect subset = SkIRect::MakeXYWH(25, 20, 50, 50);
    surface->getCanvas()->drawRect(SkRect::Make(subset), bluePaint);
    sk_sp<SkImage> sourceImage = surface->makeImageSnapshot();

    sk_sp<SkImageFilter> filter = make_grayscale(nullptr, nullptr);
    SkIRect clipBounds = SkIRect::MakeXYWH(30, 35, 100, 100);
    SkIRect outSubset;
    SkIPoint offset;
    sk_sp<SkImage> result;

    result = makeWithFilter(sourceImage, nullptr, subset, clipBounds, &outSubset, &offset);
    REPORTER_ASSERT(reporter, !result);  // filter is required

    result = makeWithFilter(sourceImage, filter.get(), subset, clipBounds, nullptr, &offset);
    REPORTER_ASSERT(reporter, !result);  // outSubset is required

    result = makeWithFilter(sourceImage, filter.get(), subset, clipBounds, &outSubset, nullptr);
    REPORTER_ASSERT(reporter, !result);  // offset is required

    SkIRect bigSubset = SkIRect::MakeXYWH(-10000, -10000, 20000, 20000);
    result = makeWithFilter(sourceImage, filter.get(), bigSubset, clipBounds, &outSubset, &offset);
    REPORTER_ASSERT(reporter, !result);  // subset needs to be w/in source's bounds

    const SkIRect kEmpty = SkIRect::MakeEmpty();
    result = makeWithFilter(sourceImage, filter.get(), kEmpty, clipBounds, &outSubset, &offset);
    REPORTER_ASSERT(reporter, !result);  // subset can't be empty

    result = makeWithFilter(sourceImage, filter.get(), subset, kEmpty, &outSubset, &offset);
    REPORTER_ASSERT(reporter, !result);  // clipBounds can't be empty

    const SkIRect kLeftField = SkIRect::MakeXYWH(-1000, 0, 100, 100);
    result = makeWithFilter(sourceImage, filter.get(), subset, kLeftField, &outSubset, &offset);
    REPORTER_ASSERT(reporter, !result);

    result = makeWithFilter(sourceImage, filter.get(), subset, clipBounds, &outSubset, &offset);

    REPORTER_ASSERT(reporter, result);
    REPORTER_ASSERT(reporter, result->bounds().contains(outSubset));
    SkIRect destRect = SkIRect::MakeXYWH(offset.x(), offset.y(),
                                         outSubset.width(), outSubset.height());
    REPORTER_ASSERT(reporter, clipBounds.contains(destRect));

    // In GPU-mode, this case creates a special image with a backing size that differs from
    // the content size
    {
        clipBounds.setXYWH(0, 0, 170, 100);
        subset.setXYWH(0, 0, 160, 90);

        filter = SkImageFilters::Blend(SkBlendMode::kSrcOver, nullptr);
        result = makeWithFilter(sourceImage, filter.get(), subset, clipBounds, &outSubset, &offset);
        REPORTER_ASSERT(reporter, result);

        // In Ganesh, we want the result image (and all intermediate steps) to have used the same
        // origin as the original surface.
        if (result && as_IB(result)->isGaneshBacked()) {
            SkImage_GaneshBase* base = static_cast<SkImage_GaneshBase*>(result.get());
            REPORTER_ASSERT(reporter, base->origin() == kTestSurfaceOrigin);
        }
    }
}

DEF_TEST(ImageFilterMakeWithFilter, reporter) {
    auto createRasterSurface = [](int width, int height) -> sk_sp<SkSurface> {
        const SkImageInfo info = SkImageInfo::MakeN32(width, height, kOpaque_SkAlphaType);
        return SkSurfaces::Raster(info);
    };

    auto raster = [](sk_sp<SkImage> src,
                     const SkImageFilter* filter,
                     const SkIRect& subset,
                     const SkIRect& clipBounds,
                     SkIRect* outSubset,
                     SkIPoint* offset) -> sk_sp<SkImage> {
                         return SkImages::MakeWithFilter(std::move(src),
                                                         filter,
                                                         subset,
                                                         clipBounds,
                                                         outSubset,
                                                         offset);
                     };

    test_make_with_filter(reporter, createRasterSurface, raster);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageFilterMakeWithFilter_Ganesh,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    GrRecordingContext* rContext = ctxInfo.directContext();

    auto createGaneshSurface = [rContext](int width, int height) -> sk_sp<SkSurface> {
        const SkImageInfo info = SkImageInfo::MakeN32(width, height, kOpaque_SkAlphaType);
        return SkSurfaces::RenderTarget(
                rContext, skgpu::Budgeted::kNo, info, 0, kTestSurfaceOrigin, nullptr);
    };

    auto ganesh = [rContext](sk_sp<SkImage> src,
                             const SkImageFilter* filter,
                             const SkIRect& subset,
                             const SkIRect& clipBounds,
                             SkIRect* outSubset,
                             SkIPoint* offset) -> sk_sp<SkImage> {
         return SkImages::MakeWithFilter(rContext,
                                         std::move(src),
                                         filter,
                                         subset,
                                         clipBounds,
                                         outSubset,
                                         offset);
    };

    test_make_with_filter(reporter, createGaneshSurface, ganesh);
}

#if defined(SK_GRAPHITE)

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(ImageFilterMakeWithFilter_Graphite,
                                         reporter,
                                         context,
                                         CtsEnforcement::kNextRelease) {
    std::unique_ptr<skgpu::graphite::Recorder> recorder =
            context->makeRecorder(ToolUtils::CreateTestingRecorderOptions());

    auto createGraphiteSurface = [r = recorder.get()](int width, int height) -> sk_sp<SkSurface> {
        const SkImageInfo info = SkImageInfo::MakeN32(width, height, kPremul_SkAlphaType);
        return SkSurfaces::RenderTarget(r, info);
    };

    auto graphite = [r = recorder.get()](sk_sp<SkImage> src,
                                         const SkImageFilter* filter,
                                         const SkIRect& subset,
                                         const SkIRect& clipBounds,
                                         SkIRect* outSubset,
                                         SkIPoint* offset) -> sk_sp<SkImage> {
        return SkImages::MakeWithFilter(r,
                                        std::move(src),
                                        filter,
                                        subset,
                                        clipBounds,
                                        outSubset,
                                        offset);
    };

    test_make_with_filter(reporter, createGraphiteSurface, graphite);
}

#endif

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(ImageFilterHugeBlur_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    sk_sp<SkSurface> surf(SkSurfaces::RenderTarget(
            ctxInfo.directContext(), skgpu::Budgeted::kNo, SkImageInfo::MakeN32Premul(100, 100)));

    SkCanvas* canvas = surf->getCanvas();

    test_huge_blur(canvas, reporter);
}

DEF_GANESH_TEST_FOR_RENDERING_CONTEXTS(XfermodeImageFilterCroppedInput_Gpu,
                                       reporter,
                                       ctxInfo,
                                       CtsEnforcement::kNever) {
    sk_sp<SkSurface> surf(SkSurfaces::RenderTarget(
            ctxInfo.directContext(),
            skgpu::Budgeted::kNo,
            SkImageInfo::Make(1, 1, kRGBA_8888_SkColorType, kPremul_SkAlphaType)));

    test_xfermode_cropped_input(surf.get(), reporter);
}

DEF_GANESH_TEST_FOR_ALL_CONTEXTS(ImageFilterBlurLargeImage_Gpu,
                                 reporter,
                                 ctxInfo,
                                 CtsEnforcement::kNever) {
    auto surface(SkSurfaces::RenderTarget(
            ctxInfo.directContext(),
            skgpu::Budgeted::kYes,
            SkImageInfo::Make(100, 100, kRGBA_8888_SkColorType, kPremul_SkAlphaType)));
    test_large_blur_input(reporter, surface->getCanvas());
}

/*
 *  Test that colorfilterimagefilter does not require its CTM to be decomposed when it has more
 *  than just scale/translate, but that other filters do.
 */
DEF_TEST(ImageFilterComplexCTM, reporter) {
    // just need a colorfilter to exercise the corresponding imagefilter
    sk_sp<SkColorFilter> cf = SkColorFilters::Blend(SK_ColorRED, SkBlendMode::kSrcATop);
    sk_sp<SkImageFilter> cfif = SkImageFilters::ColorFilter(cf, nullptr);    // can handle
    sk_sp<SkImageFilter> blif = SkImageFilters::Blur(3, 3, nullptr);         // cannot handle
    using MatrixCapability = SkImageFilter_Base::MatrixCapability;

    struct {
        sk_sp<SkImageFilter> fFilter;
        MatrixCapability     fExpectCapability;
    } recs[] = {
        { cfif,                                  MatrixCapability::kComplex },
        { SkImageFilters::ColorFilter(cf, cfif), MatrixCapability::kComplex },
        { SkImageFilters::Merge(cfif, cfif),     MatrixCapability::kComplex },
        { SkImageFilters::Compose(cfif, cfif),   MatrixCapability::kComplex },

        { blif,                                  MatrixCapability::kScaleTranslate },
        { SkImageFilters::Blur(3, 3, cfif),      MatrixCapability::kScaleTranslate },
        { SkImageFilters::ColorFilter(cf, blif), MatrixCapability::kScaleTranslate },
        { SkImageFilters::Merge(cfif, blif),     MatrixCapability::kScaleTranslate },
        { SkImageFilters::Compose(blif, cfif),   MatrixCapability::kScaleTranslate },
    };

    for (const auto& rec : recs) {
        const MatrixCapability capability = as_IFB(rec.fFilter)->getCTMCapability();
        REPORTER_ASSERT(reporter, capability == rec.fExpectCapability);
    }
}

// Test SkXfermodeImageFilter::filterBounds with different blending modes.
DEF_TEST(XfermodeImageFilterBounds, reporter) {
    SkIRect background_rect = SkIRect::MakeXYWH(0, 0, 100, 100);
    SkIRect foreground_rect = SkIRect::MakeXYWH(50, 50, 100, 100);
    sk_sp<SkImageFilter> background = SkImageFilters::Crop(SkRect::Make(background_rect), nullptr);
    sk_sp<SkImageFilter> foreground = SkImageFilters::Crop(SkRect::Make(foreground_rect), nullptr);

    SkIRect expectedBounds[kSkBlendModeCount];
    // Expect union of input rects by default.
    for (int i = 0; i < kSkBlendModeCount; ++i) {
        expectedBounds[i] = background_rect;
        expectedBounds[i].join(foreground_rect);
    }

    SkIRect intersection = background_rect;
    intersection.intersect(foreground_rect);
    expectedBounds[static_cast<int>(SkBlendMode::kClear)] = SkIRect::MakeEmpty();
    expectedBounds[static_cast<int>(SkBlendMode::kSrc)] = foreground_rect;
    expectedBounds[static_cast<int>(SkBlendMode::kDst)] = background_rect;
    expectedBounds[static_cast<int>(SkBlendMode::kSrcIn)] = intersection;
    expectedBounds[static_cast<int>(SkBlendMode::kDstIn)] = intersection;
    expectedBounds[static_cast<int>(SkBlendMode::kSrcOut)] = foreground_rect;
    expectedBounds[static_cast<int>(SkBlendMode::kDstOut)] = background_rect;
    expectedBounds[static_cast<int>(SkBlendMode::kSrcATop)] = background_rect;
    expectedBounds[static_cast<int>(SkBlendMode::kDstATop)] = foreground_rect;
    expectedBounds[static_cast<int>(SkBlendMode::kModulate)] = intersection;

    // Use a very large input bounds so that the crop rects stored in 'background' and 'foreground'
    // aren't restricted.
    SkIRect src = SkRectPriv::MakeILarge();
    for (int i = 0; i < kSkBlendModeCount; ++i) {
        sk_sp<SkImageFilter> xfermode(SkImageFilters::Blend(static_cast<SkBlendMode>(i),
                                                            background, foreground, nullptr));
        auto bounds = xfermode->filterBounds(src, SkMatrix::I(),
                                             SkImageFilter::kForward_MapDirection, nullptr);
        REPORTER_ASSERT(reporter, bounds == expectedBounds[i]);
    }

    // Test empty intersection.
    sk_sp<SkImageFilter> background2 =
            SkImageFilters::Crop(SkRect::MakeXYWH(0, 0, 20, 20), nullptr);
    sk_sp<SkImageFilter> foreground2 =
            SkImageFilters::Crop(SkRect::MakeXYWH(40, 40, 50, 50), nullptr);
    sk_sp<SkImageFilter> xfermode(SkImageFilters::Blend(
            SkBlendMode::kSrcIn, std::move(background2), std::move(foreground2), nullptr));
    auto bounds = xfermode->filterBounds(src, SkMatrix::I(),
                                         SkImageFilter::kForward_MapDirection, nullptr);
    REPORTER_ASSERT(reporter, bounds.isEmpty());
}

DEF_TEST(OffsetImageFilterBounds, reporter) {
    const SkIRect src = SkIRect::MakeXYWH(0, 0, 100, 100);
    const SkVector srcOffset = {-50.5f, -50.5f};
    sk_sp<SkImageFilter> offset(SkImageFilters::Offset(srcOffset.fX, srcOffset.fY, nullptr));

    // Because the offset has a fractional component, the final output and required input bounds
    // will be rounded out to include an extra pixel.
    SkIRect expectedForward = SkRect::Make(src).makeOffset(srcOffset.fX, srcOffset.fY).roundOut();
    SkIRect boundsForward = offset->filterBounds(src, SkMatrix::I(),
                                                 SkImageFilter::kForward_MapDirection, nullptr);
    REPORTER_ASSERT(reporter, boundsForward == expectedForward);

    SkIRect expectedReverse = SkRect::Make(src).makeOffset(-srcOffset.fX, -srcOffset.fY).roundOut();

    // Intersect 'expectedReverse' with the source because we are passing &src in as the known
    // input bounds, which is the bounds of non-transparent pixels that can be moved by the offset.
    // While the ::Offset filter could show all pixels inside 'expectedReverse' given that 'src'
    // is also the target device output of the filter, the required input can be made tighter.
    SkAssertResult(expectedReverse.intersect(src));

    SkIRect boundsReverse = offset->filterBounds(src, SkMatrix::I(),
                                                 SkImageFilter::kReverse_MapDirection, &src);
    REPORTER_ASSERT(reporter, boundsReverse == expectedReverse);
}

DEF_TEST(OffsetImageFilterBoundsNoOverflow, reporter) {
    const SkIRect src = SkIRect::MakeXYWH(-10.f, -10.f, 20.f, 20.f);
    const SkScalar bigOffset = SkIntToScalar(std::numeric_limits<int>::max()) * 2.f / 3.f;

    sk_sp<SkImageFilter> filter =
            SkImageFilters::Blend(SkBlendMode::kSrcOver,
                                  SkImageFilters::Offset(-bigOffset, -bigOffset, nullptr),
                                  SkImageFilters::Offset(bigOffset, bigOffset, nullptr));
    SkIRect boundsForward = filter->filterBounds(src, SkMatrix::I(),
                                                 SkImageFilter::kForward_MapDirection, nullptr);
    // NOTE: isEmpty() will return true even if the l/r or t/b didn't overflow but the dimensions
    // would overflow an int32. However, when isEmpty64() is false, it means the actual edge coords
    // are valid, which is good enough for our purposes (and gfx::Rect has its own strategies for
    // ensuring such a rectangle doesn't get accidentally treated as empty during chromium's
    // conversions).
    REPORTER_ASSERT(reporter, !boundsForward.isEmpty64());

    // When querying with unbounded input content, it should not overflow and should not be empty.
    SkIRect boundsReverse = filter->filterBounds(src, SkMatrix::I(),
                                                 SkImageFilter::kReverse_MapDirection, nullptr);
    REPORTER_ASSERT(reporter, !boundsReverse.isEmpty64());

    // However in this case, when 'src' is also passed as the content bounds, the ::Offset() filters
    // detect that they would be transparent black. This propagates up to the src-over blend and
    // the entire graph is identified as empty.
    boundsReverse = filter->filterBounds(src, SkMatrix::I(),
                                         SkImageFilter::kReverse_MapDirection, &src);
    REPORTER_ASSERT(reporter, boundsReverse.isEmpty64());
}

static void test_arithmetic_bounds(skiatest::Reporter* reporter, float k1, float k2, float k3,
                                   float k4, sk_sp<SkImageFilter> background,
                                   sk_sp<SkImageFilter> foreground,
                                   const SkIRect* crop, const SkIRect& expected) {
    sk_sp<SkImageFilter> arithmetic(
            SkImageFilters::Arithmetic(k1, k2, k3, k4, false, background, foreground, crop));
    // Use a very large input bounds so that the crop rects stored in 'background' and 'foreground'
    // aren't restricted.
    SkIRect src = SkRectPriv::MakeILarge();
    SkIRect bounds = arithmetic->filterBounds(src, SkMatrix::I(),
                                              SkImageFilter::kForward_MapDirection, nullptr);
    REPORTER_ASSERT(reporter, expected == bounds);
}

static void test_arithmetic_combinations(skiatest::Reporter* reporter, float v) {
    SkIRect bgRect = SkIRect::MakeXYWH(0, 0, 100, 100);
    SkIRect fgRect = SkIRect::MakeXYWH(50, 50, 100, 100);
    sk_sp<SkImageFilter> background = SkImageFilters::Crop(SkRect::Make(bgRect), nullptr);
    sk_sp<SkImageFilter> foreground = SkImageFilters::Crop(SkRect::Make(fgRect), nullptr);

    SkIRect unionRect = bgRect;
    unionRect.join(fgRect);
    SkIRect intersection = bgRect;
    intersection.intersect(fgRect);

    // Test with crop. When k4 is non-zero, the result is expected to be cropRect
    // regardless of inputs because the filter affects the whole crop area. When there is no crop
    // rect, it should report an effectively infinite output.
    static const SkIRect kInf = SkRectPriv::MakeILarge();
    test_arithmetic_bounds(reporter, 0, 0, 0, 0, background, foreground, nullptr,
                           SkIRect::MakeEmpty());
    test_arithmetic_bounds(reporter, 0, 0, 0, v, background, foreground, nullptr, kInf);
    test_arithmetic_bounds(reporter, 0, 0, v, 0, background, foreground, nullptr, bgRect);
    test_arithmetic_bounds(reporter, 0, 0, v, v, background, foreground, nullptr, kInf);
    test_arithmetic_bounds(reporter, 0, v, 0, 0, background, foreground, nullptr, fgRect);
    test_arithmetic_bounds(reporter, 0, v, 0, v, background, foreground, nullptr, kInf);
    test_arithmetic_bounds(reporter, 0, v, v, 0, background, foreground, nullptr, unionRect);
    test_arithmetic_bounds(reporter, 0, v, v, v, background, foreground, nullptr, kInf);
    test_arithmetic_bounds(reporter, v, 0, 0, 0, background, foreground, nullptr, intersection);
    test_arithmetic_bounds(reporter, v, 0, 0, v, background, foreground, nullptr, kInf);
    test_arithmetic_bounds(reporter, v, 0, v, 0, background, foreground, nullptr, bgRect);
    test_arithmetic_bounds(reporter, v, 0, v, v, background, foreground, nullptr, kInf);
    test_arithmetic_bounds(reporter, v, v, 0, 0, background, foreground, nullptr, fgRect);
    test_arithmetic_bounds(reporter, v, v, 0, v, background, foreground, nullptr, kInf);
    test_arithmetic_bounds(reporter, v, v, v, 0, background, foreground, nullptr, unionRect);
    test_arithmetic_bounds(reporter, v, v, v, v, background, foreground, nullptr, kInf);

    SkIRect cropRect = SkIRect::MakeXYWH(-111, -222, 333, 444);
    test_arithmetic_bounds(reporter, 0, 0, 0, 0, background, foreground, &cropRect,
                           SkIRect::MakeEmpty());
    test_arithmetic_bounds(reporter, 0, 0, 0, v, background, foreground, &cropRect, cropRect);
    test_arithmetic_bounds(reporter, 0, 0, v, 0, background, foreground, &cropRect, bgRect);
    test_arithmetic_bounds(reporter, 0, 0, v, v, background, foreground, &cropRect, cropRect);
    test_arithmetic_bounds(reporter, 0, v, 0, 0, background, foreground, &cropRect, fgRect);
    test_arithmetic_bounds(reporter, 0, v, 0, v, background, foreground, &cropRect, cropRect);
    test_arithmetic_bounds(reporter, 0, v, v, 0, background, foreground, &cropRect, unionRect);
    test_arithmetic_bounds(reporter, 0, v, v, v, background, foreground, &cropRect, cropRect);
    test_arithmetic_bounds(reporter, v, 0, 0, 0, background, foreground, &cropRect, intersection);
    test_arithmetic_bounds(reporter, v, 0, 0, v, background, foreground, &cropRect, cropRect);
    test_arithmetic_bounds(reporter, v, 0, v, 0, background, foreground, &cropRect, bgRect);
    test_arithmetic_bounds(reporter, v, 0, v, v, background, foreground, &cropRect, cropRect);
    test_arithmetic_bounds(reporter, v, v, 0, 0, background, foreground, &cropRect, fgRect);
    test_arithmetic_bounds(reporter, v, v, 0, v, background, foreground, &cropRect, cropRect);
    test_arithmetic_bounds(reporter, v, v, v, 0, background, foreground, &cropRect, unionRect);
    test_arithmetic_bounds(reporter, v, v, v, v, background, foreground, &cropRect, cropRect);
}

// Test SkArithmeticImageFilter::filterBounds with different blending modes.
DEF_TEST(ArithmeticImageFilterBounds, reporter) {
    test_arithmetic_combinations(reporter, 1);
    test_arithmetic_combinations(reporter, 0.5);
}

// Test SkDisplacementMapEffect::filterBounds.
DEF_TEST(DisplacementMapBounds, reporter) {
    SkIRect floodBounds(SkIRect::MakeXYWH(20, 30, 10, 10));
    sk_sp<SkImageFilter> flood(SkImageFilters::Shader(SkShaders::Color(SK_ColorGREEN),
                                                      &floodBounds));
    SkIRect tilingBounds(SkIRect::MakeXYWH(0, 0, 200, 100));
    sk_sp<SkImageFilter> tiling(SkImageFilters::Tile(SkRect::Make(floodBounds),
                                                     SkRect::Make(tilingBounds),
                                                     flood));
    sk_sp<SkImageFilter> displace(SkImageFilters::DisplacementMap(SkColorChannel::kR,
                                                                  SkColorChannel::kB,
                                                                  20.0f, nullptr, tiling));

    // The filter graph rooted at 'displace' uses the dynamic source image for the displacement
    // component of ::DisplacementMap, modifying the color component produced by the ::Tile. The
    // output of the tiling filter will be 'tilingBounds', regardless of its input, so 'floodBounds'
    // has no effect on the output. Since 'tiling' doesn't reference any dynamic source image, it
    // also will not affect the required input bounds. The displacement map is sampled 1-to-1
    // with the output pixels, and covers the output unless the color's output makes that impossible
    // and the output is a subset of the desired output. Thus, the displacement can impact the
    // reported output bounds.
    SkIRect input(SkIRect::MakeXYWH(20, 30, 40, 50));

    // 'input' is the desired output, which directly constrains the displacement component in this
    // specific filter graph.
    SkIRect actualInput = displace->filterBounds(input, SkMatrix::I(),
                                                 SkImageFilter::kReverse_MapDirection);
    REPORTER_ASSERT(reporter, input == actualInput);

    // 'input' is the content bounds, which don't affect output bounds because it's only referenced
    // by the displacement component and not the color component.
    SkIRect actualOutput = displace->filterBounds(input, SkMatrix::I(),
                                                  SkImageFilter::kForward_MapDirection);
    REPORTER_ASSERT(reporter, tilingBounds.makeOutset(10, 10) == actualOutput);
}

// Test SkImageSource::filterBounds.
DEF_TEST(ImageSourceBounds, reporter) {
    sk_sp<SkImage> image(make_gradient_circle(64, 64).asImage());
    // Default src and dst rects.
    sk_sp<SkImageFilter> source1(SkImageFilters::Image(image, SkFilterMode::kNearest));
    SkIRect imageBounds = SkIRect::MakeWH(64, 64);
    SkIRect input(SkIRect::MakeXYWH(10, 20, 30, 40));
    REPORTER_ASSERT(reporter,
                    imageBounds == source1->filterBounds(input, SkMatrix::I(),
                                                         SkImageFilter::kForward_MapDirection,
                                                         nullptr));
    REPORTER_ASSERT(reporter,
                    source1->filterBounds(input, SkMatrix::I(),
                                          SkImageFilter::kReverse_MapDirection, &input).isEmpty());
    SkMatrix scale(SkMatrix::Scale(2, 2));
    SkIRect scaledBounds = SkIRect::MakeWH(128, 128);
    REPORTER_ASSERT(reporter,
                    scaledBounds == source1->filterBounds(input, scale,
                                                          SkImageFilter::kForward_MapDirection,
                                                          nullptr));
    REPORTER_ASSERT(reporter,
                    source1->filterBounds(input, scale,
                                          SkImageFilter::kReverse_MapDirection, &input).isEmpty());

    // Specified src and dst rects (which are outside available pixels).
    SkRect src(SkRect::MakeXYWH(0.5, 0.5, 100.5, 100.5));
    SkRect dst(SkRect::MakeXYWH(-10.5, -10.5, 120.5, 120.5));
    sk_sp<SkImageFilter> source2(SkImageFilters::Image(image, src, dst,
                                                       SkSamplingOptions(SkFilterMode::kLinear,
                                                                         SkMipmapMode::kLinear)));

    SkRect clippedSrc = src;
    SkAssertResult(clippedSrc.intersect(SkRect::Make(image->dimensions())));
    SkRect clippedDst = SkMatrix::RectToRect(src, dst).mapRect(clippedSrc);

    REPORTER_ASSERT(reporter,
                    clippedDst.roundOut() ==
                    source2->filterBounds(input, SkMatrix::I(),
                                          SkImageFilter::kForward_MapDirection, nullptr));
    REPORTER_ASSERT(reporter,
                    source2->filterBounds(input, SkMatrix::I(),
                                          SkImageFilter::kReverse_MapDirection, &input).isEmpty());
    scale.mapRect(&clippedDst);
    scale.mapRect(&clippedSrc);
    REPORTER_ASSERT(reporter,
                    clippedDst.roundOut() ==
                    source2->filterBounds(input, scale,
                                          SkImageFilter::kForward_MapDirection, nullptr));
    REPORTER_ASSERT(reporter,
                    source2->filterBounds(input, scale,
                                          SkImageFilter::kReverse_MapDirection, &input).isEmpty());
}

// Test SkPictureImageFilter::filterBounds.
DEF_TEST(PictureImageSourceBounds, reporter) {
    SkPictureRecorder recorder;
    SkCanvas* recordingCanvas = recorder.beginRecording(64, 64);

    SkPaint greenPaint;
    greenPaint.setColor(SK_ColorGREEN);
    recordingCanvas->drawRect(SkRect::Make(SkIRect::MakeXYWH(10, 10, 30, 20)), greenPaint);
    sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

    // Default target rect.
    sk_sp<SkImageFilter> source1(SkImageFilters::Picture(picture));
    SkIRect pictureBounds = SkIRect::MakeWH(64, 64);
    SkIRect input(SkIRect::MakeXYWH(10, 20, 30, 40));
    REPORTER_ASSERT(reporter,
                    pictureBounds == source1->filterBounds(input, SkMatrix::I(),
                                                           SkImageFilter::kForward_MapDirection,
                                                           nullptr));
    REPORTER_ASSERT(reporter,
                    source1->filterBounds(input, SkMatrix::I(),
                                          SkImageFilter::kReverse_MapDirection, &input).isEmpty());
    SkMatrix scale(SkMatrix::Scale(2, 2));
    SkIRect scaledPictureBounds = SkIRect::MakeWH(128, 128);
    REPORTER_ASSERT(reporter,
                    scaledPictureBounds == source1->filterBounds(input, scale,
                                                                 SkImageFilter::kForward_MapDirection,
                                                                 nullptr));
    REPORTER_ASSERT(reporter,
                    source1->filterBounds(input, scale,
                                          SkImageFilter::kReverse_MapDirection, &input).isEmpty());

    // Specified target rect.
    SkRect targetRect(SkRect::MakeXYWH(9.5, 9.5, 31, 21));
    sk_sp<SkImageFilter> source2(SkImageFilters::Picture(picture, targetRect));
    REPORTER_ASSERT(reporter,
                    targetRect.roundOut() == source2->filterBounds(input, SkMatrix::I(),
                                                                   SkImageFilter::kForward_MapDirection,
                                                                   nullptr));
    REPORTER_ASSERT(reporter,
                    source2->filterBounds(input, SkMatrix::I(),
                                          SkImageFilter::kReverse_MapDirection, &input).isEmpty());
    scale.mapRect(&targetRect);
    REPORTER_ASSERT(reporter,
                    targetRect.roundOut() == source2->filterBounds(input, scale,
                                                                   SkImageFilter::kForward_MapDirection,
                                                                   nullptr));
    REPORTER_ASSERT(reporter,
                    source2->filterBounds(input, scale,
                                          SkImageFilter::kReverse_MapDirection, &input).isEmpty());
}

DEF_TEST(DropShadowImageFilter_Huge, reporter) {
    // Successful if it doesn't crash or trigger ASAN. (crbug.com/1264705)
    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(300, 150));

    SkPaint paint;
    paint.setImageFilter(SkImageFilters::DropShadowOnly(
            0.0f, 0.437009f, 14129.6f, 14129.6f, SK_ColorGRAY, nullptr));

    surf->getCanvas()->saveLayer(nullptr, &paint);
    surf->getCanvas()->restore();
}
