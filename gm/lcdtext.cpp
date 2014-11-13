/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


/* Tests text rendering with LCD and subpixel rendering turned on and off.
 */

#include "gm.h"
#include "SkCanvas.h"

class LcdTextGM : public skiagm::GM {
public:
    LcdTextGM() {
        const int pointSize = 36;
        textHeight = SkIntToScalar(pointSize);
    }
    
protected:
    
    SkString onShortName() {
        return SkString("lcdtext");
    }
    
    SkISize onISize() { return SkISize::Make(640, 480); }
    
    virtual void onDraw(SkCanvas* canvas) {
        
        y = textHeight;
        drawText(canvas, SkString("TEXT: SubpixelTrue LCDRenderTrue"),
                 true,  true);
        drawText(canvas, SkString("TEXT: SubpixelTrue LCDRenderFalse"),
                 true,  false);
        drawText(canvas, SkString("TEXT: SubpixelFalse LCDRenderTrue"),
                 false, true);
        drawText(canvas, SkString("TEXT: SubpixelFalse LCDRenderFalse"),
                 false, false);
    }
    
    void drawText(SkCanvas* canvas, const SkString& string,
                  bool subpixelTextEnabled, bool lcdRenderTextEnabled) {
        SkPaint paint;
        paint.setColor(SK_ColorBLACK);
        paint.setDither(true);
        paint.setAntiAlias(true);
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setSubpixelText(subpixelTextEnabled);
        paint.setLCDRenderText(lcdRenderTextEnabled);
        paint.setTextSize(textHeight);
        
        canvas->drawText(string.c_str(), string.size(), 0, y, paint);
        y += textHeight;
    }
    
private:
    typedef skiagm::GM INHERITED;
    SkScalar y, textHeight;
};

/*
 *  Skia will automatically disable LCD requests if the total size exceeds some limit
 *  (hard coded in this test for now, as it is now avaiable as an API)
 *
 *  Test this both by changing "textsize" and by changing the computed size (textsize * CTM)
 */
class LcdTextSizeGM : public skiagm::GM {
    enum {
        kLCDTextSizeLimit = 48
    };

    static void ScaleAbout(SkCanvas* canvas, SkScalar sx, SkScalar sy, SkScalar px, SkScalar py) {
        SkMatrix m;
        m.setScale(sx, sy, px, py);
        canvas->concat(m);
    }

public:
    LcdTextSizeGM() {}
    
protected:
    SkString onShortName() {
        return SkString("lcdtextsize");
    }
    
    SkISize onISize() { return SkISize::Make(320, 120); }
    
    virtual void onDraw(SkCanvas* canvas) {
        const char* lcd_text = "LCD";
        const char* gray_text = "GRAY";

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);

        const struct {
            SkPoint     fLoc;
            SkScalar    fTextSize;
            SkScalar    fScale;
            const char* fText;
        } rec[] = {
            { {  10,  50 }, kLCDTextSizeLimit - 1,     1,  lcd_text },
            { { 160,  50 }, kLCDTextSizeLimit + 1,     1,  gray_text },
            { {  10, 100 }, kLCDTextSizeLimit / 2, 1.99f,  lcd_text },
            { { 160, 100 }, kLCDTextSizeLimit / 2, 2.01f,  gray_text },
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(rec); ++i) {
            const SkPoint loc = rec[i].fLoc;
            SkAutoCanvasRestore acr(canvas, true);

            paint.setTextSize(rec[i].fTextSize);
            ScaleAbout(canvas, rec[i].fScale, rec[i].fScale, loc.x(), loc.y());
            canvas->drawText(rec[i].fText, strlen(rec[i].fText), loc.x(), loc.y(), paint);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

#include "SkSurface.h"

// ensure that we respect the SkPixelGeometry in SurfaceProps
class LcdTextProps : public skiagm::GM {
    static void DrawText(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setLCDRenderText(true);
        paint.setTextSize(30);
        canvas->drawText("Base", 4, 4, 30, paint);
        canvas->saveLayer(NULL, NULL);
        canvas->drawText("Layer", 5, 4, 70, paint);
        canvas->restore();
    }

public:
    SkString onShortName() SK_OVERRIDE {
        return SkString("lcdtextprops");
    }

    SkISize onISize() SK_OVERRIDE { return SkISize::Make(230, 120); }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        const SkPixelGeometry geos[] = {
            kRGB_H_SkPixelGeometry,
            kUnknown_SkPixelGeometry,
        };

        const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
        for (size_t i = 0; i < SK_ARRAY_COUNT(geos); ++i) {
            SkSurfaceProps props = SkSurfaceProps(0, geos[i]);
            SkAutoTUnref<SkSurface> surf(canvas->newSurface(info, &props));
            if (!surf) {
                surf.reset(SkSurface::NewRaster(info, &props));
            }
            DrawText(surf->getCanvas());
            surf->draw(canvas, SkIntToScalar(i * (info.width() + 10)), 0, NULL);
        }
    }
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new LcdTextGM; )
DEF_GM( return new LcdTextSizeGM; )
DEF_GM( return new LcdTextProps; )
