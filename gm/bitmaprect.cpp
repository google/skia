
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"

static void make_bitmap(SkBitmap* bitmap) {
    bitmap->setConfig(SkBitmap::kARGB_8888_Config, 64, 64);
    bitmap->allocPixels();

    SkCanvas canvas(*bitmap);

    canvas.drawColor(SK_ColorRED);
    SkPaint paint;
    paint.setAntiAlias(true);
    const SkPoint pts[] = { { 0, 0 }, { 64, 64 } };
    const SkColor colors[] = { SK_ColorWHITE, SK_ColorBLUE };
    paint.setShader(SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                   SkShader::kClamp_TileMode))->unref();
    canvas.drawCircle(32, 32, 32, paint);
}

class DrawBitmapRect2 : public skiagm::GM {
    bool fUseIRect;
public:
    DrawBitmapRect2(bool useIRect) : fUseIRect(useIRect) {
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        SkString str;
        str.printf("bitmaprect_%s", fUseIRect ? "i" : "s");
        return str;
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        canvas->drawColor(0xFFCCCCCC);

        const SkIRect src[] = {
            { 0, 0, 32, 32 },
            { 0, 0, 80, 80 },
            { 32, 32, 96, 96 },
            { -32, -32, 32, 32, }
        };

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
//        paint.setColor(SK_ColorGREEN);

        SkBitmap bitmap;
        make_bitmap(&bitmap);

        SkRect dstR = { 0, 200, 128, 380 };

        canvas->translate(16, 40);
        for (size_t i = 0; i < SK_ARRAY_COUNT(src); i++) {
            SkRect srcR;
            srcR.set(src[i]);

            canvas->drawBitmap(bitmap, 0, 0, &paint);
            if (fUseIRect) {
                canvas->drawBitmapRectToRect(bitmap, &srcR, dstR, &paint);
            } else {
                canvas->drawBitmapRect(bitmap, &src[i], dstR, &paint);
            }

            canvas->drawRect(dstR, paint);
            canvas->drawRect(srcR, paint);

            canvas->translate(160, 0);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory0(void*) { return new DrawBitmapRect2(false); }
static skiagm::GM* MyFactory1(void*) { return new DrawBitmapRect2(true); }

static skiagm::GMRegistry reg0(MyFactory0);
static skiagm::GMRegistry reg1(MyFactory1);
