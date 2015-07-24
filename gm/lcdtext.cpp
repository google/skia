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
#include "SkPicture.h"
#include "SkPictureImageFilter.h"
#include "SkPictureRecorder.h"
#include "SkSurface.h"


class LcdTextGM : public skiagm::GM {
public:
    LcdTextGM() {
        const int pointSize = 36;
        textHeight = SkIntToScalar(pointSize);
    }
    
protected:
    
    SkString onShortName() {
        SkString name("lcdtext");
        name.append(sk_tool_utils::major_platform_os_name());
        return name;
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

DEF_GM( return new LcdTextGM; )
DEF_GM( return new LcdTextSizeGM; )
