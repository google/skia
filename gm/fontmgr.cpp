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

// limit this just so we don't take too long to draw
#define MAX_FAMILIES    30

static SkScalar drawString(SkCanvas* canvas, const SkString& text, SkScalar x,
                           SkScalar y, const SkPaint& paint) {
    canvas->drawText(text.c_str(), text.size(), x, y, paint);
    return x + paint.measureText(text.c_str(), text.size());
}

class FontMgrGM : public skiagm::GM {
public:
    FontMgrGM(SkFontMgr* (*factory)() = NULL) {
        SkGraphics::SetFontCacheLimit(16 * 1024 * 1024);

        fName.set("fontmgr_iter");
        if (factory) {
            fName.append("_factory");
            fFM.reset(factory());
        } else {
            fFM.reset(SkFontMgr::RefDefault());
        }
    }

protected:
    virtual SkString onShortName() {
        return fName;
    }

    virtual SkISize onISize() {
        return SkISize::Make(640, 1024);
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

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(FontMgrGM); )
DEF_GM( return SkNEW(FontMgrMatchGM); )

#ifdef SK_BUILD_FOR_WIN
    DEF_GM( return SkNEW_ARGS(FontMgrGM, (SkFontMgr_New_DirectWrite)); )
#endif
