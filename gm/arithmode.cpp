/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "tools/ToolUtils.h"

#include <utility>

static sk_sp<SkImage> make_src(int w, int h) {
    sk_sp<SkSurface> surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(w, h)));
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    SkPoint pts[] = { {0, 0}, {SkIntToScalar(w), SkIntToScalar(h)} };
    SkColor colors[] = {
        SK_ColorTRANSPARENT, SK_ColorGREEN, SK_ColorCYAN,
        SK_ColorRED, SK_ColorMAGENTA, SK_ColorWHITE,
    };
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, std::size(colors),
                                                 SkTileMode::kClamp));
    canvas->drawPaint(paint);
    return surface->makeImageSnapshot();
}

static sk_sp<SkImage> make_dst(int w, int h) {
    sk_sp<SkSurface> surface(SkSurfaces::Raster(SkImageInfo::MakeN32Premul(w, h)));
    SkCanvas* canvas = surface->getCanvas();

    SkPaint paint;
    SkPoint pts[] = { {0, SkIntToScalar(h)}, {SkIntToScalar(w), 0} };
    SkColor colors[] = {
        SK_ColorBLUE, SK_ColorYELLOW, SK_ColorBLACK, SK_ColorGREEN,
        SK_ColorGRAY,
    };
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, nullptr, std::size(colors),
                                                 SkTileMode::kClamp));
    canvas->drawPaint(paint);
    return surface->makeImageSnapshot();
}

static void show_k_text(SkCanvas* canvas, SkScalar x, SkScalar y, const SkScalar k[]) {
    SkFont font(ToolUtils::create_portable_typeface(), 24);
    font.setEdging(SkFont::Edging::kAntiAlias);
    SkPaint paint;
    paint.setAntiAlias(true);
    for (int i = 0; i < 4; ++i) {
        SkString str;
        str.appendScalar(k[i]);
        SkScalar width = font.measureText(str.c_str(), str.size(), SkTextEncoding::kUTF8);
        canvas->drawString(str, x, y + font.getSize(), font, paint);
        x += width + SkIntToScalar(10);
    }
}

class ArithmodeGM : public skiagm::GM {
    SkString getName() const override { return SkString("arithmode"); }

    SkISize getISize() override { return {640, 572}; }

    void onDraw(SkCanvas* canvas) override {
        constexpr int WW = 100,
                      HH = 32;

        sk_sp<SkImage> src = make_src(WW, HH);
        sk_sp<SkImage> dst = make_dst(WW, HH);
        sk_sp<SkImageFilter> srcFilter = SkImageFilters::Image(src, {SkFilterMode::kLinear});
        sk_sp<SkImageFilter> dstFilter = SkImageFilters::Image(dst, {SkFilterMode::kLinear});

        constexpr SkScalar one = SK_Scalar1;
        constexpr SkScalar K[] = {
            0, 0, 0, 0,
            0, 0, 0, one,
            0, one, 0, 0,
            0, 0, one, 0,
            0, one, one, 0,
            0, one, -one, 0,
            0, one/2, one/2, 0,
            0, one/2, one/2, one/4,
            0, one/2, one/2, -one/4,
            one/4, one/2, one/2, 0,
            -one/4, one/2, one/2, 0,
        };

        const SkScalar* k = K;
        const SkScalar* stop = k + std::size(K);
        // Many of the Arithmetic filters have a 4th coefficient that's not zero, which means they
        // affect transparent black. 'rect' is used as a crop filter to make sure they don't
        // overwrite each other.
        const SkRect rect = SkRect::MakeWH(WW, HH);
        SkScalar gap = SkIntToScalar(WW + 20);
        while (k < stop) {
            {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->drawImage(src, 0, 0);
                canvas->translate(gap, 0);
                canvas->drawImage(dst, 0, 0);
                canvas->translate(gap, 0);
                SkPaint paint;
                paint.setImageFilter(SkImageFilters::Arithmetic(k[0], k[1], k[2], k[3], true,
                                                                dstFilter, srcFilter, rect));
                canvas->saveLayer(nullptr, &paint);
                canvas->restore();

                canvas->translate(gap, 0);
                show_k_text(canvas, 0, 0, k);
            }

            k += 4;
            canvas->translate(0, HH + 12);
        }

        // Draw two special cases to test enforcePMColor. In these cases, we
        // draw the dst bitmap twice, the first time it is halved and inverted,
        // leading to invalid premultiplied colors. If we enforcePMColor, these
        // invalid values should be clamped, and will not contribute to the
        // second draw.
        for (int i = 0; i < 2; i++) {
            const bool enforcePMColor = (i == 0);

            {
                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(gap, 0);
                canvas->drawImage(dst, 0, 0);
                canvas->translate(gap, 0);

                sk_sp<SkImageFilter> bg =
                        SkImageFilters::Arithmetic(0, 0, -one / 2, 1, enforcePMColor, dstFilter,
                                                   nullptr, nullptr);
                SkPaint p;
                p.setImageFilter(SkImageFilters::Arithmetic(0, one / 2, -one, 1, true,
                                                            std::move(bg), dstFilter, rect));
                canvas->saveLayer(nullptr, &p);
                canvas->restore();
                canvas->translate(gap, 0);

                // Label
                SkFont   font(ToolUtils::create_portable_typeface(), 24);
                SkString str(enforcePMColor ? "enforcePM" : "no enforcePM");
                canvas->drawString(str, 0, font.getSize(), font, SkPaint());
            }
            canvas->translate(0, HH + 12);
        }
    }

private:
    using INHERITED = GM;
};
DEF_GM( return new ArithmodeGM; )

///////////////////////////////////////////////////////////////////////////////

#include "include/effects/SkBlenders.h"

class ArithmodeBlenderGM : public skiagm::GM {
    float                  fK1, fK2, fK3, fK4;
    sk_sp<SkImage>         fSrc, fDst, fChecker;
    sk_sp<SkShader>        fSrcShader, fDstShader;
    sk_sp<SkRuntimeEffect> fRuntimeEffect;

    SkString getName() const override { return SkString("arithmode_blender"); }

    static constexpr int W = 200;
    static constexpr int H = 200;

    SkISize getISize() override { return {(W + 30) * 2, (H + 30) * 4}; }

    void onOnceBeforeDraw() override {
        // Prepare a runtime effect for this blend.
        static constexpr char kShader[] = R"(
            uniform shader srcImage;
            uniform shader dstImage;
            uniform blender arithBlend;
            half4 main(float2 xy) {
                return arithBlend.eval(srcImage.eval(xy), dstImage.eval(xy));
            }
        )";
        auto [effect, error] = SkRuntimeEffect::MakeForShader(SkString(kShader));
        SkASSERT(effect);
        fRuntimeEffect = effect;

        // Start with interesting K-values, in case we're drawn without calling onAnimate().
        fK1 = -0.25f;
        fK2 =  0.25f;
        fK3 =  0.25f;
        fK4 =  0;

        fSrc = make_src(W, H);
        fDst = make_dst(W, H);
        fSrcShader = fSrc->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions());
        fDstShader = fDst->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions());

        fChecker = ToolUtils::create_checkerboard_image(W, H, 0xFFBBBBBB, 0xFFEEEEEE, 8);
    }

    bool onAnimate(double nanos) override {
        double theta = nanos * 1e-6 * 0.001;
        fK1 = sin(theta + 0) * 0.25;
        fK2 = cos(theta + 1) * 0.25;
        fK3 = sin(theta + 2) * 0.25;
        fK4 = 0.5;
        return true;
    }

    void onDraw(SkCanvas* canvas) override {
        const SkRect rect = SkRect::MakeWH(W, H);

        canvas->drawImage(fSrc, 10, 10);
        canvas->drawImage(fDst, 10, 10 + H + 10);

        SkSamplingOptions sampling;
        sk_sp<SkBlender> blender = SkBlenders::Arithmetic(fK1, fK2, fK3, fK4,
                                                          /*enforcePremul=*/true);
        canvas->translate(10 + W + 10, 10);

        // All three images drawn below should appear identical.
        // Draw via blend step
        SkPaint blenderPaint;
        canvas->drawImage(fChecker, 0, 0);
        canvas->saveLayer(&rect, nullptr);
        canvas->drawImage(fDst, 0, 0);
        blenderPaint.setBlender(blender);
        canvas->drawImage(fSrc, 0, 0, sampling, &blenderPaint);
        canvas->restore();

        canvas->translate(0, 10 + H);

        // Draw via SkImageFilters::Blend (should appear the same as above)
        SkPaint imageFilterPaint;
        canvas->drawImage(fChecker, 0, 0);
        imageFilterPaint.setImageFilter(
                SkImageFilters::Blend(blender,
                                      /*background=*/nullptr,
                                      /*foreground=*/SkImageFilters::Image(fSrc, sampling)));
        canvas->drawImage(fDst, 0, 0, sampling, &imageFilterPaint);

        canvas->translate(0, 10 + H);

        // Draw via SkShaders::Blend (should still appear the same as above)
        SkPaint shaderBlendPaint;
        canvas->drawImage(fChecker, 0, 0);
        shaderBlendPaint.setShader(SkShaders::Blend(blender, fDstShader, fSrcShader));
        canvas->drawRect(rect, shaderBlendPaint);

        canvas->translate(0, 10 + H);

        // Draw via runtime effect (should still appear the same as above)
        SkPaint runtimePaint;
        canvas->drawImage(fChecker, 0, 0);
        SkRuntimeEffect::ChildPtr children[] = {fSrcShader, fDstShader, blender};
        runtimePaint.setShader(fRuntimeEffect->makeShader(/*uniforms=*/{}, children));
        canvas->drawRect(rect, runtimePaint);
    }

private:
    using INHERITED = GM;
};
DEF_GM( return new ArithmodeBlenderGM; )
