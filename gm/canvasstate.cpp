/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRect.h"

namespace skiagm {

/*
 * This GM exercises the flags to SkCanvas::save(). The canvas' save() and
 * restore actions can be limited to only a portion of the canvas' state through
 * the use of flags when calling save.
 */
class CanvasStateGM : public GM {
    SkSize  fSize;
    enum {
        WIDTH = 150,
        HEIGHT = 150,
    };

    SkPaint fFillPaint;
    SkPaint fStrokePaint;

    SkPath fPath;

    SkRect fOutlineRect;
    SkRect fFillRect;


public:
    CanvasStateGM() {
        fSize.set(SkIntToScalar(WIDTH), SkIntToScalar(HEIGHT));

        fFillPaint.setColor(SK_ColorRED);
        fFillPaint.setStyle(SkPaint::kFill_Style);

        fStrokePaint.setColor(SK_ColorBLUE);
        fStrokePaint.setStyle(SkPaint::kStroke_Style);
        fStrokePaint.setStrokeWidth(1);

        fPath.moveTo(25, 25);
        fPath.lineTo(125, 25);
        fPath.lineTo(75, 125);
        fPath.close();

        fOutlineRect = SkRect::MakeXYWH(1, 1, WIDTH-2, HEIGHT-2);
        fFillRect = SkRect::MakeXYWH(10, 10, WIDTH-20, HEIGHT-20);
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("canvas-state");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(WIDTH*3, HEIGHT*4);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        SkCanvas::SaveFlags flags[] = { SkCanvas::kMatrix_SaveFlag,
                                        SkCanvas::kClip_SaveFlag,
                                        SkCanvas::kMatrixClip_SaveFlag };

        // columns -- flags
        // rows -- permutations of setting the clip and matrix
        for (int i = 0; i < static_cast<int>(SK_ARRAY_COUNT(flags)); ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    this->drawTestPattern(i, (2*j)+k, canvas, flags[i],
                                          SkToBool(j), SkToBool(k));
                }
            }
        }
    }


    virtual uint32_t onGetFlags() const SK_OVERRIDE { return kSkipPicture_Flag; }

private:
    void drawTestPattern(int x, int y, SkCanvas* canvas,
                         SkCanvas::SaveFlags flags, bool doClip, bool doScale) {
        canvas->save();
        canvas->translate(SkIntToScalar(x*WIDTH), SkIntToScalar(y*HEIGHT));

        canvas->drawRect(fOutlineRect, fStrokePaint);
        canvas->save(flags);
        if(doClip) {
            canvas->clipPath(fPath);
        }
        if (doScale) {
            canvas->scale(SkDoubleToScalar(0.5), SkDoubleToScalar(0.5));
        }
        canvas->restore();
        canvas->drawRect(fFillRect, fFillPaint);

        canvas->restore();
    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

class CanvasLayerStateGM : public GM {
public:
    CanvasLayerStateGM() {
        fBluePaint.setColor(SK_ColorBLUE);
        fBluePaint.setStyle(SkPaint::kFill_Style);

        fRect = SkRect::MakeXYWH(SPACER, SPACER, WIDTH-(2*SPACER), (HEIGHT-(2*SPACER)) / 7);
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("canvas-layer-state");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(WIDTH, HEIGHT);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        // clear the canvas to red
        canvas->drawColor(SK_ColorRED);

        // both rects should appear
        drawTestPattern(canvas, 255, SkCanvas::kARGB_NoClipLayer_SaveFlag);

        canvas->translate(0, 2*(fRect.height() + 10));

        // only the top rect should appear
        drawTestPattern(canvas, 255, SkCanvas::kARGB_ClipLayer_SaveFlag);

        canvas->translate(0, 2*(fRect.height() + 10));

        // only the bottom rect should appear
        drawTestPattern(canvas, 0, SkCanvas::kARGB_NoClipLayer_SaveFlag);
    }

    virtual uint32_t onGetFlags() const SK_OVERRIDE { return kSkipGPU_Flag; }

private:
    // draw a rect within the layer's bounds and again outside the layer's bounds
    void drawTestPattern(SkCanvas* canvas, U8CPU layerAlpha, SkCanvas::SaveFlags flags) {
        canvas->saveLayerAlpha(&fRect, layerAlpha, flags);
        canvas->drawRect(fRect, fBluePaint);
        canvas->translate(0, fRect.height() + 10);
        canvas->drawRect(fRect, fBluePaint);
        canvas->restore();
    }

    enum {
        WIDTH = 400,
        HEIGHT = 400,
        SPACER = 10,
    };

    SkPaint fBluePaint;
    SkRect fRect;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return SkNEW(CanvasStateGM); )
DEF_GM( return SkNEW(CanvasLayerStateGM); )

} // end namespace
