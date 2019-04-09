/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "AnimTimer.h"
#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkLumaColorFilter.h"
#include "SkTableColorFilter.h"

// A tint filter maps colors to a given range (gradient), based on the input luminance:
//
//   c' = lerp(lo, hi, luma(c))
//
// TODO: move to public headers/API?
//
static sk_sp<SkColorFilter> MakeTintColorFilter(SkColor lo, SkColor hi) {
    const auto r_lo = SkColorGetR(lo),
    g_lo = SkColorGetG(lo),
    b_lo = SkColorGetB(lo),
    a_lo = SkColorGetA(lo),
    r_hi = SkColorGetR(hi),
    g_hi = SkColorGetG(hi),
    b_hi = SkColorGetB(hi),
    a_hi = SkColorGetA(hi);

    // We map component-wise:
    //
    //   r' = lo.r + (hi.r - lo.r) * luma
    //   g' = lo.g + (hi.g - lo.g) * luma
    //   b' = lo.b + (hi.b - lo.b) * luma
    //   a' = lo.a + (hi.a - lo.a) * luma
    //
    // The input luminance is stored in the alpha channel
    // (and RGB are cleared -- see SkLumaColorFilter). Thus:
    const SkScalar tint_matrix[] = {
        0, 0, 0, (r_hi - r_lo) / 255.0f, SkIntToScalar(r_lo),
        0, 0, 0, (g_hi - g_lo) / 255.0f, SkIntToScalar(g_lo),
        0, 0, 0, (b_hi - b_lo) / 255.0f, SkIntToScalar(b_lo),
        0, 0, 0, (a_hi - a_lo) / 255.0f, SkIntToScalar(a_lo),
    };

    return SkColorFilters::MatrixRowMajor255(tint_matrix)
    ->makeComposed(SkLumaColorFilter::Make());
}

namespace {

class MixerCFGM final : public skiagm::GM {
public:
    MixerCFGM(const SkSize& tileSize, size_t tileCount)
        : fTileSize(tileSize)
        , fTileCount(tileCount) {}

protected:
    SkString onShortName() override {
        return SkString("mixerCF");
    }

    SkISize onISize() override {
        return SkISize::Make(fTileSize.width()  * 1.2f * fTileCount,
                             fTileSize.height() * 1.2f * 3);         // 3 rows
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;

        const SkColor gradient_colors[] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorRED };
        paint.setShader(SkGradientShader::MakeSweep(fTileSize.width()  / 2,
                                                    fTileSize.height() / 2,
                                                    gradient_colors, nullptr,
                                                    SK_ARRAY_COUNT(gradient_colors)));

        auto cf0 = MakeTintColorFilter(0xff300000, 0xffa00000);  // red tint
        auto cf1 = MakeTintColorFilter(0xff003000, 0xff00a000);  // green tint

        this->mixRow(canvas, paint, nullptr,     cf1);
        this->mixRow(canvas, paint,     cf0, nullptr);
        this->mixRow(canvas, paint,     cf0,     cf1);
    }

private:
    const SkSize fTileSize;
    const size_t fTileCount;

    void mixRow(SkCanvas* canvas, SkPaint& paint,
                sk_sp<SkColorFilter> cf0, sk_sp<SkColorFilter> cf1) {
        canvas->translate(0, fTileSize.height() * 0.1f);
        {
            SkAutoCanvasRestore arc(canvas, true);
            for (size_t i = 0; i < fTileCount; ++i) {
                paint.setColorFilter(
                    SkColorFilters::Lerp(static_cast<float>(i) / (fTileCount - 1), cf0, cf1));
                canvas->translate(fTileSize.width() * 0.1f, 0);
                canvas->drawRect(SkRect::MakeWH(fTileSize.width(), fTileSize.height()), paint);
                canvas->translate(fTileSize.width() * 1.1f, 0);
            }
        }
        canvas->translate(0, fTileSize.height() * 1.1f);
    }

    using INHERITED = skiagm::GM;
};

} // namespace
DEF_GM( return new MixerCFGM(SkSize::Make(200, 250), 5); )

#include "Resources.h"
#include "SkMixer.h"

static sk_sp<SkShader> make_resource_shader(const char path[], int size) {
    auto img = GetResourceAsImage(path);
    if (!img) {
        return nullptr;
    }
    SkRect src = SkRect::MakeIWH(img->width(), img->height());
    SkRect dst = SkRect::MakeIWH(size, size);
    SkMatrix m;
    m.setRectToRect(src, dst, SkMatrix::kFill_ScaleToFit);
    return img->makeShader(&m);
}

static sk_sp<SkShader> make_grad(int size, float t) {
    SkASSERT(t >= 0 && t <= 1);
    unsigned r = SkScalarRoundToInt(t * 255);
    SkColor c = SkColorSetARGB(r, r, 0, 0);

    SkColor colors[] = { 0, c, SK_ColorRED };
    SkPoint pts[] = {{0, 0}, {size*1.0f, size*1.0f}};
    SkScalar pos[] = {0, 1 - t, 1.0f};
    return SkGradientShader::MakeLinear(pts, colors, pos, SK_ARRAY_COUNT(colors),
                                        SkTileMode::kClamp);
}

class ShaderMixerGM final : public skiagm::GM {
    enum { SIZE = 256 };
    float fPos = 0.5f;
    sk_sp<SkShader> fS0, fS1;

public:
    ShaderMixerGM() {}

protected:
    SkString onShortName() override {
        return SkString("mixershader_shadermixer");
    }

    void onOnceBeforeDraw() override {
        fS0 = make_resource_shader("images/mandrill_256.png", SIZE);
        fS1 = make_resource_shader("images/baby_tux.png", SIZE);
    }

    SkISize onISize() override { return {542, 542}; }

    void onDraw(SkCanvas* canvas) override {
        SkRect r = SkRect::MakeIWH(SIZE, SIZE);

        SkPaint paint;

        canvas->translate(10, 10);

        canvas->save();
        paint.setShader(fS0);
        canvas->drawRect(r, paint);
        canvas->translate(SIZE + 10.0f, 0);
        paint.setShader(fS1);
        canvas->drawRect(r, paint);
        canvas->restore();

        auto sh2 = make_grad(SIZE, fPos);

        canvas->translate(0, SIZE + 10.0f);
        paint.setShader(sh2);
        canvas->drawRect(r, paint);

        auto sh = SkShaders::Lerp(sh2, fS0, fS1);
        canvas->translate(SIZE + 10.0f, 0);
        paint.setShader(sh);
        canvas->drawRect(r, paint);
    }

    bool onAnimate(const AnimTimer& timer) override {
        fPos = (sin(timer.secs()) + 1) * 0.5f;
        return true;
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM( return new ShaderMixerGM; )

static void draw_rect(SkCanvas* c, const SkRect& r, const SkPaint& p, SkScalar x, SkScalar y) {
    c->save();
    c->translate(x, y);
    c->drawRect(r, p);
    c->restore();
}

DEF_SIMPLE_GM(mixercolorfilter, canvas, 768, 512) {
    auto cf0 = MakeTintColorFilter(0xff300000, 0xffa00000);  // red tint
    auto cf1 = MakeTintColorFilter(0xff003000, 0xff00a000);  // green tint

    SkRect r = { 0, 0, 256, 256 };

    SkPaint p;
    p.setShader(make_resource_shader("images/mandrill_256.png", 256));

    draw_rect(canvas, r, p,   0,   0);
    p.setColorFilter(cf0);
    draw_rect(canvas, r, p, 256,   0);
    p.setColorFilter(cf1);
    draw_rect(canvas, r, p, 512,   0);

    auto mx = SkMixer::MakeLerp(0.5f);

    p.setColorFilter(SkColorFilters::Mixer(mx, cf0, cf1));
    draw_rect(canvas, r, p,   0, 256);
    p.setColorFilter(SkColorFilters::Mixer(mx, cf0, nullptr));
    draw_rect(canvas, r, p, 256, 256);
    p.setColorFilter(SkColorFilters::Mixer(mx, nullptr, cf1));
    draw_rect(canvas, r, p, 512, 256);
}
