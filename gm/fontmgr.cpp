/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "CommonFlags.h"
#include "SkCanvas.h"
#include "SkFontMetrics.h"
#include "SkFontMgr.h"
#include "SkFontPriv.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkTypeface.h"
#include "gm.h"
#include "sk_tool_utils.h"

// limit this just so we don't take too long to draw
#define MAX_FAMILIES    30

static SkScalar drawString(SkCanvas* canvas, const SkString& text, SkScalar x,
                           SkScalar y, const SkFont& font) {
    canvas->drawString(text, x, y, font, SkPaint());
    return x + font.measureText(text.c_str(), text.size(), kUTF8_SkTextEncoding);
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
public:
    FontMgrGM() {
        SkGraphics::SetFontCacheLimit(16 * 1024 * 1024);

        fName.set("fontmgr_iter");
        fFM = SkFontMgr::RefDefault();
    }

protected:
    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(1536, 768);
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar y = 20;
        SkFont font;
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setSubpixel(true);
        font.setSize(17);

        SkFontMgr* fm = fFM.get();
        int count = SkMin32(fm->countFamilies(), MAX_FAMILIES);

        for (int i = 0; i < count; ++i) {
            SkString familyName;
            fm->getFamilyName(i, &familyName);
            font.setTypeface(nullptr);
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

private:
    sk_sp<SkFontMgr> fFM;
    SkString fName;
    typedef GM INHERITED;
};

class FontMgrMatchGM : public skiagm::GM {
    sk_sp<SkFontMgr> fFM;

public:
    FontMgrMatchGM() : fFM(SkFontMgr::RefDefault()) {
        SkGraphics::SetFontCacheLimit(16 * 1024 * 1024);
    }

protected:
    SkString onShortName() override {
        return SkString("fontmgr_match");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 1024);
    }

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
        SkFont font;
        font.setEdging(SkFont::Edging::kSubpixelAntiAlias);
        font.setSubpixel(true);
        font.setSize(17);

        const char* gNames[] = {
            "Helvetica Neue", "Arial", "sans"
        };

        sk_sp<SkFontStyleSet> fset;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gNames); ++i) {
            fset.reset(fFM->matchFamily(gNames[i]));
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

private:
    typedef GM INHERITED;
};

class FontMgrBoundsGM : public skiagm::GM {
public:
    FontMgrBoundsGM(double scale, double skew)
        : fScaleX(SkDoubleToScalar(scale))
        , fSkewX(SkDoubleToScalar(skew))
    {
        fName.set("fontmgr_bounds");
        if (scale != 1 || skew != 0) {
            fName.appendf("_%g_%g", scale, skew);
        }
        fFM = SkFontMgr::RefDefault();
    }

    static void show_bounds(SkCanvas* canvas, const SkFont& font, SkScalar x, SkScalar y,
                            SkColor boundsColor)
    {
        SkRect fontBounds = SkFontPriv::GetFontBounds(font).makeOffset(x, y);

        SkPaint boundsPaint;
        boundsPaint.setAntiAlias(true);
        boundsPaint.setColor(boundsColor);
        boundsPaint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(fontBounds, boundsPaint);

        SkFontMetrics fm;
        font.getMetrics(&fm);
        SkPaint metricsPaint(boundsPaint);
        metricsPaint.setStyle(SkPaint::kFill_Style);
        metricsPaint.setAlphaf(0.25f);
        if ((fm.fFlags & SkFontMetrics::kUnderlinePositionIsValid_Flag) &&
            (fm.fFlags & SkFontMetrics::kUnderlineThicknessIsValid_Flag))
        {
            SkRect underline{ fontBounds.fLeft,  fm.fUnderlinePosition+y,
                              fontBounds.fRight, fm.fUnderlinePosition+y + fm.fUnderlineThickness };
            canvas->drawRect(underline, metricsPaint);
        }

        if ((fm.fFlags & SkFontMetrics::kStrikeoutPositionIsValid_Flag) &&
            (fm.fFlags & SkFontMetrics::kStrikeoutThicknessIsValid_Flag))
        {
            SkRect strikeout{ fontBounds.fLeft,  fm.fStrikeoutPosition+y - fm.fStrikeoutThickness,
                              fontBounds.fRight, fm.fStrikeoutPosition+y };
            canvas->drawRect(strikeout, metricsPaint);
        }

        SkGlyphID left = 0, right = 0, top = 0, bottom = 0;
        {
            int numGlyphs = font.getTypefaceOrDefault()->countGlyphs();
            SkRect min = {0, 0, 0, 0};
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
        SkGlyphID str[] = { left, right, top, bottom };
        SkPoint location[] = {
            {fontBounds.left(), fontBounds.centerY()},
            {fontBounds.right(), fontBounds.centerY()},
            {fontBounds.centerX(), fontBounds.top()},
            {fontBounds.centerX(), fontBounds.bottom()}
        };

        SkFont labelFont;
        labelFont.setEdging(SkFont::Edging::kAntiAlias);
        labelFont.setTypeface(sk_tool_utils::create_portable_typeface());

        if (FLAGS_veryVerbose) {
            SkString name;
            font.getTypefaceOrDefault()->getFamilyName(&name);
            canvas->drawString(name, fontBounds.fLeft, fontBounds.fBottom, labelFont, SkPaint());
        }
        for (size_t i = 0; i < SK_ARRAY_COUNT(str); ++i) {
            SkPath path;
            font.getPath(str[i], &path);
            path.offset(x, y);
            SkPaint::Style style = path.isEmpty() ? SkPaint::kFill_Style : SkPaint::kStroke_Style;
            SkPaint glyphPaint;
            glyphPaint.setStyle(style);
            canvas->drawSimpleText(&str[i], sizeof(str[0]), kGlyphID_SkTextEncoding, x, y, font, glyphPaint);

            if (FLAGS_veryVerbose) {
                SkString glyphStr;
                glyphStr.appendS32(str[i]);
                canvas->drawString(glyphStr, location[i].fX, location[i].fY, labelFont, SkPaint());
            }

        }

    }

protected:
    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(1024, 850);
    }

    void onDraw(SkCanvas* canvas) override {
        SkFont font;
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setSubpixel(true);
        font.setSize(100);
        font.setScaleX(fScaleX);
        font.setSkewX(fSkewX);

        const SkColor boundsColors[2] = { SK_ColorRED, SK_ColorBLUE };

        SkFontMgr* fm = fFM.get();
        int count = SkMin32(fm->countFamilies(), 32);

        int index = 0;
        SkScalar x = 0, y = 0;

        canvas->translate(10, 120);

        for (int i = 0; i < count; ++i) {
            sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
            for (int j = 0; j < set->count() && j < 3; ++j) {
                font.setTypeface(sk_sp<SkTypeface>(set->createTypeface(j)));
                // Fonts with lots of glyphs are interesting, but can take a long time to find
                // the glyphs which make up the maximum extent.
                if (font.getTypefaceOrDefault() && font.getTypefaceOrDefault()->countGlyphs() < 1000) {
                    SkRect fontBounds = SkFontPriv::GetFontBounds(font);
                    x -= fontBounds.fLeft;
                    show_bounds(canvas, font, x, y, boundsColors[index & 1]);
                    x += fontBounds.fRight + 20;
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

private:
    sk_sp<SkFontMgr> fFM;
    SkString fName;
    SkScalar fScaleX, fSkewX;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new FontMgrGM;)
DEF_GM(return new FontMgrMatchGM;)
DEF_GM(return new FontMgrBoundsGM(1.0, 0);)
DEF_GM(return new FontMgrBoundsGM(0.75, 0);)
DEF_GM(return new FontMgrBoundsGM(1.0, -0.25);)
