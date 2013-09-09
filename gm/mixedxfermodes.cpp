
/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkBitmap.h"
#include "SkRandom.h"
#include "SkShader.h"
#include "SkXfermode.h"

namespace skiagm {

/**
 * Renders overlapping shapes with random SkXfermode::Modes against a checkerboard.
 */
class MixedXfermodesGM : public GM {
public:
    MixedXfermodesGM() {
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("mixed_xfermodes");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return make_isize(790, 640);
    }

    void drawShape(SkCanvas* canvas,
                   const SkPaint& paint,
                   SkRandom* random) {
        static const SkRect kRect = SkRect::MakeXYWH(SkIntToScalar(-50), SkIntToScalar(-50),
                                                     SkIntToScalar(75), SkIntToScalar(105));
        int shape = random->nextULessThan(5);
        switch (shape) {
        case 0:
            canvas->drawCircle(0, 0, 50, paint);
            break;
        case 1:
            canvas->drawRoundRect(kRect, SkIntToScalar(10), SkIntToScalar(20), paint);
            break;
        case 2:
            canvas->drawRect(kRect, paint);
            break;
        case 3:
            if (fConvexPath.isEmpty()) {
                SkPoint points[4];
                kRect.toQuad(points);
                fConvexPath.moveTo(points[0]);
                fConvexPath.quadTo(points[1], points[2]);
                fConvexPath.quadTo(points[3], points[0]);
                SkASSERT(fConvexPath.isConvex());
            }
            canvas->drawPath(fConvexPath, paint);
            break;
        case 4:
            if (fConcavePath.isEmpty()) {
                SkPoint points[5] = {{0, SkIntToScalar(-50)} };
                SkMatrix rot;
                rot.setRotate(SkIntToScalar(360) / 5);
                for (int i = 1; i < 5; ++i) {
                    rot.mapPoints(points + i, points + i - 1, 1);
                }
                fConcavePath.moveTo(points[0]);
                for (int i = 0; i < 5; ++i) {
                    fConcavePath.lineTo(points[(2 * i) % 5]);
                }
                fConcavePath.setFillType(SkPath::kEvenOdd_FillType);
                SkASSERT(!fConcavePath.isConvex());
            }
            canvas->drawPath(fConcavePath, paint);
            break;
        }
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        if (NULL == fBG.get()) {
            static uint32_t kCheckerPixelData[] = { 0xFFFFFFFF,
                                                    0xFFCCCCCC,
                                                    0xFFCCCCCC,
                                                    0xFFFFFFFF };
            SkBitmap bitmap;
            bitmap.setConfig(SkBitmap::kARGB_8888_Config, 2, 2, 2 * sizeof(uint32_t));
            bitmap.allocPixels();
            bitmap.lockPixels();
            memcpy(bitmap.getPixels(), kCheckerPixelData, sizeof(kCheckerPixelData));
            bitmap.unlockPixels();
            fBG.reset(SkShader::CreateBitmapShader(bitmap,
                                                   SkShader::kRepeat_TileMode,
                                                   SkShader::kRepeat_TileMode));
        }
        SkMatrix lm;
        lm.setScale(SkIntToScalar(20), SkIntToScalar(20));
        fBG->setLocalMatrix(lm);

        SkPaint bgPaint;
        bgPaint.setShader(fBG.get());
        canvas->drawPaint(bgPaint);
        SkISize size = canvas->getDeviceSize();
        SkScalar maxScale = SkScalarSqrt((SkIntToScalar(size.fWidth * size.fHeight))) / 300;
        SkRandom random;
        for (int i = 0; i < kNumShapes; ++i) {
            SkScalar s = random.nextRangeScalar(SK_Scalar1 / 8, SK_Scalar1) * maxScale;
            SkScalar r = random.nextRangeScalar(0, SkIntToScalar(360));
            SkScalar dx = random.nextRangeScalar(0, SkIntToScalar(size.fWidth));
            SkScalar dy = random.nextRangeScalar(0, SkIntToScalar(size.fHeight));
            SkColor color = random.nextU();
            SkXfermode::Mode mode =
                static_cast<SkXfermode::Mode>(random.nextULessThan(SkXfermode::kLastMode + 1));

            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(color);
            p.setXfermodeMode(mode);
            canvas->save();
            canvas->translate(dx, dy);
            canvas->scale(s, s);
            canvas->rotate(r);
            this->drawShape(canvas, p, &random);
            canvas->restore();
        }
    }

    virtual uint32_t onGetFlags() const {
        // Skip PDF rasterization since rendering this PDF takes forever.
        return kSkipPDFRasterization_Flag;
    }

private:
    enum {
        kNumShapes = 100,
    };
    SkAutoTUnref<SkShader> fBG;
    SkPath                 fConcavePath;
    SkPath                 fConvexPath;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new MixedXfermodesGM; }
static GMRegistry reg(MyFactory);

}
