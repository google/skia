/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkFontMgr.h"
#include "SkPath.h"
#include "SkGraphics.h"
#include "SkTypeface.h"

// limit this just so we don't take too long to draw
#define MAX_FAMILIES    30

static SkScalar drawString(SkCanvas* canvas, const SkString& text, SkScalar x,
                           SkScalar y, const SkPaint& paint) {
    canvas->drawString(text, x, y, paint);
    return x + paint.measureText(text.c_str(), text.size());
}

static SkScalar drawCharacter(SkCanvas* canvas, uint32_t character, SkScalar x,
                              SkScalar y, SkPaint& paint, SkFontMgr* fm,
                              const char* fontName, const char* bcp47[], int bcp47Count,
                              const SkFontStyle& fontStyle) {
    // find typeface containing the requested character and draw it
    SkString ch;
    ch.appendUnichar(character);
    sk_sp<SkTypeface> typeface(fm->matchFamilyStyleCharacter(fontName, fontStyle,
                                                             bcp47, bcp47Count, character));
    paint.setTypeface(typeface);
    x = drawString(canvas, ch, x, y, paint) + 20;

    if (nullptr == typeface) {
        return x;
    }

    // repeat the process, but this time use the family name of the typeface
    // from the first pass.  This emulates the behavior in Blink where it
    // it expects to get the same glyph when following this pattern.
    SkString familyName;
    typeface->getFamilyName(&familyName);
    paint.setTypeface(fm->legacyMakeTypeface(familyName.c_str(), typeface->fontStyle()));
    return drawString(canvas, ch, x, y, paint) + 20;
}

static const char* zh = "zh";
static const char* ja = "ja";

class FontMgrGM : public skiagm::GM {
public:
    FontMgrGM() {
        SkGraphics::SetFontCacheLimit(16 * 1024 * 1024);

        fName.set("fontmgr_iter");
        fFM = SkFontMgr::RefDefault();
        fName.append(sk_tool_utils::platform_font_manager());
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
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setSubpixelText(true);
        paint.setTextSize(17);

        SkFontMgr* fm = fFM.get();
        int count = SkMin32(fm->countFamilies(), MAX_FAMILIES);

        for (int i = 0; i < count; ++i) {
            SkString familyName;
            fm->getFamilyName(i, &familyName);
            paint.setTypeface(nullptr);
            (void)drawString(canvas, familyName, 20, y, paint);

            SkScalar x = 220;

            sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
            for (int j = 0; j < set->count(); ++j) {
                SkString sname;
                SkFontStyle fs;
                set->getStyle(j, &fs, &sname);
                sname.appendf(" [%d %d %d]", fs.weight(), fs.width(), fs.slant());

                paint.setTypeface(sk_sp<SkTypeface>(set->createTypeface(j)));
                x = drawString(canvas, sname, x, y, paint) + 20;

                // check to see that we get different glyphs in japanese and chinese
                x = drawCharacter(canvas, 0x5203, x, y, paint, fm, familyName.c_str(), &zh, 1, fs);
                x = drawCharacter(canvas, 0x5203, x, y, paint, fm, familyName.c_str(), &ja, 1, fs);
                // check that emoji characters are found
                x = drawCharacter(canvas, 0x1f601, x, y, paint, fm, familyName.c_str(), nullptr,0, fs);
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
        SkString name("fontmgr_match");
        name.append(sk_tool_utils::platform_font_manager());
        return name;
    }

    SkISize onISize() override {
        return SkISize::Make(640, 1024);
    }

    void iterateFamily(SkCanvas* canvas, const SkPaint& paint,
                       SkFontStyleSet* fset) {
        SkPaint p(paint);
        SkScalar y = 0;

        for (int j = 0; j < fset->count(); ++j) {
            SkString sname;
            SkFontStyle fs;
            fset->getStyle(j, &fs, &sname);

            sname.appendf(" [%d %d]", fs.weight(), fs.width());

            p.setTypeface(sk_sp<SkTypeface>(fset->createTypeface(j)));
            (void)drawString(canvas, sname, 0, y, p);
            y += 24;
        }
    }

    void exploreFamily(SkCanvas* canvas, const SkPaint& paint,
                       SkFontStyleSet* fset) {
        SkPaint p(paint);
        SkScalar y = 0;

        for (int weight = 100; weight <= 900; weight += 200) {
            for (int width = 1; width <= 9; width += 2) {
                SkFontStyle fs(weight, width, SkFontStyle::kUpright_Slant);
                sk_sp<SkTypeface> face(fset->matchStyle(fs));
                if (face) {
                    SkString str;
                    str.printf("request [%d %d]", fs.weight(), fs.width());
                    p.setTypeface(std::move(face));
                    (void)drawString(canvas, str, 0, y, p);
                    y += 24;
                }
            }
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setSubpixelText(true);
        paint.setTextSize(17);

        const char* gNames[] = {
            "Helvetica Neue", "Arial"
        };

        sk_sp<SkFontStyleSet> fset;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gNames); ++i) {
            fset.reset(fFM->matchFamily(gNames[i]));
            if (fset->count() > 0) {
                break;
            }
        }
        if (nullptr == fset.get()) {
            return;
        }

        canvas->translate(20, 40);
        this->exploreFamily(canvas, paint, fset.get());
        canvas->translate(150, 0);
        this->iterateFamily(canvas, paint, fset.get());
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
        fName.append(sk_tool_utils::platform_font_manager());
        fFM = SkFontMgr::RefDefault();
    }

    static void show_bounds(SkCanvas* canvas, const SkPaint& paint, SkScalar x, SkScalar y,
                            SkColor boundsColor)
    {
        SkPaint glyphPaint(paint);
        SkRect fontBounds = glyphPaint.getFontBounds();
        fontBounds.offset(x, y);
        SkPaint boundsPaint(glyphPaint);
        boundsPaint.setColor(boundsColor);
        boundsPaint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(fontBounds, boundsPaint);

        SkPaint::FontMetrics fm;
        glyphPaint.getFontMetrics(&fm);
        SkPaint metricsPaint(boundsPaint);
        metricsPaint.setStyle(SkPaint::kFill_Style);
        metricsPaint.setAlpha(0x40);
        if ((fm.fFlags & SkPaint::FontMetrics::kUnderlinePositionIsValid_Flag) &&
            (fm.fFlags & SkPaint::FontMetrics::kUnderlinePositionIsValid_Flag))
        {
            SkRect underline{ fontBounds.fLeft,  fm.fUnderlinePosition+y,
                              fontBounds.fRight, fm.fUnderlinePosition+y + fm.fUnderlineThickness };
            canvas->drawRect(underline, metricsPaint);
        }

        if ((fm.fFlags & SkPaint::FontMetrics::kStrikeoutPositionIsValid_Flag) &&
            (fm.fFlags & SkPaint::FontMetrics::kStrikeoutPositionIsValid_Flag))
        {
            SkRect strikeout{ fontBounds.fLeft,  fm.fStrikeoutPosition+y - fm.fStrikeoutThickness,
                              fontBounds.fRight, fm.fStrikeoutPosition+y };
            canvas->drawRect(strikeout, metricsPaint);
        }

        SkGlyphID left = 0, right = 0, top = 0, bottom = 0;
        {
            int numGlyphs = glyphPaint.getTypeface()->countGlyphs();
            SkRect min = {0, 0, 0, 0};
            glyphPaint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
            for (int i = 0; i < numGlyphs; ++i) {
                SkGlyphID glyphId = i;
                SkRect cur;
                glyphPaint.measureText(&glyphId, sizeof(glyphId), &cur);
                if (cur.fLeft   < min.fLeft  ) { min.fLeft   = cur.fLeft;   left   = i; }
                if (cur.fTop    < min.fTop   ) { min.fTop    = cur.fTop ;   top    = i; }
                if (min.fRight  < cur.fRight ) { min.fRight  = cur.fRight;  right  = i; }
                if (min.fBottom < cur.fBottom) { min.fBottom = cur.fBottom; bottom = i; }
            }
        }
        SkGlyphID str[] = { left, right, top, bottom };
        for (size_t i = 0; i < SK_ARRAY_COUNT(str); ++i) {
            SkPath path;
            glyphPaint.getTextPath(&str[i], sizeof(str[0]), x, y, &path);
            SkPaint::Style style = path.isEmpty() ? SkPaint::kFill_Style : SkPaint::kStroke_Style;
            glyphPaint.setStyle(style);
            canvas->drawText(&str[i], sizeof(str[0]), x, y, glyphPaint);
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
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setSubpixelText(true);
        paint.setTextSize(100);
        paint.setTextScaleX(fScaleX);
        paint.setTextSkewX(fSkewX);

        const SkColor boundsColors[2] = { SK_ColorRED, SK_ColorBLUE };

        SkFontMgr* fm = fFM.get();
        int count = SkMin32(fm->countFamilies(), 32);

        int index = 0;
        SkScalar x = 0, y = 0;

        canvas->translate(10, 120);

        for (int i = 0; i < count; ++i) {
            sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
            for (int j = 0; j < set->count() && j < 3; ++j) {
                paint.setTypeface(sk_sp<SkTypeface>(set->createTypeface(j)));
                // Fonts with lots of glyphs are interesting, but can take a long time to find
                // the glyphs which make up the maximum extent.
                if (paint.getTypeface() && paint.getTypeface()->countGlyphs() < 1000) {
                    SkRect fontBounds = paint.getFontBounds();
                    x -= fontBounds.fLeft;
                    show_bounds(canvas, paint, x, y, boundsColors[index & 1]);
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

DEF_SIMPLE_GM(monospade, canvas, 200, 200) {
	SkPaint p;
	p.setAntiAlias(true);
	p.setTextSize(48);
	p.setTypeface(SkTypeface::MakeFromName("monospace", SkFontStyle()));
	canvas->drawString("♠️♠️", 50, 50, p);
}

#include <set>
#include <string>

const char* names[] = {
	"DejaVu Math TeX Gyre",
	"DejaVu Sans",
	"DejaVu Sans Condensed",
	"DejaVu Sans Light",
	"DejaVu Sans Mono",
	"DejaVu Serif",
	"DejaVu Serif Condensed",
	"MathJax_AMS",
	"MathJax_Caligraphic",
	"MathJax_Fraktur",
	"MathJax_Main",
	"MathJax_Math",
	"MathJax_SansSerif",
	"MathJax_Script",
	"MathJax_Size1",
	"MathJax_Size2",
	"MathJax_Size3",
	"MathJax_Size4",
	"MathJax_Typewriter",
	"MathJax_Vector",
	"MathJax_WinChrome",
	"MathJax_WinIE6",
	"Noto Kufi Arabic",
	"Noto Mono",
	"Noto Naskh Arabic",
	"Noto Naskh Arabic UI",
	"Noto Nastaliq Urdu",
	"Noto Sans",
	"Noto Sans Adlam",
	"Noto Sans Adlam Unjoined",
	"Noto Sans AnatoHiero",
	"Noto Sans Anatolian Hieroglyphs",
	"Noto Sans Arabic",
	"Noto Sans Arabic UI",
	"Noto Sans Armenian",
	"Noto Sans Avestan",
	"Noto Sans Balinese",
	"Noto Sans Bamum",
	"Noto Sans Batak",
	"Noto Sans Bengali",
	"Noto Sans Bengali UI",
	"Noto Sans Brahmi",
	"Noto Sans Buginese",
	"Noto Sans Buhid",
	"Noto Sans CJK JP",
	"Noto Sans CJK JP Black",
	"Noto Sans CJK JP Bold",
	"Noto Sans CJK JP DemiLight",
	"Noto Sans CJK JP Light",
	"Noto Sans CJK JP Medium",
	"Noto Sans CJK JP Regular",
	"Noto Sans CJK JP Thin",
	"Noto Sans CJK KR",
	"Noto Sans CJK KR Black",
	"Noto Sans CJK KR Bold",
	"Noto Sans CJK KR DemiLight",
	"Noto Sans CJK KR Light",
	"Noto Sans CJK KR Medium",
	"Noto Sans CJK KR Regular",
	"Noto Sans CJK KR Thin",
	"Noto Sans CJK SC",
	"Noto Sans CJK SC Black",
	"Noto Sans CJK SC Bold",
	"Noto Sans CJK SC DemiLight",
	"Noto Sans CJK SC Light",
	"Noto Sans CJK SC Medium",
	"Noto Sans CJK SC Regular",
	"Noto Sans CJK SC Thin",
	"Noto Sans CJK TC",
	"Noto Sans CJK TC Black",
	"Noto Sans CJK TC Bold",
	"Noto Sans CJK TC DemiLight",
	"Noto Sans CJK TC Light",
	"Noto Sans CJK TC Medium",
	"Noto Sans CJK TC Regular",
	"Noto Sans CJK TC Thin",
	"Noto Sans Canadian Aboriginal",
	"Noto Sans Carian",
	"Noto Sans Chakma",
	"Noto Sans Cham",
	"Noto Sans Cherokee",
	"Noto Sans Coptic",
	"Noto Sans Cuneiform",
	"Noto Sans Cypriot",
	"Noto Sans Deseret",
	"Noto Sans Devanagari",
	"Noto Sans Devanagari UI",
	"Noto Sans Disp",
	"Noto Sans Display",
	"Noto Sans EgyptHiero",
	"Noto Sans Egyptian Hieroglyphs",
	"Noto Sans Ethiopic",
	"Noto Sans Georgian",
	"Noto Sans Glagolitic",
	"Noto Sans Gothic",
	"Noto Sans Gujarati",
	"Noto Sans Gujarati UI",
	"Noto Sans Gurmukhi",
	"Noto Sans Gurmukhi UI",
	"Noto Sans Hanunoo",
	"Noto Sans Hebrew",
	"Noto Sans ImpAramaic",
	"Noto Sans Imperial Aramaic",
	"Noto Sans InsPahlavi",
	"Noto Sans InsParthi",
	"Noto Sans Inscriptional Pahlavi",
	"Noto Sans Inscriptional Parthian",
	"Noto Sans Javanese",
	"Noto Sans Kaithi",
	"Noto Sans Kannada",
	"Noto Sans Kannada UI",
	"Noto Sans Kayah Li",
	"Noto Sans Kharoshthi",
	"Noto Sans Khmer",
	"Noto Sans Khmer UI",
	"Noto Sans Lao",
	"Noto Sans Lao UI",
	"Noto Sans Lepcha",
	"Noto Sans Limbu",
	"Noto Sans Linear B",
	"Noto Sans Lisu",
	"Noto Sans Lycian",
	"Noto Sans Lydian",
	"Noto Sans Malayalam",
	"Noto Sans Malayalam UI",
	"Noto Sans Mandaic",
	"Noto Sans Meetei Mayek",
	"Noto Sans Mongolian",
	"Noto Sans Mono",
	"Noto Sans Mono CJK JP",
	"Noto Sans Mono CJK JP Bold",
	"Noto Sans Mono CJK JP Regular",
	"Noto Sans Mono CJK KR",
	"Noto Sans Mono CJK KR Bold",
	"Noto Sans Mono CJK KR Regular",
	"Noto Sans Mono CJK SC",
	"Noto Sans Mono CJK SC Bold",
	"Noto Sans Mono CJK SC Regular",
	"Noto Sans Mono CJK TC",
	"Noto Sans Mono CJK TC Bold",
	"Noto Sans Mono CJK TC Regular",
	"Noto Sans Myanmar",
	"Noto Sans Myanmar UI",
	"Noto Sans N'Ko",
	"Noto Sans NKo",
	"Noto Sans New Tai Lue",
	"Noto Sans Ogham",
	"Noto Sans Ol Chiki",
	"Noto Sans Old Italic",
	"Noto Sans Old Persian",
	"Noto Sans Old South Arabian",
	"Noto Sans Old Turkic",
	"Noto Sans OldSouArab",
	"Noto Sans Oriya",
	"Noto Sans Oriya UI",
	"Noto Sans Osage",
	"Noto Sans Osmanya",
	"Noto Sans Phags Pa",
	"Noto Sans Phoenician",
	"Noto Sans Rejang",
	"Noto Sans Runic",
	"Noto Sans Samaritan",
	"Noto Sans Saurashtra",
	"Noto Sans Shavian",
	"Noto Sans Sinhala",
	"Noto Sans Sinhala UI",
	"Noto Sans Sundanese",
	"Noto Sans Syloti Nagri",
	"Noto Sans Symbols",
	"Noto Sans Symbols2",
	"Noto Sans Syriac Eastern",
	"Noto Sans Syriac Estrangela",
	"Noto Sans Syriac Western",
	"Noto Sans Tagalog",
	"Noto Sans Tagbanwa",
	"Noto Sans Tai Le",
	"Noto Sans Tai Tham",
	"Noto Sans Tai Viet",
	"Noto Sans Tamil",
	"Noto Sans Tamil UI",
	"Noto Sans Telugu",
	"Noto Sans Telugu UI",
	"Noto Sans Thaana",
	"Noto Sans Thai",
	"Noto Sans Thai UI",
	"Noto Sans Tibetan",
	"Noto Sans Tifinagh",
	"Noto Sans Ugaritic",
	"Noto Sans Vai",
	"Noto Sans Yi",
	"Noto Serif",
	"Noto Serif Armenian",
	"Noto Serif Bengali",
	"Noto Serif CJK JP",
	"Noto Serif CJK JP Black",
	"Noto Serif CJK JP ExtraLight",
	"Noto Serif CJK JP Light",
	"Noto Serif CJK JP Medium",
	"Noto Serif CJK JP SemiBold",
	"Noto Serif CJK KR",
	"Noto Serif CJK KR Black",
	"Noto Serif CJK KR ExtraLight",
	"Noto Serif CJK KR Light",
	"Noto Serif CJK KR Medium",
	"Noto Serif CJK KR SemiBold",
	"Noto Serif CJK SC",
	"Noto Serif CJK SC Black",
	"Noto Serif CJK SC ExtraLight",
	"Noto Serif CJK SC Light",
	"Noto Serif CJK SC Medium",
	"Noto Serif CJK SC SemiBold",
	"Noto Serif CJK TC",
	"Noto Serif CJK TC Black",
	"Noto Serif CJK TC ExtraLight",
	"Noto Serif CJK TC Light",
	"Noto Serif CJK TC Medium",
	"Noto Serif CJK TC SemiBold",
	"Noto Serif Devanagari",
	"Noto Serif Disp",
	"Noto Serif Display",
	"Noto Serif Ethiopic",
	"Noto Serif Georgian",
	"Noto Serif Gujarati",
	"Noto Serif Hebrew",
	"Noto Serif Kannada",
	"Noto Serif Khmer",
	"Noto Serif Lao",
	"Noto Serif Malayalam",
	"Noto Serif Myanmar",
	"Noto Serif Sinhala",
	"Noto Serif Tamil",
	"Noto Serif Telugu",
	"Noto Serif Thai",
	"Roboto",
	"Roboto Condensed",
};

DEF_SIMPLE_GM(uniquenames, canvas, 1, 1) {
	SkPaint p;
	p.setAntiAlias(true);
	p.setTextSize(18);
	for (auto name : names) {
		sk_sp<SkTypeface> face(SkTypeface::MakeFromName(name, SkFontStyle()));
		if (!face) {
			continue;
		}
		SkGlyphID glyphs[2];
		int result = face->charsToGlyphs("♠️", SkTypeface::kUTF8_Encoding, glyphs, 2);
		if (!result) {
			continue;
		}
		p.setTypeface(face);
		canvas->drawString(name, 5, 20, p);
		canvas->drawString("♠️", 200, 20, p);
		canvas->translate(0, 20);
	}
}
