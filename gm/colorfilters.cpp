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
#include "include/effects/SkColorMatrixFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkNx.h"
#include "src/core/SkColorFilterPriv.h"
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
    return SkGradientShader::MakeLinear(pts, colors, nullptr, SK_ARRAY_COUNT(colors),
                                        SkTileMode::kClamp);
}

typedef void (*InstallPaint)(SkPaint*, uint32_t, uint32_t);

static void install_nothing(SkPaint* paint, uint32_t, uint32_t) {
    paint->setColorFilter(nullptr);
}

static void install_lighting(SkPaint* paint, uint32_t mul, uint32_t add) {
    paint->setColorFilter(SkColorMatrixFilter::MakeLightingFilter(mul, add));
}

class ColorFiltersGM : public skiagm::GM {
    SkString onShortName() override { return SkString("lightingcolorfilter"); }

    SkISize onISize() override { return {620, 430}; }

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
        for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
            rec[i].fProc(&paint, rec[i].fData0, rec[i].fData1);
            canvas->drawRect(r, paint);
            canvas->translate(0, r.height() + 10);
        }
    }
};

DEF_GM(return new ColorFiltersGM;)

namespace  {

// Based on work by Sam Hocevar, Emil Persson, and Ian Taylor [1][2][3].  High-level ideas:
//
//   - minimize the number of branches by sorting and computing the hue phase in parallel (vec4s)
//
//   - trade the third sorting branch for a potentially faster std::min and leaving 2nd/3rd
//     channels unsorted (based on the observation that swapping both the channels and the bias sign
//     has no effect under abs)
//
//   - use epsilon offsets for denominators, to avoid explicit zero-checks
//
// An additional trick we employ is deferring premul->unpremul conversion until the very end: the
// alpha factor gets naturally simplified for H and S, and only L requires a full-blown division
// (so we trade three divs for one).
//
// [1] http://lolengine.net/blog/2013/01/13/fast-rgb-to-hsv
// [2] http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
// [3] http://www.chilliant.com/rgb2hsv.html

SkRGBA4f<kUnpremul_SkAlphaType> RGBA2HSLA(const SkRGBA4f<kPremul_SkAlphaType>& c) {
    const auto p = (c.fG < c.fB) ? SkVector4{ c.fB, c.fG, -1,  2/3.f }
                                 : SkVector4{ c.fG, c.fB,  0, -1/3.f },
               q = (c.fR < p[0]) ? SkVector4{ p[0], c.fR, p[1], p[3] }
                                 : SkVector4{ c.fR, p[0], p[1], p[2] };

    // q[0]      -> max channel value
    // q[1],q[2] -> 2nd/3rd channel values (unsorted)
    // q[3]      -> bias value dependent on max channel selection

    const auto eps = 0.0001f, // matching SkSL/ColorMatrix half4 epsilon
               pmV = q[0],
               pmC = pmV - std::min(q[1], q[2]),
               pmL = pmV - pmC * 0.5f,
                 H = std::abs(q[3] + (q[1] - q[2]) / (pmC * 6 + eps)),
                 S = pmC / (c.fA + eps - abs(pmL * 2 - c.fA)),
                 L = pmL / (c.fA + eps);

    return { H, S, L, c.fA };
}

SkRGBA4f<kPremul_SkAlphaType> HSLA2RGBA(const SkRGBA4f<kUnpremul_SkAlphaType>& c) {
    static constexpr float r_bias = 0,
                           g_bias = 2.f / 3,
                           b_bias = 1.f / 3;
    const auto H = c[0],
               S = c[1],
               L = c[2],
               C = (1 - abs(2 * L - 1)) * S;

    const auto p = H + Sk4f(r_bias, g_bias, b_bias, 0),
               q = ((p - p.floor()) * 6 - 3).abs() - 1,
             RGB = C * (Sk4f::Min(Sk4f::Max(q, 0), 1) - 0.5f) + L,
           pmRGB = Sk4f::Min(Sk4f::Max(RGB, 0), 1) * c.fA;

    return { pmRGB[0], pmRGB[1], pmRGB[2], c.fA };
}

void HSLAMatrix_CPU(float colors[4], const void* ctx) {
    auto& inout = *reinterpret_cast<SkRGBA4f<kPremul_SkAlphaType>*>(colors);

    auto c = RGBA2HSLA(inout);

    {
        const auto* m = static_cast<const float*>(ctx);
        const auto c0 = c[0]*m[ 0] + c[1]*m[ 1] + c[2]*m[ 2] + c[3]*m[ 3] + m[ 4],
                   c1 = c[0]*m[ 5] + c[1]*m[ 6] + c[2]*m[ 7] + c[3]*m[ 8] + m[ 9],
                   c2 = c[0]*m[10] + c[1]*m[11] + c[2]*m[12] + c[3]*m[13] + m[14],
                   c3 = c[0]*m[15] + c[1]*m[16] + c[2]*m[17] + c[3]*m[18] + m[19];
        c[0] = c0;
        c[1] = c1;
        c[2] = c2;
        c[3] = c3;
    }

    inout = HSLA2RGBA(c);
}

const char HSLAMatrix_GPU_SRC[] = R"(
    // WTB matrix/vector inputs.
    layout(ctype=float) in uniform half m0 , m1 , m2 , m3 , m4 ,
                                        m5 , m6 , m7 , m8 , m9 ,
                                        m10, m11, m12, m13, m14,
                                        m15, m16, m17, m18, m19;


    void main(inout half4 c) {
        // RGBA->HSLA
        {
            half4 p = (c.g < c.b) ? half4(c.bg, -1,  2/3.0)
                                  : half4(c.gb,  0, -1/3.0);
            half4 q = (c.r < p.x) ? half4(p.x, c.r, p.yw)
                                  : half4(c.r, p.x, p.yz);

            half eps = 0.0001;
            half pmV = q.x;
            half pmC = pmV - min(q.y, q.z);
            half pmL = pmV - pmC * 0.5;
            half   H = abs(q.w + (q.y - q.z) / (pmC * 6 + eps));
            half   S = pmC / (c.a + eps - abs(pmL * 2 - c.a));
            half   L = pmL / (c.a + eps);

            c.rgb = half3(H, S, L);
        }

        half4x4 m = half4x4(m0, m5, m10, m15,
                            m1, m6, m11, m16,
                            m2, m7, m12, m17,
                            m3, m8, m13, m18);
        c = m * c + half4  (m4, m9, m14, m19);

        // HSLA -> RGBA
        {
            half3   hsl = c.rgb;
            half3 kBias = half3(0, 2/3.0, 1/3.0);
            half3   rgb = saturate(abs(fract(hsl.xxx + kBias) * 6 - 3) - 1);
            half      C = (1 - abs(2 * hsl.z - 1)) * hsl.y;

            c.rgb = (rgb - 0.5) * C + hsl.z;
        }

        c = saturate(c);
        c.rgb *= c.a;
    }
)";

} // namespace

class HSLColorFilterGM : public skiagm::GM {
public:
    HSLColorFilterGM(bool is_runtime) : fIsRuntime(is_runtime) {}

protected:
    SkString onShortName() override {
        return SkStringPrintf("hslcolorfilter%s", fIsRuntime ? "_runtime" : "");
    }

    SkISize onISize() override { return { 840, 1100 }; }

    void onOnceBeforeDraw() override {
        sk_sp<SkImage> mandrill = GetResourceAsImage("images/mandrill_256.png");
        const auto lm = SkMatrix::MakeRectToRect(SkRect::MakeWH(mandrill->width(),
                                                                mandrill->height()),
                                                 SkRect::MakeWH(kWheelSize, kWheelSize),
                                                 SkMatrix::kFill_ScaleToFit);
        fShaders.push_back(mandrill->makeShader(&lm));

        static constexpr SkColor gGrads[][4] = {
            { 0xffff0000, 0xff00ff00, 0xff0000ff, 0xffff0000 },
            { 0xdfc08040, 0xdf8040c0, 0xdf40c080, 0xdfc08040 },
        };

        for (const auto& cols : gGrads) {
            fShaders.push_back(SkGradientShader::MakeSweep(kWheelSize / 2, kWheelSize / 2,
                                                           cols, nullptr, SK_ARRAY_COUNT(cols),
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
                        paint.setColorFilter(this->make_filter(h, s, l));
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
    static constexpr SkScalar kWheelSize  = 100;
    static constexpr size_t   kSteps = 7;

    sk_sp<SkColorFilter> make_filter(float h, float s, float l) {
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

        if (fIsRuntime) {
            static SkRuntimeColorFilterFactory fact(SkString(HSLAMatrix_GPU_SRC), HSLAMatrix_CPU);
            return fact.make(SkData::MakeWithCopy(cm, sizeof(cm)));
        }

        return SkColorFilters::HSLAMatrix(cm);
    }

    const bool fIsRuntime;
    std::vector<sk_sp<SkShader>> fShaders;
};

DEF_GM(return new HSLColorFilterGM(false);)
DEF_GM(return new HSLColorFilterGM(true);)
