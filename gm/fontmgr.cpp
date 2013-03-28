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

// limit this just so we don't take too long to draw
#define MAX_FAMILIES    30

static SkScalar drawString(SkCanvas* canvas, const SkString& text, SkScalar x,
                           SkScalar y, const SkPaint& paint) {
    canvas->drawText(text.c_str(), text.size(), x, y, paint);
    return x + paint.measureText(text.c_str(), text.size());
}

class FontMgrGM : public skiagm::GM {
public:
    FontMgrGM() {
        SkGraphics::SetFontCacheLimit(16 * 1024 * 1024);
    }

protected:
    virtual SkString onShortName() {
        return SkString("fontmgr");
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
        
        SkAutoTUnref<SkFontMgr> fm(SkFontMgr::RefDefault());
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
                
                SkSafeUnref(paint.setTypeface(set->createTypeface(j)));
                x = drawString(canvas, sname, x, y, paint) + 20;
            }            
            y += 24;
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(FontMgrGM); )

