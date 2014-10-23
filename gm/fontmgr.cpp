/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkFontMgr.h"
#include "SkGraphics.h"
#include "SkTypeface.h"

#ifdef SK_BUILD_FOR_WIN
    #include "SkTypeface_win.h"
#endif

static void scale(SkRect* rect, SkScalar scale) {
    rect->fLeft *= scale;
    rect->fTop *= scale;
    rect->fRight *= scale;
    rect->fBottom *= scale;
}

// limit this just so we don't take too long to draw
#define MAX_FAMILIES    30

static SkScalar drawString(SkCanvas* canvas, const SkString& text, SkScalar x,
                           SkScalar y, const SkPaint& paint) {
    canvas->drawText(text.c_str(), text.size(), x, y, paint);
    return x + paint.measureText(text.c_str(), text.size());
}

static SkScalar drawCharacter(SkCanvas* canvas, uint32_t character, SkScalar x,
                              SkScalar y, SkPaint& paint, SkFontMgr* fm,
                              const char* fontName, const char* bpc47,
                              const SkFontStyle& fontStyle) {
    // find typeface containing the requested character and draw it
    SkString ch;
    ch.appendUnichar(character);
#ifdef SK_FM_NEW_MATCH_FAMILY_STYLE_CHARACTER
    SkTypeface* typeface = fm->matchFamilyStyleCharacter(fontName, fontStyle, &bpc47, 1, character);
#else
    SkTypeface* typeface = fm->matchFamilyStyleCharacter(fontName, fontStyle, bpc47, character);
#endif
    SkSafeUnref(paint.setTypeface(typeface));
    x = drawString(canvas, ch, x, y, paint) + 20;

    if (NULL == typeface) {
        return x;
    }

    // repeat the process, but this time use the family name of the typeface
    // from the first pass.  This emulates the behavior in Blink where it
    // it expects to get the same glyph when following this pattern.
    SkString familyName;
    typeface->getFamilyName(&familyName);
    SkTypeface* typefaceCopy = fm->legacyCreateTypeface(familyName.c_str(), typeface->style());
    SkSafeUnref(paint.setTypeface(typefaceCopy));
    return drawString(canvas, ch, x, y, paint) + 20;
}

class FontMgrGM : public skiagm::GM {
public:
    FontMgrGM(SkFontMgr* fontMgr = NULL) {
        SkGraphics::SetFontCacheLimit(16 * 1024 * 1024);

        fName.set("fontmgr_iter");
        if (fontMgr) {
            fName.append("_factory");
            fFM.reset(fontMgr);
        } else {
            fFM.reset(SkFontMgr::RefDefault());
        }
    }

protected:
    virtual SkString onShortName() {
        return fName;
    }

    virtual SkISize onISize() {
        return SkISize::Make(1536, 768);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkScalar y = 20;
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setSubpixelText(true);
        paint.setTextSize(17);

        SkFontMgr* fm = fFM;
        int count = SkMin32(fm->countFamilies(), MAX_FAMILIES);

        for (int i = 0; i < count; ++i) {
            SkString fname;
            fm->getFamilyName(i, &fname);
            paint.setTypeface(NULL);
            (void)drawString(canvas, fname, 20, y, paint);

            SkScalar x = 220;

            SkAutoTUnref<SkFontStyleSet> set(fm->createStyleSet(i));
            for (int j = 0; j < set->count(); ++j) {
                SkString sname;
                SkFontStyle fs;
                set->getStyle(j, &fs, &sname);
                sname.appendf(" [%d %d %d]", fs.weight(), fs.width(), fs.isItalic());

                SkSafeUnref(paint.setTypeface(set->createTypeface(j)));
                x = drawString(canvas, sname, x, y, paint) + 20;

                // check to see that we get different glyphs in japanese and chinese
                x = drawCharacter(canvas, 0x5203, x, y, paint, fm, fName.c_str(), "zh", fs);
                x = drawCharacter(canvas, 0x5203, x, y, paint, fm, fName.c_str(), "ja", fs);
                // check that emoji characters are found
                x = drawCharacter(canvas, 0x1f601, x, y, paint, fm, fName.c_str(), NULL, fs);
            }
            y += 24;
        }
    }

    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        // fontdescriptors (and therefore serialization) don't yet understand
        // these new styles, so skip tests that exercise that for now.

        // If certain fonts are picked up (e.g. Microsoft Jhenghei 20MB for Regular, 12MB for Bold),
        // the resulting pdf can be ~700MB and crashes Chrome's PDF viewer.

        return kSkipPicture_Flag | kSkipPipe_Flag | kSkipPDF_Flag;
    }

private:
    SkAutoTUnref<SkFontMgr> fFM;
    SkString fName;
    typedef GM INHERITED;
};

class FontMgrMatchGM : public skiagm::GM {
    SkAutoTUnref<SkFontMgr> fFM;

public:
    FontMgrMatchGM() : fFM(SkFontMgr::RefDefault()) {
        SkGraphics::SetFontCacheLimit(16 * 1024 * 1024);
    }

protected:
    virtual SkString onShortName() {
        return SkString("fontmgr_match");
    }

    virtual SkISize onISize() {
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

            SkSafeUnref(p.setTypeface(fset->createTypeface(j)));
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
                SkTypeface* face = fset->matchStyle(fs);
                if (face) {
                    SkString str;
                    str.printf("request [%d %d]", fs.weight(), fs.width());
                    p.setTypeface(face)->unref();
                    (void)drawString(canvas, str, 0, y, p);
                    y += 24;
                }
            }
        }
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setSubpixelText(true);
        paint.setTextSize(17);

        static const char* gNames[] = {
            "Helvetica Neue", "Arial"
        };

        SkAutoTUnref<SkFontStyleSet> fset;
        for (size_t i = 0; i < SK_ARRAY_COUNT(gNames); ++i) {
            fset.reset(fFM->matchFamily(gNames[i]));
            if (fset->count() > 0) {
                break;
            }
        }
        if (NULL == fset.get()) {
            return;
        }

        canvas->translate(20, 40);
        this->exploreFamily(canvas, paint, fset);
        canvas->translate(150, 0);
        this->iterateFamily(canvas, paint, fset);
    }

    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        // fontdescriptors (and therefore serialization) don't yet understand
        // these new styles, so skip tests that exercise that for now.
        return kSkipPicture_Flag | kSkipPipe_Flag;
    }

private:
    typedef GM INHERITED;
};

class FontMgrBoundsGM : public skiagm::GM {
public:
    FontMgrBoundsGM() {
        fName.set("fontmgr_bounds");
        fFM.reset(SkFontMgr::RefDefault());
    }

    static void show_bounds(SkCanvas* canvas, const SkPaint& paint, SkScalar x, SkScalar y,
                            SkColor boundsColor) {
        const char str[] = "jyHO[]{}@-_&%$";

        const SkTypeface* tf = paint.getTypeface();
        for (int i = 0; str[i]; ++i) {
            canvas->drawText(&str[i], 1, x, y, paint);
        }
        
        SkRect r = tf->getBounds();
        scale(&r, paint.getTextSize());
        r.offset(x, y);
        SkPaint p(paint);
        p.setColor(boundsColor);
        canvas->drawRect(r, p);
    }

protected:
    virtual SkString onShortName() {
        return fName;
    }
    
    virtual SkISize onISize() {
        return SkISize::Make(1024, 850);
    }
    
    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setSubpixelText(true);
        paint.setTextSize(100);
        paint.setStyle(SkPaint::kStroke_Style);

        const SkColor boundsColors[2] = { SK_ColorRED, SK_ColorBLUE };
        
        SkFontMgr* fm = fFM;
        int count = SkMin32(fm->countFamilies(), 32);

        int index = 0;
        SkScalar x = 0, y = 0;

        canvas->translate(80, 120);

        for (int i = 0; i < count; ++i) {
            SkAutoTUnref<SkFontStyleSet> set(fm->createStyleSet(i));
            for (int j = 0; j < set->count(); ++j) {
                SkSafeUnref(paint.setTypeface(set->createTypeface(j)));
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
    
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        // fontdescriptors (and therefore serialization) don't yet understand
        // these new styles, so skip tests that exercise that for now.
        
        // If certain fonts are picked up (e.g. Microsoft Jhenghei 20MB for Regular, 12MB for Bold),
        // the resulting pdf can be ~700MB and crashes Chrome's PDF viewer.
        
        return kSkipPicture_Flag | kSkipPipe_Flag | kSkipPDF_Flag;
    }
    
private:
    SkAutoTUnref<SkFontMgr> fFM;
    SkString fName;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(FontMgrGM); )
DEF_GM( return SkNEW(FontMgrBoundsGM); )
DEF_GM( return SkNEW(FontMgrMatchGM); )

#ifdef SK_BUILD_FOR_WIN
    DEF_GM( return SkNEW_ARGS(FontMgrGM, (SkFontMgr_New_DirectWrite())); )
#endif
