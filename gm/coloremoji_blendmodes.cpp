/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkTextUtils.h"
#include "src/utils/SkUTF.h"
#include "tools/ToolUtils.h"

namespace skiagm {

static uint16_t gData[] = { 0xFFFF, 0xCCCF, 0xCCCF, 0xFFFF };

class ColorEmojiBlendModesGM : public GM {
public:
    const static int W = 64;
    const static int H = 64;
    ColorEmojiBlendModesGM() {}

protected:
    void onOnceBeforeDraw() override {
        const SkColor colors[] = {
            SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE,
            SK_ColorMAGENTA, SK_ColorCYAN, SK_ColorYELLOW
        };
        SkMatrix local;
        local.setRotate(180);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setShader(SkGradientShader::MakeSweep(0, 0, colors, nullptr, SK_ARRAY_COUNT(colors),
                                                    0, &local));

        sk_sp<SkTypeface> orig(ToolUtils::create_portable_typeface("serif", SkFontStyle::Bold()));
        if (nullptr == orig) {
            orig = SkTypeface::MakeDefault();
        }
        fColorType = ToolUtils::emoji_typeface();

        fBG.installPixels(SkImageInfo::Make(2, 2, kARGB_4444_SkColorType,
                                            kOpaque_SkAlphaType), gData, 4);
    }

    virtual SkString onShortName() override {
        return SkString("coloremoji_blendmodes");
    }

    virtual SkISize onISize() override {
        return SkISize::Make(400, 640);
    }

    virtual void onDraw(SkCanvas* canvas) override {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(20));

        const SkBlendMode gModes[] = {
            SkBlendMode::kClear,
            SkBlendMode::kSrc,
            SkBlendMode::kDst,
            SkBlendMode::kSrcOver,
            SkBlendMode::kDstOver,
            SkBlendMode::kSrcIn,
            SkBlendMode::kDstIn,
            SkBlendMode::kSrcOut,
            SkBlendMode::kDstOut,
            SkBlendMode::kSrcATop,
            SkBlendMode::kDstATop,

            SkBlendMode::kXor,
            SkBlendMode::kPlus,
            SkBlendMode::kModulate,
            SkBlendMode::kScreen,
            SkBlendMode::kOverlay,
            SkBlendMode::kDarken,
            SkBlendMode::kLighten,
            SkBlendMode::kColorDodge,
            SkBlendMode::kColorBurn,
            SkBlendMode::kHardLight,
            SkBlendMode::kSoftLight,
            SkBlendMode::kDifference,
            SkBlendMode::kExclusion,
            SkBlendMode::kMultiply,
            SkBlendMode::kHue,
            SkBlendMode::kSaturation,
            SkBlendMode::kColor,
            SkBlendMode::kLuminosity,
        };

        const SkScalar w = SkIntToScalar(W);
        const SkScalar h = SkIntToScalar(H);
        SkMatrix m;
        m.setScale(SkIntToScalar(6), SkIntToScalar(6));
        auto s = fBG.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, &m);

        SkFont labelFont(ToolUtils::create_portable_typeface());

        SkPaint textP;
        textP.setAntiAlias(true);
        SkFont textFont(fColorType, 70);

        const int W = 5;

        SkScalar x0 = 0;
        SkScalar y0 = 0;
        SkScalar x = x0, y = y0;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gModes); i++) {
            SkRect r;
            r.set(x, y, x+w, y+h);

            SkPaint p;
            p.setStyle(SkPaint::kFill_Style);
            p.setShader(s);
            canvas->drawRect(r, p);

            r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
            p.setStyle(SkPaint::kStroke_Style);
            p.setShader(nullptr);
            canvas->drawRect(r, p);

            {
                SkAutoCanvasRestore arc(canvas, true);
                canvas->clipRect(r);
                textP.setBlendMode(gModes[i]);
                const char* text    = ToolUtils::emoji_sample_text();
                SkUnichar unichar = SkUTF::NextUTF8(&text, text + strlen(text));
                SkASSERT(unichar >= 0);
                canvas->drawSimpleText(&unichar, 4, kUTF32_SkTextEncoding, x+ w/10.f, y + 7.f*h/8.f,
                                       textFont, textP);
            }
#if 1
            const char* label = SkBlendMode_Name(gModes[i]);
            SkTextUtils::DrawString(canvas, label, x + w/2, y - labelFont.getSize()/2, labelFont, SkPaint(),
                                    SkTextUtils::kCenter_Align);
#endif
            x += w + SkIntToScalar(10);
            if ((i % W) == W - 1) {
                x = x0;
                y += h + SkIntToScalar(30);
            }
        }
    }

private:
    SkBitmap            fBG;
    sk_sp<SkTypeface>   fColorType;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ColorEmojiBlendModesGM; )

}
