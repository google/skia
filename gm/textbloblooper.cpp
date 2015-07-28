/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "Sk2DPathEffect.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkColorFilter.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkLayerDrawLooper.h"
#include "SkRandom.h"
#include "SkTextBlob.h"

namespace skiagm {

static const int kWidth = 1250;
static const int kHeight = 700;

// Unlike the variant in sk_tool_utils, this version positions the glyphs on a diagonal
static void add_to_text_blob(SkTextBlobBuilder* builder, const char* text, const SkPaint& origPaint,
                             SkScalar x, SkScalar y) {
    SkPaint paint(origPaint);
    SkTDArray<uint16_t> glyphs;

    size_t len = strlen(text);
    glyphs.append(paint.textToGlyphs(text, len, NULL));
    paint.textToGlyphs(text, len, glyphs.begin());

    const SkScalar advanceX = paint.getTextSize() * 0.85f;
    const SkScalar advanceY = paint.getTextSize() * 1.5f;

    SkTDArray<SkScalar> pos;
    for (unsigned i = 0; i < len; ++i) {
        *pos.append() = x + i * advanceX;
        *pos.append() = y + i * (advanceY / len);
    }

    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    const SkTextBlobBuilder::RunBuffer& run = builder->allocRunPos(paint, glyphs.count());
    memcpy(run.glyphs, glyphs.begin(), glyphs.count() * sizeof(uint16_t));
    memcpy(run.pos, pos.begin(), len * sizeof(SkScalar) * 2);
}

typedef void (*LooperProc)(SkPaint*);

struct LooperSettings {
    SkXfermode::Mode fMode;
    SkColor          fColor;
    SkPaint::Style   fStyle;
    SkScalar         fWidth;
    SkScalar         fOffset;
    SkScalar         fSkewX;
    bool             fEffect;
};

static void mask_filter(SkPaint* paint) {
    SkMaskFilter* mf = SkBlurMaskFilter::Create(kNormal_SkBlurStyle,
                        SkBlurMask::ConvertRadiusToSigma(3.f));
    paint->setMaskFilter(mf)->unref();
}

static SkPathEffect* make_tile_effect() {
    SkMatrix m;
    m.setScale(1.f, 1.f);

    SkPath path;
    path.addCircle(0, 0, SkIntToScalar(5));

    return SkPath2DPathEffect::Create(m, path);
}

static void path_effect(SkPaint* paint) {
    paint->setPathEffect(make_tile_effect())->unref();
}

static SkShader* make_shader(const SkRect& bounds) {
    const SkPoint pts[] = {
        { bounds.left(), bounds.top() },
        { bounds.right(), bounds.bottom() },
    };
    const SkColor colors[] = {
        SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorBLACK,
        SK_ColorCYAN, SK_ColorMAGENTA, SK_ColorYELLOW,
    };
    return SkGradientShader::CreateLinear(pts,
                                          colors, NULL, SK_ARRAY_COUNT(colors),
                                          SkShader::kClamp_TileMode);
}

static void color_filter(SkPaint* paint) {
    SkRect r;
    r.setWH(SkIntToScalar(kWidth), 50);
    paint->setShader(make_shader(r))->unref();
    paint->setColorFilter(SkColorFilter::CreateLightingFilter(0xF0F0F0, 0))->unref();
}

static void kitchen_sink(SkPaint* paint) {
    color_filter(paint);
    path_effect(paint);
    mask_filter(paint);

}

static SkLayerDrawLooper* setupLooper(SkLayerDrawLooper::BitFlags bits,
                                      LooperProc proc,
                                      const LooperSettings settings[],
                                      size_t size) {
    SkLayerDrawLooper::Builder looperBuilder;

    SkLayerDrawLooper::LayerInfo info;
    info.fPaintBits = bits;

    info.fColorMode = SkXfermode::kSrc_Mode;

    for (size_t i = 0; i < size; i++) {
        info.fOffset.set(settings[i].fOffset, settings[i].fOffset);
        SkPaint* paint = looperBuilder.addLayer(info);
        paint->setXfermodeMode(settings[i].fMode);
        paint->setColor(settings[i].fColor);
        paint->setStyle(settings[i].fStyle);
        paint->setStrokeWidth(settings[i].fWidth);
        if (settings[i].fEffect) {
            (*proc)(paint);
        }
    }
    return looperBuilder.detachLooper();
}

class TextBlobLooperGM : public GM {
public:
    TextBlobLooperGM() {}

protected:
    void onOnceBeforeDraw() override {
        SkTextBlobBuilder builder;

        // LCD
        SkPaint paint;
        paint.setTextSize(32);
        const char* text = "The quick brown fox jumps over the lazy dog";
        paint.setSubpixelText(true);
        paint.setLCDRenderText(true);
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        add_to_text_blob(&builder, text, paint, 0, 0);
        fBlob.reset(builder.build());

        // create a looper which sandwhiches an effect in two normal draws
        LooperSettings looperSandwhich[] = {
           { SkXfermode::kSrc_Mode, SK_ColorMAGENTA, SkPaint::kFill_Style, 0, 0, 0, false },
           { SkXfermode::kSrcOver_Mode, 0x88000000, SkPaint::kFill_Style, 0, 10.f, 0, true },
           { SkXfermode::kSrcOver_Mode, 0x50FF00FF, SkPaint::kFill_Style, 0, 20.f, 0, false },
        };

        LooperSettings compound[] = {
            { SkXfermode::kSrc_Mode, SK_ColorWHITE, SkPaint::kStroke_Style, 1.f * 3/4, 0, 0, false },
            { SkXfermode::kSrc_Mode, SK_ColorRED, SkPaint::kStroke_Style, 4.f, 0, 0, false },
            { SkXfermode::kSrc_Mode, SK_ColorBLUE, SkPaint::kFill_Style, 0, 0, 0, false },
            { SkXfermode::kSrcOver_Mode, 0x88000000, SkPaint::kFill_Style, 0, 10.f, 0, true }
        };

        LooperSettings xfermode[] = {
            { SkXfermode::kDifference_Mode, SK_ColorWHITE, SkPaint::kFill_Style, 0, 0, 0, false },
            { SkXfermode::kSrcOver_Mode, 0xFF000000, SkPaint::kFill_Style, 0, 1.f, 0, true },
            { SkXfermode::kSrcOver_Mode, 0x50FF00FF, SkPaint::kFill_Style, 0, 2.f, 0, false },
        };

        // NOTE, this should be ignored by textblobs
        LooperSettings skew[] = {
            { SkXfermode::kSrc_Mode, SK_ColorRED, SkPaint::kFill_Style, 0, 0, -1.f, false },
            { SkXfermode::kSrc_Mode, SK_ColorGREEN, SkPaint::kFill_Style, 0, 10.f, -1.f, false },
            { SkXfermode::kSrc_Mode, SK_ColorBLUE, SkPaint::kFill_Style, 0, 20.f, -1.f, false },
        };

        LooperSettings kitchenSink[] = {
            { SkXfermode::kSrc_Mode, SK_ColorWHITE, SkPaint::kStroke_Style, 1.f * 3/4, 0, 0, false },
            { SkXfermode::kSrc_Mode, SK_ColorBLACK, SkPaint::kFill_Style, 0, 0, 0, false },
            { SkXfermode::kDifference_Mode, SK_ColorWHITE, SkPaint::kFill_Style, 1.f, 10.f, 0, false },
            { SkXfermode::kSrc_Mode, SK_ColorWHITE, SkPaint::kFill_Style, 0, 10.f, 0, true },
            { SkXfermode::kSrcOver_Mode, 0x50FF00FF, SkPaint::kFill_Style, 0, 20.f, 0, false },
        };

        fLoopers.push_back().reset(setupLooper(SkLayerDrawLooper::kMaskFilter_Bit |
                                               SkLayerDrawLooper::kXfermode_Bit |
                                               SkLayerDrawLooper::kStyle_Bit, &mask_filter,
                                               compound, SK_ARRAY_COUNT(compound)));
        fLoopers.push_back().reset(setupLooper(SkLayerDrawLooper::kPathEffect_Bit |
                                               SkLayerDrawLooper::kXfermode_Bit, &path_effect,
                                               looperSandwhich, SK_ARRAY_COUNT(looperSandwhich)));
        fLoopers.push_back().reset(setupLooper(SkLayerDrawLooper::kShader_Bit |
                                               SkLayerDrawLooper::kColorFilter_Bit |
                                               SkLayerDrawLooper::kXfermode_Bit, &color_filter,
                                               looperSandwhich, SK_ARRAY_COUNT(looperSandwhich)));
        fLoopers.push_back().reset(setupLooper(SkLayerDrawLooper::kShader_Bit |
                                               SkLayerDrawLooper::kColorFilter_Bit |
                                               SkLayerDrawLooper::kXfermode_Bit, &color_filter,
                                               xfermode, SK_ARRAY_COUNT(xfermode)));
        fLoopers.push_back().reset(setupLooper(0, NULL, skew, SK_ARRAY_COUNT(skew)));
        fLoopers.push_back().reset(setupLooper(SkLayerDrawLooper::kMaskFilter_Bit |
                                               SkLayerDrawLooper::kShader_Bit |
                                               SkLayerDrawLooper::kColorFilter_Bit |
                                               SkLayerDrawLooper::kPathEffect_Bit |
                                               SkLayerDrawLooper::kStyle_Bit |
                                               SkLayerDrawLooper::kXfermode_Bit, &kitchen_sink,
                                               kitchenSink, SK_ARRAY_COUNT(kitchenSink)));

        // Test we respect overrides
        fLoopers.push_back().reset(setupLooper(0, &kitchen_sink,
                                               kitchenSink, SK_ARRAY_COUNT(kitchenSink)));
    }

    SkString onShortName() override {
        return SkString("textbloblooper");
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, kHeight);
    }

    void onDraw(SkCanvas* canvas) override {

        canvas->drawColor(sk_tool_utils::color_to_565(SK_ColorGRAY));

        SkPaint paint;
        canvas->translate(10, 40);

        paint.setTextSize(40);

        SkRect bounds = fBlob->bounds();

        int y = 0;
        for (int looper = 0; looper < fLoopers.count(); looper++) {
            paint.setLooper(fLoopers[looper]);
            canvas->save();
            canvas->translate(0, SkIntToScalar(y));
            canvas->drawTextBlob(fBlob, 0, 0, paint);
            canvas->restore();
            y += SkScalarFloorToInt(bounds.height());
        }
    }

private:
    SkAutoTUnref<const SkTextBlob> fBlob;
    SkTArray<SkAutoTUnref<SkLayerDrawLooper>, true> fLoopers;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(TextBlobLooperGM); )
}
