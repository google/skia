/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "tools/Resources.h"

#include <vector>
#include <tuple>

static sk_sp<SkShader> make_shader(const SkRect& bounds) {
    const SkPoint pts[] = {
        { bounds.left(), bounds.top() },
        { bounds.right(), bounds.bottom() },
    };
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorBLACK,
        SK_ColorCYAN, SK_ColorMAGENTA, SK_ColorYELLOW,
    };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, std::size(colors),
                                        SkTileMode::kClamp);
}

typedef void (*InstallPaint)(SkPaint*, uint32_t, uint32_t);

static void install_nothing(SkPaint* paint, uint32_t, uint32_t) {
    paint->setColorFilter(nullptr);
}

static void install_lighting(SkPaint* paint, uint32_t mul, uint32_t add) {
    paint->setColorFilter(SkColorFilters::Lighting(mul, add));
}

class ColorFiltersGM : public skiagm::GM {
    SkString getName() const override { return SkString("lightingcolorfilter"); }

    SkISize getISize() override { return {620, 430}; }

    void onDraw(SkCanvas* canvas) override {
        SkRect r = {0, 0, 600, 50};

        SkPaint paint;
        paint.setShader(make_shader(r));

        const struct {
            InstallPaint    fProc;
            uint32_t        fData0, fData1;
        } rec[] = {
            { install_nothing, 0, 0 },
            { install_lighting, 0xFF0000, 0 },
            { install_lighting, 0x00FF00, 0 },
            { install_lighting, 0x0000FF, 0 },
            { install_lighting, 0x000000, 0xFF0000 },
            { install_lighting, 0x000000, 0x00FF00 },
            { install_lighting, 0x000000, 0x0000FF },
        };

        canvas->translate(10, 10);
        for (size_t i = 0; i < std::size(rec); ++i) {
            rec[i].fProc(&paint, rec[i].fData0, rec[i].fData1);
            canvas->drawRect(r, paint);
            canvas->translate(0, r.height() + 10);
        }
    }
};

DEF_GM(return new ColorFiltersGM;)

class HSLColorFilterGM : public skiagm::GM {
protected:
    SkString getName() const override { return SkString("hslcolorfilter"); }

    SkISize getISize() override { return {840, 1100}; }

    void onOnceBeforeDraw() override {
        sk_sp<SkImage> mandrill = GetResourceAsImage("images/mandrill_256.png");
        const auto lm = SkMatrix::RectToRect(SkRect::MakeWH(mandrill->width(), mandrill->height()),
                                             SkRect::MakeWH(kWheelSize, kWheelSize));
        fShaders.push_back(mandrill->makeShader(SkSamplingOptions(), &lm));

        static constexpr SkColor gGrads[][4] = {
            { 0xffff0000, 0xff00ff00, 0xff0000ff, 0xffff0000 },
            { 0xdfc08040, 0xdf8040c0, 0xdf40c080, 0xdfc08040 },
        };

        for (const auto& cols : gGrads) {
            fShaders.push_back(SkGradientShader::MakeSweep(kWheelSize / 2, kWheelSize / 2,
                                                           cols, nullptr, std::size(cols),
                                                           SkTileMode::kRepeat, -90, 270, 0,
                                                           nullptr));
        }
    }

    void onDraw(SkCanvas* canvas) override {
        using std::make_tuple;

        static constexpr struct {
            std::tuple<float, float> h, s, l;
        } gTests[] = {
            { make_tuple(-0.5f, 0.5f), make_tuple( 0.0f, 0.0f), make_tuple( 0.0f, 0.0f) },
            { make_tuple( 0.0f, 0.0f), make_tuple(-1.0f, 1.0f), make_tuple( 0.0f, 0.0f) },
            { make_tuple( 0.0f, 0.0f), make_tuple( 0.0f, 0.0f), make_tuple(-1.0f, 1.0f) },
        };

        const auto rect = SkRect::MakeWH(kWheelSize, kWheelSize);

        canvas->drawColor(0xffcccccc);
        SkPaint paint;

        for (const auto& shader : fShaders) {
            paint.setShader(shader);

            for (const auto& tst: gTests) {
                canvas->translate(0, kWheelSize * 0.1f);

                const auto dh = (std::get<1>(tst.h) - std::get<0>(tst.h)) / (kSteps - 1),
                           ds = (std::get<1>(tst.s) - std::get<0>(tst.s)) / (kSteps - 1),
                           dl = (std::get<1>(tst.l) - std::get<0>(tst.l)) / (kSteps - 1);
                auto h = std::get<0>(tst.h),
                     s = std::get<0>(tst.s),
                     l = std::get<0>(tst.l);
                {
                    SkAutoCanvasRestore acr(canvas, true);
                    for (size_t i = 0; i < kSteps; ++i) {
                        paint.setColorFilter(make_filter(h, s, l));
                        canvas->translate(kWheelSize * 0.1f, 0);
                        canvas->drawRect(rect, paint);
                        canvas->translate(kWheelSize * 1.1f, 0);
                        h += dh;
                        s += ds;
                        l += dl;
                    }
                }
                canvas->translate(0, kWheelSize * 1.1f);
            }
            canvas->translate(0, kWheelSize * 0.1f);
        }
    }

private:
    inline static constexpr SkScalar kWheelSize  = 100;
    inline static constexpr size_t   kSteps = 7;

    static sk_sp<SkColorFilter> make_filter(float h, float s, float l) {
        // These are roughly AE semantics.
        const auto h_bias  = h,
                   h_scale = 1.0f,
                   s_bias  = std::max(s, 0.0f),
                   s_scale = 1 - std::abs(s),
                   l_bias  = std::max(l, 0.0f),
                   l_scale = 1 - std::abs(l);

        const float cm[20] = {
            h_scale,       0,       0, 0, h_bias,
                  0, s_scale,       0, 0, s_bias,
                  0,       0, l_scale, 0, l_bias,
                  0,       0,       0, 1,      0,
        };

        return SkColorFilters::HSLAMatrix(cm);
    }

    std::vector<sk_sp<SkShader>> fShaders;
};

DEF_GM(return new HSLColorFilterGM;)
