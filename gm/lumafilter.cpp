/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/effects/SkLumaColorFilter.h"
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkColorFilterPriv.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <string.h>

static SkScalar kSize   = 80;
static SkScalar kInset  = 10;
static SkColor  kColor1 = SkColorSetARGB(0xff, 0xff, 0xff, 0);
static SkColor  kColor2 = SkColorSetARGB(0xff, 0x82, 0xff, 0);

static void draw_label(SkCanvas* canvas, const char* label,
                       const SkPoint& offset) {
    SkFont font(ToolUtils::create_portable_typeface());
    font.setEdging(SkFont::Edging::kAlias);

    size_t len = strlen(label);

    SkScalar width = font.measureText(label, len, SkTextEncoding::kUTF8);
    canvas->drawSimpleText(label, len, SkTextEncoding::kUTF8, offset.x() - width / 2, offset.y(),
                           font, SkPaint());
}

static void draw_scene(SkCanvas* canvas, const sk_sp<SkColorFilter>& filter, SkBlendMode mode,
                       const sk_sp<SkShader>& s1, const sk_sp<SkShader>& s2) {
    SkPaint paint;
    paint.setAntiAlias(true);
    SkRect r, c, bounds = SkRect::MakeWH(kSize, kSize);

    c = bounds;
    c.fRight = bounds.centerX();
    paint.setARGB(0x20, 0, 0, 0xff);
    canvas->drawRect(bounds, paint);

    canvas->saveLayer(&bounds, nullptr);

    r = bounds;
    r.inset(kInset, 0);
    paint.setShader(s1);
    paint.setColor(s1 ? SK_ColorBLACK : SkColorSetA(kColor1, 0x80));
    canvas->drawOval(r, paint);
    if (!s1) {
        canvas->save();
        canvas->clipRect(c);
        paint.setColor(kColor1);
        canvas->drawOval(r, paint);
        canvas->restore();
    }

    SkPaint xferPaint;
    xferPaint.setBlendMode(mode);
    canvas->saveLayer(&bounds, &xferPaint);

    r = bounds;
    r.inset(0, kInset);
    paint.setShader(s2);
    paint.setColor(s2 ? SK_ColorBLACK : SkColorSetA(kColor2, 0x80));
    paint.setColorFilter(filter);
    canvas->drawOval(r, paint);
    if (!s2) {
        canvas->save();
        canvas->clipRect(c);
        paint.setColor(kColor2);
        canvas->drawOval(r, paint);
        canvas->restore();
    }

    canvas->restore();
    canvas->restore();
}

class LumaFilterGM : public skiagm::GM {
protected:
    void onOnceBeforeDraw() override {
        SkColor  g1Colors[] = { kColor1, SkColorSetA(kColor1, 0x20) };
        SkColor  g2Colors[] = { kColor2, SkColorSetA(kColor2, 0x20) };
        SkPoint  g1Points[] = { { 0, 0 }, { 0,     100 } };
        SkPoint  g2Points[] = { { 0, 0 }, { kSize, 0   } };
        SkScalar pos[] = { 0.2f, 1.0f };

        fFilter = SkLumaColorFilter::Make();
        fGr1 = SkGradientShader::MakeLinear(g1Points, g1Colors, pos, std::size(g1Colors),
                                            SkTileMode::kClamp);
        fGr2 = SkGradientShader::MakeLinear(g2Points, g2Colors, pos, std::size(g2Colors),
                                            SkTileMode::kClamp);
    }

    SkString getName() const override { return SkString("lumafilter"); }

    SkISize getISize() override { return SkISize::Make(600, 420); }

    void onDraw(SkCanvas* canvas) override {
        SkBlendMode modes[] = {
            SkBlendMode::kSrcOver,
            SkBlendMode::kDstOver,
            SkBlendMode::kSrcATop,
            SkBlendMode::kDstATop,
            SkBlendMode::kSrcIn,
            SkBlendMode::kDstIn,
        };
        struct {
            const sk_sp<SkShader>& fShader1;
            const sk_sp<SkShader>& fShader2;
        } shaders[] = {
            { nullptr, nullptr },
            { nullptr, fGr2 },
            { fGr1, nullptr },
            { fGr1, fGr2 },
        };

        SkScalar gridStep = kSize + 2 * kInset;
        for (size_t i = 0; i < std::size(modes); ++i) {
            draw_label(canvas, SkBlendMode_Name(modes[i]),
                       SkPoint::Make(gridStep * (0.5f + i), 20));
        }

        for (size_t i = 0; i < std::size(shaders); ++i) {
            canvas->save();
            canvas->translate(kInset, gridStep * i + 30);
            for (size_t m = 0; m < std::size(modes); ++m) {
                draw_scene(canvas, fFilter, modes[m], shaders[i].fShader1,
                           shaders[i].fShader2);
                canvas->translate(gridStep, 0);
            }
            canvas->restore();
        }
    }

private:
    sk_sp<SkColorFilter>    fFilter;
    sk_sp<SkShader>         fGr1, fGr2;

    using INHERITED = skiagm::GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new LumaFilterGM;)

DEF_SIMPLE_GM(AlternateLuma, canvas, 384,128) {
    sk_sp<SkImage> img = ToolUtils::GetResourceAsImage("images/mandrill_128.png");
    if (!img) {
        return;
    }

    // Normal luma colorfilter on the left.
    SkPaint paint;
    paint.setColorFilter(SkLumaColorFilter::Make());
    canvas->drawImage(img, 0,0, SkFilterMode::kNearest, &paint);
    canvas->translate(128,0);

    // Original image in the middle for reference.
    canvas->drawImage(img, 0,0);
    canvas->translate(128,0);

    // Here, RGB holds CIE XYZ. Splatting the G (Y) channel should result in (near) greyscale.
    auto [effect, err] = SkRuntimeEffect::MakeForColorFilter(SkString{
            "half4 main(half4 inColor) { return inColor.ggga; }"});
    SkASSERT(effect && err.isEmpty());

    sk_sp<SkColorFilter> filter = effect->makeColorFilter(SkData::MakeEmpty());
    SkASSERT(filter);

    SkAlphaType unpremul = kUnpremul_SkAlphaType;
    paint.setColorFilter(SkColorFilterPriv::WithWorkingFormat(std::move(filter),
                                                              &SkNamedTransferFn::kLinear,
                                                              &SkNamedGamut::kXYZ,
                                                              &unpremul));
    canvas->drawImage(img, 0,0, SkFilterMode::kNearest, &paint);
}
