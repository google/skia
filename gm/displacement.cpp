/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkDisplacementMapEffect.h"
#include "SkBitmapSource.h"

namespace skiagm {

class DisplacementMapGM : public GM {
public:
    DisplacementMapGM() : fInitialized(false) {
        this->setBGColor(0xFF000000);
    }

protected:
    virtual SkString onShortName() {
        return SkString("displacement");
    }

    void make_bitmap() {
        fBitmap.setConfig(SkBitmap::kARGB_8888_Config, 80, 80);
        fBitmap.allocPixels();
        SkDevice device(fBitmap);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0xFF884422);
        paint.setTextSize(SkIntToScalar(96));
        const char* str = "g";
        canvas.drawText(str, strlen(str), SkIntToScalar(15), SkIntToScalar(55), paint);
    }

    void make_checkerboard() {
        fCheckerboard.setConfig(SkBitmap::kARGB_8888_Config, 80, 80);
        fCheckerboard.allocPixels();
        SkDevice device(fCheckerboard);
        SkCanvas canvas(&device);
        canvas.clear(0x00000000);
        SkPaint darkPaint;
        darkPaint.setColor(0xFF804020);
        SkPaint lightPaint;
        lightPaint.setColor(0xFF244484);
        for (int y = 0; y < 80; y += 16) {
          for (int x = 0; x < 80; x += 16) {
            canvas.save();
            canvas.translate(SkIntToScalar(x), SkIntToScalar(y));
            canvas.drawRect(SkRect::MakeXYWH(0, 0, 8, 8), darkPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 0, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(0, 8, 8, 8), lightPaint);
            canvas.drawRect(SkRect::MakeXYWH(8, 8, 8, 8), darkPaint);
            canvas.restore();
          }
        }
    }

    virtual SkISize onISize() {
        return make_isize(500, 200);
    }

    void drawClippedBitmap(SkCanvas* canvas, int x, int y, const SkPaint& paint) {
        canvas->save();
        canvas->clipRect(SkRect::MakeXYWH(SkIntToScalar(x), SkIntToScalar(y),
            SkIntToScalar(fBitmap.width()), SkIntToScalar(fBitmap.height())));
        canvas->drawBitmap(fBitmap, SkIntToScalar(x), SkIntToScalar(y), &paint);
        canvas->restore();
    }

    virtual void onDraw(SkCanvas* canvas) {
        if (!fInitialized) {
            make_bitmap();
            make_checkerboard();
            fInitialized = true;
        }
        canvas->clear(0x00000000);
        SkPaint paint;
        SkAutoTUnref<SkImageFilter> displ(SkNEW_ARGS(SkBitmapSource, (fCheckerboard)));
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kR_ChannelSelectorType,
             SkDisplacementMapEffect::kG_ChannelSelectorType, 0.0f, displ)))->unref();
        drawClippedBitmap(canvas, 0, 0, paint);
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kB_ChannelSelectorType,
             SkDisplacementMapEffect::kA_ChannelSelectorType, 16.0f, displ)))->unref();
        drawClippedBitmap(canvas, 100, 0, paint);
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kR_ChannelSelectorType,
             SkDisplacementMapEffect::kB_ChannelSelectorType, 32.0f, displ)))->unref();
        drawClippedBitmap(canvas, 200, 0, paint);
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kG_ChannelSelectorType,
             SkDisplacementMapEffect::kA_ChannelSelectorType, 48.0f, displ)))->unref();
        drawClippedBitmap(canvas, 300, 0, paint);
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kR_ChannelSelectorType,
             SkDisplacementMapEffect::kA_ChannelSelectorType, 64.0f, displ)))->unref();
        drawClippedBitmap(canvas, 400, 0, paint);

        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kR_ChannelSelectorType,
             SkDisplacementMapEffect::kG_ChannelSelectorType, 40.0f, displ)))->unref();
        drawClippedBitmap(canvas, 0, 100, paint);
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kB_ChannelSelectorType,
             SkDisplacementMapEffect::kA_ChannelSelectorType, 40.0f, displ)))->unref();
        drawClippedBitmap(canvas, 100, 100, paint);
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kR_ChannelSelectorType,
             SkDisplacementMapEffect::kB_ChannelSelectorType, 40.0f, displ)))->unref();
        drawClippedBitmap(canvas, 200, 100, paint);
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kG_ChannelSelectorType,
             SkDisplacementMapEffect::kA_ChannelSelectorType, 40.0f, displ)))->unref();
        drawClippedBitmap(canvas, 300, 100, paint);
        paint.setImageFilter(SkNEW_ARGS(SkDisplacementMapEffect,
            (SkDisplacementMapEffect::kR_ChannelSelectorType,
             SkDisplacementMapEffect::kA_ChannelSelectorType, 40.0f, displ)))->unref();
        drawClippedBitmap(canvas, 400, 100, paint);
    }

private:
    typedef GM INHERITED;
    SkBitmap fBitmap, fCheckerboard;
    bool fInitialized;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new DisplacementMapGM; }
static GMRegistry reg(MyFactory);

}
