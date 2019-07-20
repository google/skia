/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkDrawLooper.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/Sk2DPathEffect.h"
#include "include/effects/SkColorMatrixFilter.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkLayerDrawLooper.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkBlurMask.h"
#include "tools/ToolUtils.h"

#include <string.h>

namespace skiagm {

constexpr int kWidth = 1250;
constexpr int kHeight = 700;

// Unlike the variant in ToolUtils, this version positions the glyphs on a diagonal
static void add_to_text_blob(SkTextBlobBuilder* builder, const char* text, const SkFont& font,
                             SkScalar x, SkScalar y) {
    SkTDArray<uint16_t> glyphs;

    size_t len = strlen(text);
    glyphs.append(font.countText(text, len, SkTextEncoding::kUTF8));
    font.textToGlyphs(text, len, SkTextEncoding::kUTF8, glyphs.begin(), glyphs.count());

    const SkScalar advanceX = font.getSize() * 0.85f;
    const SkScalar advanceY = font.getSize() * 1.5f;

    SkTDArray<SkScalar> pos;
    for (unsigned i = 0; i < len; ++i) {
        *pos.append() = x + i * advanceX;
        *pos.append() = y + i * (advanceY / len);
    }
    const SkTextBlobBuilder::RunBuffer& run = builder->allocRunPos(font, glyphs.count());
    memcpy(run.glyphs, glyphs.begin(), glyphs.count() * sizeof(uint16_t));
    memcpy(run.pos, pos.begin(), len * sizeof(SkScalar) * 2);
}

typedef void (*LooperProc)(SkPaint*);

struct LooperSettings {
    SkBlendMode      fMode;
    SkColor          fColor;
    SkPaint::Style   fStyle;
    SkScalar         fWidth;
    SkScalar         fOffset;
    SkScalar         fSkewX;
    bool             fEffect;
};

static void mask_filter(SkPaint* paint) {
    paint->setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                                SkBlurMask::ConvertRadiusToSigma(3.f)));
}

static sk_sp<SkPathEffect> make_tile_effect() {
    SkMatrix m;
    m.setScale(1.f, 1.f);

    SkPath path;
    path.addCircle(0, 0, SkIntToScalar(5));

    return SkPath2DPathEffect::Make(m, path);
}

static void path_effect(SkPaint* paint) {
    paint->setPathEffect(make_tile_effect());
}

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

static void color_filter(SkPaint* paint) {
    SkRect r;
    r.setWH(SkIntToScalar(kWidth), 50);
    paint->setShader(make_shader(r));
    paint->setColorFilter(SkColorMatrixFilter::MakeLightingFilter(0xF0F0F0, 0));
}

static void kitchen_sink(SkPaint* paint) {
    color_filter(paint);
    path_effect(paint);
    mask_filter(paint);

}

static sk_sp<SkDrawLooper> setupLooper(SkLayerDrawLooper::BitFlags bits,
                                       LooperProc proc,
                                       const LooperSettings settings[],
                                       size_t size) {
    SkLayerDrawLooper::Builder looperBuilder;

    SkLayerDrawLooper::LayerInfo info;
    info.fPaintBits = bits;

    info.fColorMode = SkBlendMode::kSrc;

    for (size_t i = 0; i < size; i++) {
        info.fOffset.set(settings[i].fOffset, settings[i].fOffset);
        SkPaint* paint = looperBuilder.addLayer(info);
        paint->setBlendMode(settings[i].fMode);
        paint->setColor(settings[i].fColor);
        paint->setStyle(settings[i].fStyle);
        paint->setStrokeWidth(settings[i].fWidth);
        if (settings[i].fEffect) {
            (*proc)(paint);
        }
    }
    return looperBuilder.detach();
}

class TextBlobLooperGM : public GM {
public:
    TextBlobLooperGM() {}

protected:
    void onOnceBeforeDraw() override {
        SkTextBlobBuilder builder;

        // LCD
        SkFont font;
        font.setSize(32);
        const char* text = "The quick brown fox jumps over the lazy dog";
        font.setSubpixel(true);
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setTypeface(ToolUtils::create_portable_typeface());
        add_to_text_blob(&builder, text, font, 0, 0);
        fBlob = builder.make();

        // create a looper which sandwhiches an effect in two normal draws
        LooperSettings looperSandwhich[] = {
           { SkBlendMode::kSrc, SK_ColorMAGENTA, SkPaint::kFill_Style, 0, 0, 0, false },
           { SkBlendMode::kSrcOver, 0x88000000, SkPaint::kFill_Style, 0, 10.f, 0, true },
           { SkBlendMode::kSrcOver, 0x50FF00FF, SkPaint::kFill_Style, 0, 20.f, 0, false },
        };

        LooperSettings compound[] = {
            { SkBlendMode::kSrc, SK_ColorWHITE, SkPaint::kStroke_Style, 1.f * 3/4, 0, 0, false },
            { SkBlendMode::kSrc, SK_ColorRED, SkPaint::kStroke_Style, 4.f, 0, 0, false },
            { SkBlendMode::kSrc, SK_ColorBLUE, SkPaint::kFill_Style, 0, 0, 0, false },
            { SkBlendMode::kSrcOver, 0x88000000, SkPaint::kFill_Style, 0, 10.f, 0, true }
        };

        LooperSettings xfermode[] = {
            { SkBlendMode::kDifference, SK_ColorWHITE, SkPaint::kFill_Style, 0, 0, 0, false },
            { SkBlendMode::kSrcOver, 0xFF000000, SkPaint::kFill_Style, 0, 1.f, 0, true },
            { SkBlendMode::kSrcOver, 0x50FF00FF, SkPaint::kFill_Style, 0, 2.f, 0, false },
        };

        // NOTE, this should be ignored by textblobs
        LooperSettings skew[] = {
            { SkBlendMode::kSrc, SK_ColorRED, SkPaint::kFill_Style, 0, 0, -1.f, false },
            { SkBlendMode::kSrc, SK_ColorGREEN, SkPaint::kFill_Style, 0, 10.f, -1.f, false },
            { SkBlendMode::kSrc, SK_ColorBLUE, SkPaint::kFill_Style, 0, 20.f, -1.f, false },
        };

        LooperSettings kitchenSink[] = {
            { SkBlendMode::kSrc, SK_ColorWHITE, SkPaint::kStroke_Style, 1.f * 3/4, 0, 0, false },
            { SkBlendMode::kSrc, SK_ColorBLACK, SkPaint::kFill_Style, 0, 0, 0, false },
            { SkBlendMode::kDifference, SK_ColorWHITE, SkPaint::kFill_Style, 1.f, 10.f, 0, false },
            { SkBlendMode::kSrc, SK_ColorWHITE, SkPaint::kFill_Style, 0, 10.f, 0, true },
            { SkBlendMode::kSrcOver, 0x50FF00FF, SkPaint::kFill_Style, 0, 20.f, 0, false },
        };

        fLoopers.push_back(setupLooper(SkLayerDrawLooper::kMaskFilter_Bit |
                                       SkLayerDrawLooper::kXfermode_Bit |
                                       SkLayerDrawLooper::kStyle_Bit, &mask_filter,
                                       compound, SK_ARRAY_COUNT(compound)));
        fLoopers.push_back(setupLooper(SkLayerDrawLooper::kPathEffect_Bit |
                                       SkLayerDrawLooper::kXfermode_Bit, &path_effect,
                                       looperSandwhich, SK_ARRAY_COUNT(looperSandwhich)));
        fLoopers.push_back(setupLooper(SkLayerDrawLooper::kShader_Bit |
                                       SkLayerDrawLooper::kColorFilter_Bit |
                                       SkLayerDrawLooper::kXfermode_Bit, &color_filter,
                                       looperSandwhich, SK_ARRAY_COUNT(looperSandwhich)));
        fLoopers.push_back(setupLooper(SkLayerDrawLooper::kShader_Bit |
                                       SkLayerDrawLooper::kColorFilter_Bit |
                                       SkLayerDrawLooper::kXfermode_Bit, &color_filter,
                                       xfermode, SK_ARRAY_COUNT(xfermode)));
        fLoopers.push_back(setupLooper(0, nullptr, skew, SK_ARRAY_COUNT(skew)));
        fLoopers.push_back(setupLooper(SkLayerDrawLooper::kMaskFilter_Bit |
                                       SkLayerDrawLooper::kShader_Bit |
                                       SkLayerDrawLooper::kColorFilter_Bit |
                                       SkLayerDrawLooper::kPathEffect_Bit |
                                       SkLayerDrawLooper::kStyle_Bit |
                                       SkLayerDrawLooper::kXfermode_Bit, &kitchen_sink,
                                       kitchenSink, SK_ARRAY_COUNT(kitchenSink)));

        // Test we respect overrides
        fLoopers.push_back(setupLooper(0, &kitchen_sink,
                                       kitchenSink, SK_ARRAY_COUNT(kitchenSink)));
    }

    SkString onShortName() override {
        return SkString("textbloblooper");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(SK_ColorGRAY);

        SkPaint paint;
        canvas->translate(10, 40);

        SkRect bounds = fBlob->bounds();

        int y = 0;
        for (int looper = 0; looper < fLoopers.count(); looper++) {
            if (0) {
                paint.setLooper(fLoopers[looper]);
                canvas->save();
                canvas->translate(0, SkIntToScalar(y));
                canvas->drawTextBlob(fBlob, 0, 0, paint);
                canvas->restore();
            } else {
                auto b = fBlob;
                fLoopers[looper]->apply(canvas, paint, [b, y](SkCanvas* c, const SkPaint& p) {
                    c->save();
                    c->translate(0, SkIntToScalar(y));
                    c->drawTextBlob(b, 0, 0, p);
                    c->restore();
                });
            }
            y += SkScalarFloorToInt(bounds.height());
        }
    }

private:
    sk_sp<SkTextBlob> fBlob;
    SkTArray<sk_sp<SkDrawLooper>> fLoopers;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new TextBlobLooperGM;)
}
