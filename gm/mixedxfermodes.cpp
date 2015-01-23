
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
    enum ShapeType {
        kShapeTypeCircle,
        kShapeTypeRoundRect,
        kShapeTypeRect,
        kShapeTypeConvexPath,
        kShapeTypeConcavePath,
        kNumShapeTypes
    };

    SkString onShortName() SK_OVERRIDE {
        return SkString("mixed_xfermodes");
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(790, 640);
    }

    void drawShape(SkCanvas* canvas,
                   const SkPaint& paint,
                   ShapeType type) {
        static const SkRect kRect = SkRect::MakeXYWH(SkIntToScalar(-50), SkIntToScalar(-50),
                                                     SkIntToScalar(75), SkIntToScalar(105));
        switch (type) {
            case kShapeTypeCircle:
                canvas->drawCircle(0, 0, 50, paint);
                break;
            case kShapeTypeRoundRect:
                canvas->drawRoundRect(kRect, SkIntToScalar(10), SkIntToScalar(20), paint);
                break;
            case kShapeTypeRect:
                canvas->drawRect(kRect, paint);
                break;
            case kShapeTypeConvexPath:
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
            case kShapeTypeConcavePath:
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
            default:
                break;
        }
    }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        if (NULL == fBG.get()) {
            static uint32_t kCheckerPixelData[] = { 0xFFFFFFFF,
                                                    0xFFCCCCCC,
                                                    0xFFCCCCCC,
                                                    0xFFFFFFFF };
            SkBitmap bitmap;
            bitmap.allocN32Pixels(2, 2);
            memcpy(bitmap.getPixels(), kCheckerPixelData, sizeof(kCheckerPixelData));
            SkMatrix lm;
            lm.setScale(SkIntToScalar(20), SkIntToScalar(20));
            fBG.reset(SkShader::CreateBitmapShader(bitmap,
                                                   SkShader::kRepeat_TileMode,
                                                   SkShader::kRepeat_TileMode,
                                                   &lm));
        }

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
            ShapeType shapeType = static_cast<ShapeType>(random.nextULessThan(kNumShapeTypes));

            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(color);
            p.setXfermodeMode(mode);
            canvas->save();
            canvas->translate(dx, dy);
            canvas->scale(s, s);
            canvas->rotate(r);
            this->drawShape(canvas, p, shapeType);
            canvas->restore();
        }

        // This draw should not affect the test's result.
        drawWithHueOnWhite(canvas);
    }

    /**
     * Draws white color into a white square using the hue blend mode.
     * The result color should be white, so it doesn't change the expectations.
     * This will test a divide by 0 bug in shaders' setLum function,
     * which used to output black pixels.
     */
    void drawWithHueOnWhite(SkCanvas* canvas) {
        SkColor color = SkColorSetARGBMacro(225, 255, 255, 255);
        SkXfermode::Mode mode = SkXfermode::kHue_Mode;
        ShapeType shapeType = kShapeTypeConvexPath;

        // Make it fit into a square.
        SkScalar s = 0.15f;
        // Look for a clean white square.
        SkScalar dx = 30.f;
        SkScalar dy = 350.f;

        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(color);
        p.setXfermodeMode(mode);
        canvas->save();
        canvas->translate(dx, dy);
        canvas->scale(s, s);
        this->drawShape(canvas, p, shapeType);
        canvas->restore();
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
