/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkFontPriv.h"
#include "tools/Resources.h"
#include "tools/fonts/FontToolUtils.h"

#include <string.h>
#include <utility>

using namespace skia_private;

static void getGlyphPositions(const SkFont& font, SkSpan<const SkGlyphID> glyphs,
                              SkScalar x, SkScalar y, SkPoint pos[]) {
    const auto count = glyphs.size();
    AutoSTMalloc<128, SkScalar> widthStorage(count);
    SkScalar* widths = widthStorage.get();
    font.getWidths(glyphs, {widths, count});

    for (size_t i = 0; i < count; ++i) {
        pos[i].set(x, y);
        x += widths[i];
    }
}

static void applyKerning(SkPoint pos[], const int32_t adjustments[], int count,
                         const SkFont& font) {
    SkScalar scale = font.getSize() / font.getTypeface()->getUnitsPerEm();

    SkScalar globalAdj = 0;
    for (int i = 0; i < count - 1; ++i) {
        globalAdj += adjustments[i] * scale;
        pos[i + 1].fX += globalAdj;
    }
}

static void drawKernText(SkCanvas* canvas, const void* text, size_t len,
                         SkScalar x, SkScalar y, const SkFont& font, const SkPaint& paint) {
    SkTypeface* face = font.getTypeface();
    if (!face) {
        canvas->drawSimpleText(text, len, SkTextEncoding::kUTF8, x, y, font, paint);
        return;
    }

    AutoSTMalloc<128, SkGlyphID> glyphStorage(len);
    SkGlyphID* glyphs = glyphStorage.get();
    int glyphCount = font.textToGlyphs(text, len, SkTextEncoding::kUTF8, {glyphs, len});
    if (glyphCount < 1) {
        return;
    }

    AutoSTArray<128, int32_t> adjustmentStorage(glyphCount - 1);
    int32_t* adjustments = adjustmentStorage.get();
    if (!face->getKerningPairAdjustments({glyphs, glyphCount}, adjustmentStorage)) {
        canvas->drawSimpleText(text, len, SkTextEncoding::kUTF8, x, y, font, paint);
        return;
    }


    SkTextBlobBuilder builder;
    auto rec = builder.allocRunPos(font, glyphCount);
    memcpy(rec.glyphs, glyphs, glyphCount * sizeof(SkGlyphID));
    getGlyphPositions(font, {glyphs, glyphCount}, x, y, rec.points());
    applyKerning(rec.points(), adjustments, glyphCount, font);

    canvas->drawTextBlob(builder.make(), 0, 0, paint);
}

static constexpr SkFontStyle gStyles[] = {
    SkFontStyle::Normal(),
    SkFontStyle::Bold(),
    SkFontStyle::Italic(),
    SkFontStyle::BoldItalic(),
};

constexpr int gStylesCount = std::size(gStyles);

// TODO(bungeman) delete this GM, as it is no longer useful.
class TypefaceStylesGM : public skiagm::GM {
    sk_sp<SkTypeface> fFaces[gStylesCount];
    bool fApplyKerning;

public:
    TypefaceStylesGM(bool applyKerning) : fApplyKerning(applyKerning) {}

protected:
    void onOnceBeforeDraw() override {
        for (int i = 0; i < gStylesCount; i++) {
            fFaces[i] = ToolUtils::CreateTestTypeface(nullptr, gStyles[i]);
        }
    }

    SkString getName() const override {
        SkString name("typefacestyles");
        if (fApplyKerning) {
            name.append("_kerning");
        }
        return name;
    }

    SkISize getISize() override { return SkISize::Make(640, 480); }

    void onDraw(SkCanvas* canvas) override {
        // Need to use a font to get dy below.
        SkFont font = ToolUtils::DefaultFont();
        font.setSize(30);

        const char* text = fApplyKerning ? "Type AWAY" : "Hamburgefons";
        const size_t textLen = strlen(text);

        SkScalar x = SkIntToScalar(10);
        SkScalar dy = font.getMetrics(nullptr);
        SkASSERT(dy > 0);
        SkScalar y = dy;

        if (fApplyKerning) {
            font.setSubpixel(true);
        } else {
            font.setLinearMetrics(true);
        }

        SkPaint paint;
        for (int i = 0; i < gStylesCount; i++) {
            SkASSERT(fFaces[i]);
            font.setTypeface(fFaces[i]);
            canvas->drawSimpleText(text, textLen, SkTextEncoding::kUTF8, x, y, font, paint);
            if (fApplyKerning) {
                drawKernText(canvas, text, textLen, x + 240, y, font, paint);
            }
            y += dy;
        }
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM( return new TypefaceStylesGM(false); )
DEF_GM( return new TypefaceStylesGM(true); )

////////////////////////////////////////////////////////////////////////////////

static void draw_typeface_rendering_gm(SkCanvas* canvas, sk_sp<SkTypeface> face, SkGlyphID glyph) {
    struct AliasType {
        SkFont::Edging edging;
        bool inLayer;
    } constexpr aliasTypes[] {
#ifndef SK_BUILD_FOR_IOS
        // This gm crashes on iOS when drawing an embedded bitmap when requesting aliased rendering.
        // The crash looks like
        //   libTrueTypeScaler.dylib`<redacted> + 80
        //   stop reason = EXC_BAD_ACCESS (code=EXC_ARM_DA_ALIGN, address=...)
        //   ->  0x330b19d0 <+80>: strd   r2, r3, [r5, #36]
        //       0x330b19d4 <+84>: movs   r3, #0x0
        //       0x330b19d6 <+86>: add    r2, sp, #0x28
        //       0x330b19d8 <+88>: ldr    r0, [r4, #0x4]
        // Disable testing embedded bitmaps on iOS for now.
        // See skbug.com/40036707 .
        { SkFont::Edging::kAlias            , false },
#endif
        { SkFont::Edging::kAntiAlias        , false },
        { SkFont::Edging::kSubpixelAntiAlias, false },
        { SkFont::Edging::kAntiAlias        , true  },
        { SkFont::Edging::kSubpixelAntiAlias, true  },
    };

    // The hintgasp.ttf is designed for the following sizes to be different.
    // GASP_DOGRAY                                      0x0002   0<=ppem<=10
    // GASP_SYMMETRIC_SMOOTHING                         0x0008   0<=ppem<=10
    // GASP_GRIDFIT                                     0x0001  11<=ppem<=12
    // GASP_SYMMETRIC_GRIDFIT                           0x0004  11<=ppem<=12
    // GASP_DOGRAY|GASP_GRIDFIT                         0x0003  13<=ppem<=14
    // GASP_SYMMETRIC_SMOOTHING|GASP_SYMMETRIC_GRIDFIT  0x000C  13<=ppem<=14
    // (neither)                                        0x0000  15<=ppem
    // Odd sizes have embedded bitmaps.
    constexpr SkScalar textSizes[] = { 9, 10, 11, 12, 13, 14, 15, 16 };

    constexpr SkFontHinting hintingTypes[] = {
        SkFontHinting::kNone,
        SkFontHinting::kSlight,
        SkFontHinting::kNormal,
        SkFontHinting::kFull
    };

    struct SubpixelType {
        bool requested;
        SkVector offset;
    } constexpr subpixelTypes[] = {
        { false, { 0.00, 0.00 } },
        { true , { 0.00, 0.00 } },
        { true , { 0.25, 0.00 } },
        { true , { 0.25, 0.25 } },
    };

    constexpr bool rotateABitTypes[] = { false, true };

    SkScalar y = 0;  // The baseline of the previous output
    {
        SkPaint paint;

        SkFont font(face);
        font.setEmbeddedBitmaps(true);

        SkScalar x = 0;
        SkScalar xMax = x;
        SkScalar xBase = 0;
        for (const SubpixelType subpixel : subpixelTypes) {
            y = 0;
            font.setSubpixel(subpixel.requested);

            for (const AliasType& alias : aliasTypes) {
                font.setEdging(alias.edging);
                SkAutoCanvasRestore acr1(canvas, false);
                if (alias.inLayer) {
                    canvas->saveLayer(nullptr, &paint);
                }

                for (const SkScalar& textSize : textSizes) {
                    x = xBase + 5;
                    font.setSize(textSize);

                    SkScalar dy = SkScalarCeilToScalar(font.getMetrics(nullptr));
                    y += dy;
                    for (const SkFontHinting& hinting : hintingTypes) {
                        font.setHinting(hinting);

                        for (const bool& rotateABit : rotateABitTypes) {
                            SkAutoCanvasRestore acr2(canvas, true);
                            if (rotateABit) {
                                canvas->rotate(2, x + subpixel.offset.x(),
                                                  y + subpixel.offset.y());
                            }
                            canvas->drawSimpleText(&glyph, sizeof(glyph), SkTextEncoding::kGlyphID,
                                                   x + subpixel.offset.x(),
                                                   y + subpixel.offset.y(), font, paint);

                            SkScalar dx = SkScalarCeilToScalar(font.measureText(
                                    &glyph, sizeof(glyph), SkTextEncoding::kGlyphID)) + 5;
                            x += dx;
                            xMax = std::max(x, xMax);
                        }
                    }
                }
                y += 10;
            }
            xBase = xMax;
        }
    }

    constexpr struct StyleTests {
        SkPaint::Style style;
        SkScalar strokeWidth;
    } styleTypes[] = {
        { SkPaint::kFill_Style, 0.0f},
        { SkPaint::kStroke_Style, 0.0f},
        { SkPaint::kStroke_Style, 0.5f},
        { SkPaint::kStrokeAndFill_Style, 1.0f},
    };

    constexpr bool fakeBoldTypes[] = { false, true };

    {
        SkPaint paint;

        SkFont font(face, 16);

        SkScalar x = 0;
        for (const bool& fakeBold : fakeBoldTypes) {
            SkScalar dy = SkScalarCeilToScalar(font.getMetrics(nullptr));
            y += dy;
            x = 5;

            font.setEmbolden(fakeBold);
            for (const AliasType& alias : aliasTypes) {
                font.setEdging(alias.edging);
                SkAutoCanvasRestore acr(canvas, false);
                if (alias.inLayer) {
                    canvas->saveLayer(nullptr, &paint);
                }
                for (const StyleTests& style : styleTypes) {
                    paint.setStyle(style.style);
                    paint.setStrokeWidth(style.strokeWidth);
                    canvas->drawSimpleText(&glyph, sizeof(glyph), SkTextEncoding::kGlyphID,
                                           x, y, font, paint);

                    SkScalar dx = SkScalarCeilToScalar(font.measureText(
                            &glyph, sizeof(glyph), SkTextEncoding::kGlyphID)) + 5;
                    x += dx;
                }
            }
            y += 10;
        }
    }

    constexpr struct MaskTests {
        SkBlurStyle style;
        SkScalar sigma;
    } maskTypes[] = {
        { SkBlurStyle::kNormal_SkBlurStyle, 0.0f},
        { SkBlurStyle::kSolid_SkBlurStyle, 0.0f},
        { SkBlurStyle::kOuter_SkBlurStyle, 0.0f},
        { SkBlurStyle::kInner_SkBlurStyle, 0.0f},

        { SkBlurStyle::kNormal_SkBlurStyle, 0.5f},
        { SkBlurStyle::kSolid_SkBlurStyle, 0.5f},
        { SkBlurStyle::kOuter_SkBlurStyle, 0.5f},
        { SkBlurStyle::kInner_SkBlurStyle, 0.5f},

        { SkBlurStyle::kNormal_SkBlurStyle, 2.0f},
        { SkBlurStyle::kSolid_SkBlurStyle, 2.0f},
        { SkBlurStyle::kOuter_SkBlurStyle, 2.0f},
        { SkBlurStyle::kInner_SkBlurStyle, 2.0f},
    };

    {
        SkPaint paint;

        SkFont font(face, 16);

        SkScalar x = 0;
        {
            for (const AliasType& alias : aliasTypes) {
                SkScalar dy = SkScalarCeilToScalar(font.getMetrics(nullptr));
                y += dy;
                x = 5;

                font.setEdging(alias.edging);
                SkAutoCanvasRestore acr(canvas, false);
                if (alias.inLayer) {
                    canvas->saveLayer(nullptr, &paint);
                }
                for (const MaskTests& mask : maskTypes) {
                    paint.setMaskFilter(SkMaskFilter::MakeBlur(mask.style, mask.sigma));
                    canvas->drawSimpleText(&glyph, sizeof(glyph), SkTextEncoding::kGlyphID,
                                           x, y, font, paint);

                    SkScalar dx = SkScalarCeilToScalar(font.measureText(
                            &glyph, sizeof(glyph), SkTextEncoding::kGlyphID)) + 5;
                    x += dx;
                }
                paint.setMaskFilter(nullptr);
            }
            y += 10;
        }
    }
}

DEF_SIMPLE_GM_CAN_FAIL(typefacerendering, canvas, errMsg, 640, 840) {
    sk_sp<SkTypeface> face = ToolUtils::CreateTypefaceFromResource("fonts/hintgasp.ttf");
    if (!face) {
        return skiagm::DrawResult::kSkip;
    }
    draw_typeface_rendering_gm(canvas, face, face->unicharToGlyph('A'));

    // Should draw nothing and not do anything undefined.
    draw_typeface_rendering_gm(canvas, face, 0xFFFF);
    return skiagm::DrawResult::kOk;
}

// Type1 fonts don't currently work in Skia on Windows.
#ifndef SK_BUILD_FOR_WIN

DEF_SIMPLE_GM_CAN_FAIL(typefacerendering_pfa, canvas, errMsg, 640, 840) {
    sk_sp<SkTypeface> face = ToolUtils::CreateTypefaceFromResource("fonts/Roboto2-Regular.pfa");
    if (!face) {
       return skiagm::DrawResult::kSkip;
    }
    draw_typeface_rendering_gm(canvas, face, face->unicharToGlyph('O'));
    return skiagm::DrawResult::kOk;
}

DEF_SIMPLE_GM_CAN_FAIL(typefacerendering_pfb, canvas, errMsg, 640, 840) {
    sk_sp<SkTypeface> face = ToolUtils::CreateTypefaceFromResource("fonts/Roboto2-Regular.pfb");
    if (!face) {
        return skiagm::DrawResult::kSkip;
    }
    draw_typeface_rendering_gm(canvas, face, face->unicharToGlyph('O'));
    return skiagm::DrawResult::kOk;
}

#endif

////////////////////////////////////////////////////////////////////////////////

// Exercise different paint styles and embolden, and compare with strokeandfill patheffect
DEF_SIMPLE_GM(typeface_styling, canvas, 710, 360) {
    sk_sp<SkTypeface> face = ToolUtils::CreateTypefaceFromResource("fonts/Roboto-Regular.ttf");
    if (!face) {
        face = ToolUtils::DefaultPortableTypeface();
    }
    SkFont font(face, 100);
    font.setEdging(SkFont::Edging::kAntiAlias);

    SkGlyphID glyphs[1] = { font.unicharToGlyph('A') };
    SkPoint pos[1] = { {0, 0} };

    auto draw = [&](SkPaint::Style style, float width) {
        // Draws 3 rows:
        //  1. normal
        //  2. emboldened
        //  3. normal(white) on top of emboldened (to show the delta)

        SkPaint paint;
        paint.setStyle(style);
        paint.setStrokeWidth(width);

        font.setEmbolden(true);
        canvas->drawGlyphs(glyphs, pos, {20, 120*2}, font, paint);
        canvas->drawGlyphs(glyphs, pos, {20, 120*3}, font, paint);

        font.setEmbolden(false);
        canvas->drawGlyphs(glyphs, pos, {20, 120*1}, font, paint);
        paint.setColor(SK_ColorYELLOW);
        canvas->drawGlyphs(glyphs, pos, {20, 120*3}, font, paint);
    };

    const struct {
        SkPaint::Style  style;
        float           width;
    } recs[] = {
        { SkPaint::kFill_Style,             0 },
        { SkPaint::kStroke_Style,           0 },
        { SkPaint::kStroke_Style,           3 },
        { SkPaint::kStrokeAndFill_Style,    0 },
        { SkPaint::kStrokeAndFill_Style,    3 },
    };

    canvas->translate(0, -20);
    for (auto r : recs) {
        draw(r.style, r.width);
        canvas->translate(100, 0);
    }
}
