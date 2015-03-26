
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"

namespace skiagm {

/** Create a bitmap image suitable for testing SkBitmap::scrollRect().
 *
 *  @param quarterWidth bitmap will be 4x this many pixels wide
 *  @param quarterHeight bitmap will be 4x this many pixels tall
 *  @param bitmap the bitmap data is written into this object
 */
static void make_bitmap(int quarterWidth, int quarterHeight, SkBitmap *bitmap) {
    SkPaint pRed, pWhite, pGreen, pBlue, pLine, pAlphaGray;
    pRed.setColor(0xFFFF9999);
    pWhite.setColor(0xFFFFFFFF);
    pGreen.setColor(0xFF99FF99);
    pBlue.setColor(0xFF9999FF);
    pLine.setColor(0xFF000000);
    pLine.setStyle(SkPaint::kStroke_Style);
    pAlphaGray.setColor(0x66888888);

    // Prepare bitmap, and a canvas that draws into it.
    bitmap->allocN32Pixels(quarterWidth*4, quarterHeight*4);
    SkCanvas canvas(*bitmap);

    SkScalar w = SkIntToScalar(quarterWidth);
    SkScalar h = SkIntToScalar(quarterHeight);
    canvas.drawRectCoords(  0,   0, w*2, h*2, pRed);
    canvas.drawRectCoords(w*2,   0, w*4, h*2, pGreen);
    canvas.drawRectCoords(  0, h*2, w*2, h*4, pBlue);
    canvas.drawRectCoords(w*2, h*2, w*4, h*4, pWhite);
    canvas.drawRectCoords(w, h, w*3, h*3, pAlphaGray);
    canvas.drawLine(w*2,   0, w*2, h*4, pLine);
    canvas.drawLine(  0, h*2, w*4, h*2, pLine);
    canvas.drawRectCoords(w, h, w*3, h*3, pLine);
}

class BitmapScrollGM : public GM {
    bool fInited;
    void init() {
        if (fInited) {
            return;
        }
        fInited = true;
        // Create the original bitmap.
        make_bitmap(quarterWidth, quarterHeight, &origBitmap);
    }

public:
    BitmapScrollGM() {
        fInited = false;
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString onShortName() override {
        return SkString("bitmapscroll");
    }

    SkISize onISize() override {
      return SkISize::Make(800, 600);
    }

    void onDraw(SkCanvas* canvas) override {
        this->init();
        SkIRect scrollCenterRegion = SkIRect::MakeXYWH(
            quarterWidth, quarterHeight, quarterWidth*2+1, quarterHeight*2+1);
        int x = quarterWidth;
        int y = quarterHeight;
        int xSpacing = quarterWidth * 20;
        int ySpacing = quarterHeight * 16;

        // Draw left-hand text labels.
        drawLabel(canvas, "scroll entire bitmap",
                  x, y, x, y + ySpacing);
        drawLabel(canvas, "scroll part of bitmap",
                  x, y + ySpacing, x, y + ySpacing*2);
        x += 30;

        // Draw various permutations of scrolled bitmaps, scrolling a bit
        // further each time.
        draw9(canvas, x, y, NULL, quarterWidth*1/2, quarterHeight*1/2);
        draw9(canvas, x, y+ySpacing, &scrollCenterRegion,
              quarterWidth*1/2, quarterHeight*1/2);
        x += xSpacing;
        draw9(canvas, x, y, NULL, quarterWidth*3/2, quarterHeight*3/2);
        draw9(canvas, x, y+ySpacing, &scrollCenterRegion,
              quarterWidth*3/2, quarterHeight*3/2);
        x += xSpacing;
        draw9(canvas, x, y, NULL, quarterWidth*5/2, quarterHeight*5/2);
        draw9(canvas, x, y+ySpacing, &scrollCenterRegion,
              quarterWidth*5/2, quarterHeight*5/2);
        x += xSpacing;
        draw9(canvas, x, y, NULL, quarterWidth*9/2, quarterHeight*9/2);
        draw9(canvas, x, y+ySpacing, &scrollCenterRegion,
              quarterWidth*9/2, quarterHeight*9/2);
    }

    void drawLabel(SkCanvas* canvas, const char *text, int startX, int startY,
                 int endX, int endY) {
        SkPaint paint;
        sk_tool_utils::set_portable_typeface(&paint);
        paint.setColor(0xFF000000);
        SkPath path;
        path.moveTo(SkIntToScalar(startX), SkIntToScalar(startY));
        path.lineTo(SkIntToScalar(endX), SkIntToScalar(endY));
        canvas->drawTextOnPath(text, strlen(text), path, NULL, paint);
    }

    /** Stamp out 9 copies of origBitmap, scrolled in each direction (and
     *  not scrolled at all).
     */
    void draw9(SkCanvas* canvas, int x, int y, SkIRect* subset,
               int scrollX, int scrollY) {
        for (int yMult=-1; yMult<=1; yMult++) {
            for (int xMult=-1; xMult<=1; xMult++) {
                // Figure out the (x,y) to draw this copy at
                SkScalar bitmapX = SkIntToScalar(
                    x + quarterWidth * 5 * (xMult+1));
                SkScalar bitmapY = SkIntToScalar(
                    y + quarterHeight * 5 * (yMult+1));

                // Scroll a new copy of the bitmap, and then draw it.
                // scrollRect() should always return true, even if it's a no-op
                SkBitmap scrolledBitmap;
                SkDEBUGCODE(bool copyToReturnValue = )origBitmap.copyTo(
                    &scrolledBitmap, origBitmap.colorType());
                SkASSERT(copyToReturnValue);
                SkDEBUGCODE(bool scrollRectReturnValue = )scrolledBitmap.scrollRect(
                    subset, scrollX * xMult, scrollY * yMult);
                SkASSERT(scrollRectReturnValue);
                canvas->drawBitmap(scrolledBitmap, bitmapX, bitmapY);
            }
        }
    }

private:
    typedef GM INHERITED;
    static const int quarterWidth = 10;
    static const int quarterHeight = 14;
    SkBitmap origBitmap;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new BitmapScrollGM; }
static GMRegistry reg(MyFactory);

}
