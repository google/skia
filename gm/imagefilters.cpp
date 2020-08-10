/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkColorMatrix.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkHighContrastFilter.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkShaderMaskFilter.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkHalf.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <utility>

/**
 *  Test drawing a primitive w/ an imagefilter (in this case, just matrix w/ identity) to see
 *  that we apply the xfermode *after* the image has been created and filtered, and not during
 *  the creation step (i.e. before it is filtered).
 *
 *  see https://bug.skia.org/3741
 */
static void do_draw(SkCanvas* canvas, SkBlendMode mode, sk_sp<SkImageFilter> imf) {
        SkAutoCanvasRestore acr(canvas, true);
        canvas->clipRect(SkRect::MakeWH(220, 220));

        // want to force a layer, so modes like DstIn can combine meaningfully, but the final
        // image can still be shown against our default (opaque) background. non-opaque GMs
        // are a lot more trouble to compare/triage.
        canvas->saveLayer(nullptr, nullptr);
        canvas->drawColor(SK_ColorGREEN);

        SkPaint paint;
        paint.setAntiAlias(true);

        SkRect r0 = SkRect::MakeXYWH(10, 60, 200, 100);
        SkRect r1 = SkRect::MakeXYWH(60, 10, 100, 200);

        paint.setColor(SK_ColorRED);
        canvas->drawOval(r0, paint);

        paint.setColor(0x660000FF);
        paint.setImageFilter(std::move(imf));
        paint.setBlendMode(mode);
        canvas->drawOval(r1, paint);
}

DEF_SIMPLE_GM(imagefilters_xfermodes, canvas, 480, 480) {
        canvas->translate(10, 10);

        // just need an imagefilter to trigger the code-path (which creates a tmp layer)
        sk_sp<SkImageFilter> imf(SkImageFilters::MatrixTransform(SkMatrix::I(),
                                                                 kNone_SkFilterQuality,
                                                                 nullptr));

        const SkBlendMode modes[] = {
            SkBlendMode::kSrcATop, SkBlendMode::kDstIn
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(modes); ++i) {
            canvas->save();
            do_draw(canvas, modes[i], nullptr);
            canvas->translate(240, 0);
            do_draw(canvas, modes[i], imf);
            canvas->restore();

            canvas->translate(0, 240);
        }
}

static sk_sp<SkImage> make_image(SkCanvas* canvas) {
    const SkImageInfo info = SkImageInfo::MakeS32(100, 100, kPremul_SkAlphaType);
    auto              surface(ToolUtils::makeSurface(canvas, info));
    surface->getCanvas()->drawRect(SkRect::MakeXYWH(25, 25, 50, 50), SkPaint());
    return surface->makeImageSnapshot();
}

// Compare blurs when we're tightly clipped (fast) and not as tightly (slower)
//
// Expect the two to draw the same (modulo the extra border of pixels when the clip is larger)
//
DEF_SIMPLE_GM(fast_slow_blurimagefilter, canvas, 620, 260) {
    sk_sp<SkImage> image(make_image(canvas));
    const SkRect r = SkRect::MakeIWH(image->width(), image->height());

    canvas->translate(10, 10);
    for (SkScalar sigma = 8; sigma <= 128; sigma *= 2) {
        SkPaint paint;
        paint.setImageFilter(SkImageFilters::Blur(sigma, sigma, nullptr));

        canvas->save();
        // we outset the clip by 1, to fall out of the fast-case in drawImage
        // i.e. the clip is larger than the image
        for (SkScalar outset = 0; outset <= 1; ++outset) {
            canvas->save();
            canvas->clipRect(r.makeOutset(outset, outset));
            canvas->drawImage(image, 0, 0, &paint);
            canvas->restore();
            canvas->translate(0, r.height() + 20);
        }
        canvas->restore();
        canvas->translate(r.width() + 20, 0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static void draw_set(SkCanvas* canvas, sk_sp<SkImageFilter> filters[], int count) {
    const SkRect r = SkRect::MakeXYWH(30, 30, 200, 200);
    const SkScalar offset = 250;
    SkScalar dx = 0, dy = 0;

    for (int i = 0; i < count; ++i) {
        canvas->save();
        SkRRect rr = SkRRect::MakeRectXY(r.makeOffset(dx, dy), 20, 20);
        canvas->clipRRect(rr, true);
        canvas->saveLayer(SkCanvas::SaveLayerRec(&rr.getBounds(), nullptr, filters[i].get(), 0));
        canvas->drawColor(0x40FFFFFF);
        canvas->restore();
        canvas->restore();

        if (0 == dx) {
            dx = offset;
        } else {
            dx = 0;
            dy = offset;
        }
    }
}

class SaveLayerWithBackdropGM : public skiagm::GM {
protected:
    bool runAsBench() const override { return true; }
    SkString onShortName() override { return SkString("savelayer_with_backdrop"); }
    SkISize onISize() override { return SkISize::Make(830, 550); }

    void onDraw(SkCanvas* canvas) override {
        SkColorMatrix cm;
        cm.setSaturation(10);
        sk_sp<SkColorFilter> cf(SkColorFilters::Matrix(cm));
        const SkScalar kernel[] = { 4, 0, 4, 0, -15, 0, 4, 0, 4 };
        sk_sp<SkImageFilter> filters[] = {
            SkImageFilters::Blur(10, 10, nullptr),
            SkImageFilters::Dilate(8, 8, nullptr),
            SkImageFilters::MatrixConvolution({ 3, 3 }, kernel, 1, 0, { 0, 0 },
                                              SkTileMode::kDecal, true, nullptr),
            SkImageFilters::ColorFilter(std::move(cf), nullptr),
        };

        const struct {
            SkScalar    fSx, fSy, fTx, fTy;
        } xforms[] = {
            { 1, 1, 0, 0 },
            { 0.5f, 0.5f, 530, 0 },
            { 0.25f, 0.25f, 530, 275 },
            { 0.125f, 0.125f, 530, 420 },
        };

        SkPaint paint;
        paint.setFilterQuality(kMedium_SkFilterQuality);
        sk_sp<SkImage> image(GetResourceAsImage("images/mandrill_512.png"));

        canvas->translate(20, 20);
        for (const auto& xform : xforms) {
            canvas->save();
            canvas->translate(xform.fTx, xform.fTy);
            canvas->scale(xform.fSx, xform.fSy);
            canvas->drawImage(image, 0, 0, &paint);
            draw_set(canvas, filters, SK_ARRAY_COUNT(filters));
            canvas->restore();
        }
    }
};

DEF_GM(return new SaveLayerWithBackdropGM();)

///////////////////////////////////////////////////////////////////////////////////////////////////

// Test that color filters and mask filters are applied before the image filter, even if it would
// normally be a sprite draw that could avoid an auto-saveLayer.
DEF_SIMPLE_GM(imagefilters_effect_order, canvas, 512, 512) {
    sk_sp<SkImage> image(GetResourceAsImage("images/mandrill_256.png"));
    auto direct = GrAsDirectContext(canvas->recordingContext());
    if (direct) {
        if (sk_sp<SkImage> gpuImage = image->makeTextureImage(direct)) {
            image = std::move(gpuImage);
        }
    }

    SkISize kernelSize = SkISize::Make(3, 3);
    SkIPoint kernelOffset = SkIPoint::Make(1, 1);
    // A Laplacian edge detector, ie https://en.wikipedia.org/wiki/Kernel_(image_processing)
    SkScalar kernel[9] = {-1.f, -1.f, -1.f,
                          -1.f,  8.f, -1.f,
                          -1.f, -1.f, -1.f};
    auto edgeDetector = SkImageFilters::MatrixConvolution(
            kernelSize, kernel, 1.f, 0.f, kernelOffset, SkTileMode::kClamp, false, nullptr);
    // This uses the high contrast filter because it resembles a pre-processing step you may perform
    // prior to edge detection. The specifics of the high contrast algorithm don't matter for the GM
    auto edgeAmplify = SkHighContrastFilter::Make(
            {false, SkHighContrastConfig::InvertStyle::kNoInvert, 0.5f});

    SkPaint testCFPaint;
    testCFPaint.setColorFilter(edgeAmplify);
    testCFPaint.setImageFilter(edgeDetector);

    // The expected result is color filter then image filter, so represent this explicitly in the
    // image filter graph.
    SkPaint expectedCFPaint;
    expectedCFPaint.setImageFilter(SkImageFilters::Compose(edgeDetector,
            SkImageFilters::ColorFilter(edgeAmplify, nullptr)));

    // Draw the image twice (expected on the left, test on the right that should match)
    SkRect crop = SkRect::Make(image->bounds());
    canvas->save();
    canvas->clipRect(crop);
    canvas->drawImage(image, 0, 0, &expectedCFPaint); // Filter applied by draw's SkPaint
    canvas->restore();

    canvas->save();
    canvas->translate(image->width(), 0);
    canvas->clipRect(crop);
    canvas->drawImage(image, 0, 0, &testCFPaint);
    canvas->restore();

    // Now test mask filters. These should be run before the image filter, and thus have the same
    // effect as multiplying by an alpha mask.

    // This mask filter pokes a hole in the center of the image
    static constexpr SkColor kAlphas[] = { SK_ColorBLACK, SK_ColorTRANSPARENT };
    static constexpr SkScalar kPos[] = { 0.4f, 0.9f };
    sk_sp<SkShader> alphaMaskShader = SkGradientShader::MakeRadial(
            {128.f, 128.f}, 128.f, kAlphas, kPos, 2, SkTileMode::kClamp);
    sk_sp<SkMaskFilter> maskFilter = SkShaderMaskFilter::Make(alphaMaskShader);

    // If edge detector sees the mask filter, it'll have alpha and then blend with the original
    // image; otherwise the mask filter will apply late (incorrectly) and none of the original
    // image will be visible.
    sk_sp<SkImageFilter> edgeBlend = SkImageFilters::Xfermode(SkBlendMode::kSrcOver,
            SkImageFilters::Image(image), edgeDetector);

    SkPaint testMaskPaint;
    testMaskPaint.setMaskFilter(maskFilter);
    testMaskPaint.setImageFilter(edgeBlend);

    SkPaint alphaPaint;
    alphaPaint.setShader(alphaMaskShader);
    SkPaint expectedMaskPaint;
    expectedMaskPaint.setImageFilter(SkImageFilters::Compose(edgeBlend,
            SkImageFilters::Xfermode(SkBlendMode::kSrcIn,
                                     SkImageFilters::Paint(alphaPaint))));

    canvas->save();
    canvas->translate(0, image->height());
    canvas->clipRect(crop);
    canvas->drawImage(image, 0, 0, &expectedMaskPaint);
    canvas->restore();

    canvas->save();
    canvas->translate(image->width(), image->height());
    canvas->clipRect(crop);
    canvas->drawImage(image, 0, 0, &testMaskPaint);
    canvas->restore();
}



DEF_SIMPLE_GM(imagefilters_shader_colorspace, canvas, 64, 256) {
    // Should hopefully match Android's Bitmap's getPixel() and getColor() implementations.
    auto getPixel = [](const SkImage* img, int x, int y) {
        SkImageInfo ii = SkImageInfo::Make({1, 1}, {kBGRA_8888_SkColorType, kUnpremul_SkAlphaType,
                                                    SkColorSpace::MakeSRGB()});
        SkColor color;
        img->readPixels(ii, &color, ii.minRowBytes(), x, y);
        return SkColor4f::FromColor(color);
    };
    auto getColor = [](const SkImage* img, int x, int y) {
        SkImageInfo ii = SkImageInfo::Make({1, 1}, {kRGBA_F16_SkColorType, kUnpremul_SkAlphaType,
                                                    img->imageInfo().refColorSpace()});
        uint64_t rgba;
        img->readPixels(ii, &rgba, ii.minRowBytes(), x, y);
        Sk4f color = SkHalfToFloat_finite_ftz(rgba);
        return SkColor4f{color[0], color[1], color[2], color[3]};
    };

    sk_sp<SkColorSpace> p3 = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                   SkNamedGamut::kDisplayP3);
    SkColor4f colors[] = {{1.f, 0.f, 0.f, 1.f},
                          {0.f, 1.f, 0.f, 1.f},
                          {0.f, 0.f, 1.f, 1.f}};
    SkPoint pts[] = {{0,0},{0,40}};

    SkPaint pShader;
    pShader.setShader(SkGradientShader::MakeLinear(pts, colors, p3, nullptr, 3,
                                                   SkTileMode::kClamp));
    SkPaint pFilter;
    pFilter.setImageFilter(SkImageFilters::Xfermode(SkBlendMode::kDstIn,
                                                    SkImageFilters::Paint(pShader)));

    SkImageInfo fp16sRGB = SkImageInfo::Make({40, 40}, {kRGBA_F16_SkColorType,
                                                        kPremul_SkAlphaType,
                                                        SkColorSpace::MakeSRGB()});

    sk_sp<SkSurface> shaderSurface = canvas->makeSurface(fp16sRGB);
    shaderSurface->getCanvas()->drawPaint(pShader);
    sk_sp<SkImage> shaderResult = shaderSurface->makeImageSnapshot();

    sk_sp<SkSurface> filterSurface = canvas->makeSurface(fp16sRGB);
    filterSurface->getCanvas()->drawPaint(pFilter);
    sk_sp<SkImage> filterResult = filterSurface->makeImageSnapshot();

    // Draw the two versions to the screen
    canvas->drawImage(shaderResult, 5, 5, nullptr);
    canvas->drawImage(filterResult, 50, 5, nullptr);

    // Simulate the pixel comparisons in the linear gradient CTS test
    auto colorsEqual = [](const SkColor4f& c1, const SkColor4f& c2) {
        bool eq = SkScalarNearlyEqual(c1.fR, c2.fR, 0.09f) &&
                  SkScalarNearlyEqual(c1.fG, c2.fG, 0.09f) &&
                  SkScalarNearlyEqual(c1.fB, c2.fB, 0.09f) &&
                  SkScalarNearlyEqual(c1.fA, c2.fA, 0.09f);
        SkDebugf("Colors equal? %d\n"
                    "   expected [%.3f %.3f %.3f %.3f], \n"
                    "    and got [%.3f %.3f %.3f %.3f]\n",
                    eq, c1.fR, c1.fG, c1.fB, c1.fA, c2.fR, c2.fG, c2.fB, c2.fA);
        return eq;
    };

    SkPaint statusPaint;
    statusPaint.setAntiAlias(false);
    statusPaint.setStyle(SkPaint::kStroke_Style);

    SkDebugf("Checking with getPixel (8888):\n");
    bool pixelEqual = colorsEqual(getPixel(shaderResult.get(), 0, 20),
                                  getPixel(filterResult.get(), 0, 20));
    statusPaint.setColor(pixelEqual ? SK_ColorGREEN : SK_ColorRED);
    canvas->drawRect(SkRect::MakeLTRB(3, 3, 92, 47), statusPaint);

    SkDebugf("Checking with getColor (F16):\n");
    bool colorEqual = colorsEqual(getColor(shaderResult.get(), 0, 20),
                                  getColor(filterResult.get(), 0, 20));
    statusPaint.setColor(colorEqual ? SK_ColorGREEN : SK_ColorRED);
    canvas->drawRect(SkRect::MakeLTRB(1, 1, 94, 49), statusPaint);
}
