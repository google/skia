
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkShader.h"
#include "SkColorPriv.h"

// effects
#include "SkGradientShader.h"


namespace skiagm {

static void makebm(SkBitmap* bm, SkBitmap::Config config, int w, int h) {
    bm->setConfig(config, w, h);
    bm->allocPixels();
    bm->eraseColor(0);

    SkCanvas    canvas(*bm);

    SkScalar wScalar = SkIntToScalar(w);
    SkScalar hScalar = SkIntToScalar(h);

    SkPoint     pt = { wScalar / 2, hScalar / 2 };

    SkScalar    radius = 4 * SkMaxScalar(wScalar, hScalar);

    SkColor     colors[] = { SK_ColorRED, SK_ColorYELLOW,
                             SK_ColorGREEN, SK_ColorMAGENTA,
                             SK_ColorBLUE, SK_ColorCYAN,
                             SK_ColorRED};

    SkScalar    pos[] = {0,
                         SK_Scalar1 / 6,
                         2 * SK_Scalar1 / 6,
                         3 * SK_Scalar1 / 6,
                         4 * SK_Scalar1 / 6,
                         5 * SK_Scalar1 / 6,
                         SK_Scalar1};

    SkPaint     paint;
    paint.setShader(SkGradientShader::CreateRadial(
                    pt, radius,
                    colors, pos,
                    SK_ARRAY_COUNT(colors),
                    SkShader::kRepeat_TileMode))->unref();
    SkRect rect = SkRect::MakeWH(wScalar, hScalar);
    SkMatrix mat = SkMatrix::I();
    for (int i = 0; i < 4; ++i) {
        paint.getShader()->setLocalMatrix(mat);
        canvas.drawRect(rect, paint);
        rect.inset(wScalar / 8, hScalar / 8);
        mat.postScale(SK_Scalar1 / 4, SK_Scalar1 / 4);
    }
}

static const int gSize = 1024;

class DrawBitmapRectGM : public GM {
public:
    DrawBitmapRectGM() {
    }

    SkBitmap    fLargeBitmap;

protected:
    SkString onShortName() {
        return SkString("drawbitmaprect");
    }
    
    SkISize onISize() { return make_isize(gSize, gSize); }
    
    virtual void onDraw(SkCanvas* canvas) {
        static const int kBmpSize = 2048;
        if (fLargeBitmap.isNull()) {
            makebm(&fLargeBitmap,
                   SkBitmap::kARGB_8888_Config,
                   kBmpSize, kBmpSize);
        }
        SkRect dstRect = { 0, 0, SkIntToScalar(64), SkIntToScalar(64)};
        static const int kMaxSrcRectSize = 1 << (SkNextLog2(kBmpSize) + 2);

        static const int kPadX = 30;
        static const int kPadY = 40;
        SkPaint paint;
        paint.setAlpha(0x20);
        canvas->drawBitmapRect(fLargeBitmap, NULL,
                               SkRect::MakeWH(gSize * SK_Scalar1,
                                              gSize * SK_Scalar1),
                               &paint);
        canvas->translate(SK_Scalar1 * kPadX / 2,
                          SK_Scalar1 * kPadY / 2);
        SkPaint blackPaint;
        SkScalar titleHeight = SK_Scalar1 * 24;
        blackPaint.setColor(SK_ColorBLACK);
        blackPaint.setTextSize(titleHeight);
        blackPaint.setAntiAlias(true);
        SkString title;
        title.printf("Bitmap size: %d x %d", kBmpSize, kBmpSize);
        canvas->drawText(title.c_str(), title.size(), 0,
                         titleHeight, blackPaint);

        canvas->translate(0, SK_Scalar1 * kPadY / 2  + titleHeight);
        int rowCount = 0;
        canvas->save();
        for (int w = 1; w <= kMaxSrcRectSize; w *= 4) {
            for (int h = 1; h <= kMaxSrcRectSize; h *= 4) {

                SkIRect srcRect = SkIRect::MakeXYWH((kBmpSize - w) / 2,
                                                    (kBmpSize - h) / 2,
                                                    w, h);
                canvas->drawBitmapRect(fLargeBitmap, &srcRect, dstRect);

                SkString label;
                label.appendf("%d x %d", w, h);
                blackPaint.setAntiAlias(true);
                blackPaint.setStyle(SkPaint::kFill_Style);
                blackPaint.setTextSize(SK_Scalar1 * 10);
                SkScalar baseline = dstRect.height() +
                                    blackPaint.getTextSize() + SK_Scalar1 * 3;
                canvas->drawText(label.c_str(), label.size(),
                                    0, baseline,
                                    blackPaint);
                blackPaint.setStyle(SkPaint::kStroke_Style);
                blackPaint.setStrokeWidth(SK_Scalar1);
                blackPaint.setAntiAlias(false);
                canvas->drawRect(dstRect, blackPaint);

                canvas->translate(dstRect.width() + SK_Scalar1 * kPadX, 0);
                ++rowCount;
                if ((dstRect.width() + kPadX) * rowCount > gSize) {
                    canvas->restore();
                    canvas->translate(0, dstRect.height() + SK_Scalar1 * kPadY);
                    canvas->save();
                    rowCount = 0;
                }
            }
        }
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new DrawBitmapRectGM; }
static GMRegistry reg(MyFactory);

}

