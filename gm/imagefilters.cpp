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
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "tools/DecodeUtils.h"
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
                                                                 SkSamplingOptions(),
                                                                 nullptr));

        const SkBlendMode modes[] = {
            SkBlendMode::kSrcATop, SkBlendMode::kDstIn
        };

        for (size_t i = 0; i < std::size(modes); ++i) {
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
            canvas->drawImage(image, 0, 0, SkSamplingOptions(), &paint);
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
    SkString getName() const override { return SkString("savelayer_with_backdrop"); }
    SkISize getISize() override { return SkISize::Make(830, 550); }

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

        SkSamplingOptions sampling(SkFilterMode::kLinear,
                                   SkMipmapMode::kLinear);
        sk_sp<SkImage> image(ToolUtils::GetResourceAsImage("images/mandrill_512.png"));

        canvas->translate(20, 20);
        for (const auto& xform : xforms) {
            canvas->save();
            canvas->translate(xform.fTx, xform.fTy);
            canvas->scale(xform.fSx, xform.fSy);
            canvas->drawImage(image, 0, 0, sampling, nullptr);
            draw_set(canvas, filters, std::size(filters));
            canvas->restore();
        }
    }
};

DEF_GM(return new SaveLayerWithBackdropGM();)

///////////////////////////////////////////////////////////////////////////////////////////////////

// Test that color filters and mask filters are applied before the image filter, even if it would
// normally be a sprite draw that could avoid an auto-saveLayer.
DEF_SIMPLE_GM(imagefilters_effect_order, canvas, 512, 512) {
    sk_sp<SkImage> image(ToolUtils::GetResourceAsImage("images/mandrill_256.png"));
    auto direct = GrAsDirectContext(canvas->recordingContext());
    if (direct) {
        if (sk_sp<SkImage> gpuImage = SkImages::TextureFromImage(direct, image)) {
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
    canvas->drawImage(image, 0, 0, SkSamplingOptions(), &expectedCFPaint); // Filter applied by draw's SkPaint
    canvas->restore();

    canvas->save();
    canvas->translate(image->width(), 0);
    canvas->clipRect(crop);
    canvas->drawImage(image, 0, 0, SkSamplingOptions(), &testCFPaint);
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
    sk_sp<SkImageFilter> edgeBlend = SkImageFilters::Blend(SkBlendMode::kSrcOver,
            SkImageFilters::Image(image, SkFilterMode::kNearest), edgeDetector);

    SkPaint testMaskPaint;
    testMaskPaint.setMaskFilter(maskFilter);
    testMaskPaint.setImageFilter(edgeBlend);

    SkPaint expectedMaskPaint;
    expectedMaskPaint.setImageFilter(SkImageFilters::Compose(edgeBlend,
            SkImageFilters::Blend(SkBlendMode::kSrcIn,
                                  SkImageFilters::Shader(alphaMaskShader))));

    canvas->save();
    canvas->translate(0, image->height());
    canvas->clipRect(crop);
    canvas->drawImage(image, 0, 0, SkSamplingOptions(), &expectedMaskPaint);
    canvas->restore();

    canvas->save();
    canvas->translate(image->width(), image->height());
    canvas->clipRect(crop);
    canvas->drawImage(image, 0, 0, SkSamplingOptions(), &testMaskPaint);
    canvas->restore();
}
