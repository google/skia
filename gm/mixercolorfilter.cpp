/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkCanvas.h"
#include "SkColorFilter.h"
#include "SkGradientShader.h"
#include "SkLumaColorFilter.h"
#include "SkTableColorFilter.h"

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
                    SkColorFilter::MakeLerp(cf0, cf1, static_cast<float>(i) / (fTileCount - 1)));
                canvas->translate(fTileSize.width() * 0.1f, 0);
                canvas->drawRect(SkRect::MakeWH(fTileSize.width(), fTileSize.height()), paint);
                canvas->translate(fTileSize.width() * 1.1f, 0);
            }
        }
        canvas->translate(0, fTileSize.height() * 1.1f);
    }

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

        return SkColorFilter::MakeMatrixFilterRowMajor255(tint_matrix)
                 ->makeComposed(SkLumaColorFilter::Make());
    }

    using INHERITED = skiagm::GM;
};

} // namespace

DEF_GM( return new MixerCFGM(SkSize::Make(200, 250), 5); )
