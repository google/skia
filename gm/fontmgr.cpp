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
#include "SkGraphics.h"
#include "SkTypeface.h"

#ifdef SK_BUILD_FOR_WIN
    #include "SkTypeface_win.h"
#endif

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
    paint.setTypeface(sk_sp<SkTypeface>(fm->legacyCreateTypeface(familyName.c_str(),
                                                                 typeface->fontStyle())));
    return drawString(canvas, ch, x, y, paint) + 20;
}

static const char* zh = "zh";
static const char* ja = "ja";

class FontMgrGM : public skiagm::GM {
public:
    FontMgrGM(sk_sp<SkFontMgr> fontMgr = nullptr) {
        SkGraphics::SetFontCacheLimit(16 * 1024 * 1024);

        fName.set("fontmgr_iter");
        if (fontMgr) {
            fName.append("_factory");
            fFM = std::move(fontMgr);
        } else {
            fFM = SkFontMgr::RefDefault();
        }
        fName.append(sk_tool_utils::platform_os_name());
        fName.append(sk_tool_utils::platform_extra_config("GDI"));
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
        name.append(sk_tool_utils::platform_os_name());
        name.append(sk_tool_utils::platform_extra_config("GDI"));
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
        fName.append(sk_tool_utils::platform_os_name());
        fName.append(sk_tool_utils::platform_extra_config("GDI"));
        fFM = SkFontMgr::RefDefault();
    }

    static void show_bounds(SkCanvas* canvas, const SkPaint& paint, SkScalar x, SkScalar y,
                            SkColor boundsColor) {
        const char str[] = "jyHO[]{}@-_&%$";

        for (int i = 0; str[i]; ++i) {
            canvas->drawText(&str[i], 1, x, y, paint);
        }

        SkRect r = paint.getFontBounds();
        r.offset(x, y);
        SkPaint p(paint);
        p.setColor(boundsColor);
        canvas->drawRect(r, p);
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
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setTextScaleX(fScaleX);
        paint.setTextSkewX(fSkewX);

        const SkColor boundsColors[2] = { SK_ColorRED, SK_ColorBLUE };

        SkFontMgr* fm = fFM.get();
        int count = SkMin32(fm->countFamilies(), 32);

        int index = 0;
        SkScalar x = 0, y = 0;

        canvas->translate(80, 120);

        for (int i = 0; i < count; ++i) {
            sk_sp<SkFontStyleSet> set(fm->createStyleSet(i));
            for (int j = 0; j < set->count(); ++j) {
                paint.setTypeface(sk_sp<SkTypeface>(set->createTypeface(j)));
                if (paint.getTypeface()) {
                    show_bounds(canvas, paint, x, y, boundsColors[index & 1]);
                    index += 1;
                    x += 160;
                    if (0 == (index % 6)) {
                        x = 0;
                        y += 160;
                    }
                    if (index >= 30) {
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

#ifdef SK_BUILD_FOR_WIN
DEF_GM(return new FontMgrGM(SkFontMgr_New_DirectWrite());)
#endif
