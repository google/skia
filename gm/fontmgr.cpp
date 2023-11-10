/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "src/core/SkFontPriv.h"
#include "tools/SkMetaData.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"

#include <utility>

// limit this just so we don't take too long to draw
#define MAX_FAMILIES    30

static SkScalar drawString(SkCanvas* canvas, const SkString& text, SkScalar x,
                           SkScalar y, const SkFont& font) {
    canvas->drawString(text, x, y, font, SkPaint());
    return x + font.measureText(text.c_str(), text.size(), SkTextEncoding::kUTF8);
}

static SkScalar drawCharacter(SkCanvas* canvas, uint32_t character, SkScalar x,
                              SkScalar y, const SkFont& origFont, SkFontMgr* fm,
                              const char* fontName, const char* bcp47[], int bcp47Count,
                              const SkFontStyle& fontStyle) {
    SkFont font = origFont;
    // find typeface containing the requested character and draw it
    SkString ch;
    ch.appendUnichar(character);
    sk_sp<SkTypeface> typeface(fm->matchFamilyStyleCharacter(fontName, fontStyle,
                                                             bcp47, bcp47Count, character));
    font.setTypeface(typeface);
    x = drawString(canvas, ch, x, y, font) + 20;

    if (nullptr == typeface) {
        return x;
    }

    // repeat the process, but this time use the family name of the typeface
    // from the first pass.  This emulates the behavior in Blink where it
    // it expects to get the same glyph when following this pattern.
    SkString familyName;
    typeface->getFamilyName(&familyName);
    font.setTypeface(fm->legacyMakeTypeface(familyName.c_str(), typeface->fontStyle()));
    return drawString(canvas, ch, x, y, font) + 20;
}

static const char* zh = "zh";
static const char* ja = "ja";

class FontMgrGM : public skiagm::GM {
    sk_sp<SkFontMgr> fFM;

    void onOnceBeforeDraw() override {
        SkGraphics::SetFontCacheLimit(16 * 1024 * 1024);
        fFM = SkFontMgr::RefDefault();
    }

    SkString getName() const override { return SkString("fontmgr_iter"); }

    SkISize getISize() override { return {1536, 768}; }

    void onDraw(SkCanvas* canvas) override {
        SkScalar y = 20;
        SkFont font = ToolUtils::DefaultFont();
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setSubpixel(true);
        font.setSize(17);

        SkFontMgr* fm = fFM.get();
        int count = std::min(fm->countFamilies(), MAX_FAMILIES);

        for (int i = 0; i < count; ++i) {
            SkString familyName;
            fm->getFamilyName(i, &familyName);
            font.setTypeface(ToolUtils::DefaultTypeface());
            (void)drawString(canvas, familyName, 20, y, font);

            SkScalar x = 220;

            sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
            for (int j = 0; j < set->count(); ++j) {
                SkString sname;
                SkFontStyle fs;
                set->getStyle(j, &fs, &sname);
                sname.appendf(" [%d %d %d]", fs.weight(), fs.width(), fs.slant());

                font.setTypeface(sk_sp<SkTypeface>(set->createTypeface(j)));
                x = drawString(canvas, sname, x, y, font) + 20;

                // check to see that we get different glyphs in japanese and chinese
                x = drawCharacter(canvas, 0x5203, x, y, font, fm, familyName.c_str(), &zh, 1, fs);
                x = drawCharacter(canvas, 0x5203, x, y, font, fm, familyName.c_str(), &ja, 1, fs);
                // check that emoji characters are found
                x = drawCharacter(canvas, 0x1f601, x, y, font, fm, familyName.c_str(), nullptr,0, fs);
            }
            y += 24;
        }
    }
};

class FontMgrMatchGM : public skiagm::GM {
    sk_sp<SkFontMgr> fFM;

    void onOnceBeforeDraw() override {
        fFM = SkFontMgr::RefDefault();
        SkGraphics::SetFontCacheLimit(16 * 1024 * 1024);
    }

    SkString getName() const override { return SkString("fontmgr_match"); }

    SkISize getISize() override { return {640, 1024}; }

    void iterateFamily(SkCanvas* canvas, const SkFont& font, SkFontStyleSet* fset) {
        SkFont f(font);
        SkScalar y = 0;

        for (int j = 0; j < fset->count(); ++j) {
            SkString sname;
            SkFontStyle fs;
            fset->getStyle(j, &fs, &sname);

            sname.appendf(" [%d %d]", fs.weight(), fs.width());

            f.setTypeface(sk_sp<SkTypeface>(fset->createTypeface(j)));
            (void)drawString(canvas, sname, 0, y, f);
            y += 24;
        }
    }

    void exploreFamily(SkCanvas* canvas, const SkFont& font, SkFontStyleSet* fset) {
        SkFont f(font);
        SkScalar y = 0;

        for (int weight = 100; weight <= 900; weight += 200) {
            for (int width = 1; width <= 9; width += 2) {
                SkFontStyle fs(weight, width, SkFontStyle::kUpright_Slant);
                sk_sp<SkTypeface> face(fset->matchStyle(fs));
                if (face) {
                    SkString str;
                    str.printf("request [%d %d]", fs.weight(), fs.width());
                    f.setTypeface(std::move(face));
                    (void)drawString(canvas, str, 0, y, f);
                    y += 24;
                }
            }
        }
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        SkFont font = ToolUtils::DefaultFont();
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setSubpixel(true);
        font.setSize(17);

        const char* gNames[] = {
            "Helvetica Neue", "Arial", "sans"
        };

        sk_sp<SkFontStyleSet> fset;
        for (size_t i = 0; i < std::size(gNames); ++i) {
            fset = fFM->matchFamily(gNames[i]);
            if (fset->count() > 0) {
                break;
            }
        }
        if (nullptr == fset.get()) {
            *errorMsg = "No SkFontStyleSet";
            return DrawResult::kFail;
        }

        canvas->translate(20, 40);
        this->exploreFamily(canvas, font, fset.get());
        canvas->translate(150, 0);
        this->iterateFamily(canvas, font, fset.get());
        return DrawResult::kOk;
    }
};

class FontMgrBoundsGM : public skiagm::GM {
public:
    FontMgrBoundsGM(float scale, float skew) : fScaleX(scale) , fSkewX(skew) {}

private:
    SkString getName() const override {
        if (fScaleX != 1 || fSkewX != 0) {
            return SkStringPrintf("fontmgr_bounds_%g_%g", fScaleX, fSkewX);
        }
        return SkString("fontmgr_bounds");
    }

    void onOnceBeforeDraw() override {
        fFM = SkFontMgr::RefDefault();
    }

    bool onGetControls(SkMetaData* controls) override {
        controls->setBool("Label Bounds", fLabelBounds);
        return true;
    }

    void onSetControls(const SkMetaData& controls) override {
        controls.findBool("Label Bounds", &fLabelBounds);
    }

    static SkRect show_bounds(SkCanvas* canvas, const SkFont& font, SkScalar x, SkScalar y,
                              SkColor boundsColor, bool labelBounds)
    {
        SkGlyphID left = 0, right = 0, top = 0, bottom = 0;
        SkRect min = SkRect::MakeLTRB(SK_ScalarInfinity, SK_ScalarInfinity,
                                      SK_ScalarNegativeInfinity, SK_ScalarNegativeInfinity);
        {
            int numGlyphs = SkFontPriv::GetTypefaceOrDefault(font)->countGlyphs();
            for (int i = 0; i < numGlyphs; ++i) {
                SkGlyphID glyphId = i;
                SkRect cur;
                font.getBounds(&glyphId, 1, &cur, nullptr);
                if (cur.fLeft   < min.fLeft  ) { min.fLeft   = cur.fLeft;   left   = i; }
                if (cur.fTop    < min.fTop   ) { min.fTop    = cur.fTop ;   top    = i; }
                if (min.fRight  < cur.fRight ) { min.fRight  = cur.fRight;  right  = i; }
                if (min.fBottom < cur.fBottom) { min.fBottom = cur.fBottom; bottom = i; }
            }
        }

        SkRect fontBounds = SkFontPriv::GetFontBounds(font);

        SkRect drawBounds = min;
        drawBounds.join(fontBounds);

        SkAutoCanvasRestore acr(canvas, true);
        canvas->translate(x - drawBounds.left(), y);

        SkPaint boundsPaint;
        boundsPaint.setAntiAlias(true);
        boundsPaint.setColor(boundsColor);
        boundsPaint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(fontBounds, boundsPaint);

        const SkScalar intervals[] = { 10.f, 10.f };
        boundsPaint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0.f));
        canvas->drawRect(min, boundsPaint);

        SkFontMetrics fm;
        font.getMetrics(&fm);
        SkPaint metricsPaint(boundsPaint);
        metricsPaint.setStyle(SkPaint::kFill_Style);
        metricsPaint.setAlphaf(0.25f);
        if ((fm.fFlags & SkFontMetrics::kUnderlinePositionIsValid_Flag) &&
            (fm.fFlags & SkFontMetrics::kUnderlineThicknessIsValid_Flag))
        {
            SkRect underline{ min.fLeft,  fm.fUnderlinePosition,
                              min.fRight, fm.fUnderlinePosition + fm.fUnderlineThickness };
            canvas->drawRect(underline, metricsPaint);
        }

        if ((fm.fFlags & SkFontMetrics::kStrikeoutPositionIsValid_Flag) &&
            (fm.fFlags & SkFontMetrics::kStrikeoutThicknessIsValid_Flag))
        {
            SkRect strikeout{ min.fLeft,  fm.fStrikeoutPosition - fm.fStrikeoutThickness,
                              min.fRight, fm.fStrikeoutPosition };
            canvas->drawRect(strikeout, metricsPaint);
        }

        struct GlyphToDraw {
            SkGlyphID id;
            SkPoint location;
            SkScalar rotation;
        } glyphsToDraw [] = {
            {left,   {min.left(),    min.centerY()}, 270},
            {right,  {min.right(),   min.centerY()},  90},
            {top,    {min.centerX(), min.top()    },   0},
            {bottom, {min.centerX(), min.bottom() }, 180},
        };

        SkFont labelFont;
        labelFont.setEdging(SkFont::Edging::kAntiAlias);
        labelFont.setTypeface(ToolUtils::DefaultPortableTypeface());

        if (labelBounds) {
            SkString name;
            SkFontPriv::GetTypefaceOrDefault(font)->getFamilyName(&name);
            canvas->drawString(name, min.fLeft, min.fBottom, labelFont, SkPaint());
        }
        for (const GlyphToDraw& glyphToDraw : glyphsToDraw) {
            SkPath path;
            font.getPath(glyphToDraw.id, &path);
            SkPaint::Style style = path.isEmpty() ? SkPaint::kFill_Style : SkPaint::kStroke_Style;
            SkPaint glyphPaint;
            glyphPaint.setStyle(style);
            canvas->drawSimpleText(&glyphToDraw.id, sizeof(glyphToDraw.id),
                                   SkTextEncoding::kGlyphID, 0, 0, font, glyphPaint);

            if (labelBounds) {
                SkAutoCanvasRestore acr2(canvas, true);
                canvas->translate(glyphToDraw.location.fX, glyphToDraw.location.fY);
                canvas->rotate(glyphToDraw.rotation);
                SkString glyphStr;
                glyphStr.appendS32(glyphToDraw.id);
                canvas->drawString(glyphStr, 0, 0, labelFont, SkPaint());
            }
        }

        return drawBounds;
    }

    SkISize getISize() override { return {1024, 850}; }

    void onDraw(SkCanvas* canvas) override {
        SkFont font = ToolUtils::DefaultFont();
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setSubpixel(true);
        font.setSize(100);
        font.setScaleX(fScaleX);
        font.setSkewX(fSkewX);

        const SkColor boundsColors[2] = { SK_ColorRED, SK_ColorBLUE };

        SkFontMgr* fm = fFM.get();
        int count = std::min(fm->countFamilies(), 32);

        int index = 0;
        SkScalar x = 0, y = 0;

        canvas->translate(10, 120);

        for (int i = 0; i < count; ++i) {
            sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
            for (int j = 0; j < set->count() && j < 3; ++j) {
                font.setTypeface(sk_sp<SkTypeface>(set->createTypeface(j)));
                // Fonts with lots of glyphs are interesting, but can take a long time to find
                // the glyphs which make up the maximum extent.
                SkTypeface* typeface = SkFontPriv::GetTypefaceOrDefault(font);
                if (typeface && 0 < typeface->countGlyphs() && typeface->countGlyphs() < 1000) {
                    SkColor color = boundsColors[index & 1];
                    SkRect drawBounds = show_bounds(canvas, font, x, y, color, fLabelBounds);
                    x += drawBounds.width() + 20;
                    index += 1;
                    if (x > 900) {
                        x = 0;
                        y += 160;
                    }
                    if (y >= 700) {
                        return;
                    }
                }
            }
        }
    }

    sk_sp<SkFontMgr> fFM;
    const SkScalar fScaleX;
    const SkScalar fSkewX;
    bool fLabelBounds = false;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new FontMgrGM;)
DEF_GM(return new FontMgrMatchGM;)
DEF_GM(return new FontMgrBoundsGM(1, 0);)
DEF_GM(return new FontMgrBoundsGM(0.75f, 0);)
DEF_GM(return new FontMgrBoundsGM(1, -0.25f);)
