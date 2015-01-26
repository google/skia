
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
 * Renders overlapping shapes with colorburn against a checkerboard.
 */
class DstReadShuffle : public GM {
public:
    DstReadShuffle() {
       this->setBGColor(SkColorSetARGB(0xff, 0xff, 0, 0xff));
    }

protected:
    enum ShapeType {
        kCircle_ShapeType,
        kRoundRect_ShapeType,
        kRect_ShapeType,
        kConvexPath_ShapeType,
        kConcavePath_ShapeType,
        kText_ShapeType,
        kNumShapeTypes
    };

    SkString onShortName() SK_OVERRIDE {
        return SkString("dstreadshuffle");
    }

    SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(kWidth, kHeight);
    }

    void drawShape(SkCanvas* canvas,
                   SkPaint* paint,
                   ShapeType type) {
        static const SkRect kRect = SkRect::MakeXYWH(SkIntToScalar(-50), SkIntToScalar(-50),
                                                     SkIntToScalar(75), SkIntToScalar(105));
        switch (type) {
            case kCircle_ShapeType:
                canvas->drawCircle(0, 0, 50, *paint);
                break;
            case kRoundRect_ShapeType:
                canvas->drawRoundRect(kRect, SkIntToScalar(10), SkIntToScalar(20), *paint);
                break;
            case kRect_ShapeType:
                canvas->drawRect(kRect, *paint);
                break;
            case kConvexPath_ShapeType:
                if (fConvexPath.isEmpty()) {
                    SkPoint points[4];
                    kRect.toQuad(points);
                    fConvexPath.moveTo(points[0]);
                    fConvexPath.quadTo(points[1], points[2]);
                    fConvexPath.quadTo(points[3], points[0]);
                    SkASSERT(fConvexPath.isConvex());
                }
                canvas->drawPath(fConvexPath, *paint);
                break;
            case kConcavePath_ShapeType:
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
                canvas->drawPath(fConcavePath, *paint);
                break;
            case kText_ShapeType: {
                const char* text = "Hello!";
                paint->setTextSize(30);
                canvas->drawText(text, strlen(text), 0, 0, *paint);
            }
            default:
                break;
        }
    }

    static SkColor GetColor(SkRandom* random, int i) {
        SkColor color;
        switch (i) {
            case 0:
                color = SK_ColorTRANSPARENT;
                break;
            case 1:
                color = SkColorSetARGB(0xff,
                                       random->nextULessThan(256),
                                       random->nextULessThan(256),
                                       random->nextULessThan(256));
                break;
            default:
                uint8_t alpha = random->nextULessThan(256);
                color = SkColorSetARGB(alpha,
                                       random->nextRangeU(0, alpha),
                                       random->nextRangeU(0, alpha),
                                       random->nextRangeU(0, alpha));
                break;
        }
        return color;
    }

    static void SetStyle(SkPaint* p, int style, int width) {
        switch (style) {
            case 0:
                p->setStyle(SkPaint::kStroke_Style);
                p->setStrokeWidth((SkScalar)width);
                break;
            case 1:
                p->setStyle(SkPaint::kStrokeAndFill_Style);
                p->setStrokeWidth((SkScalar)width);
                break;
            default:
                p->setStyle(SkPaint::kFill_Style);
                break;
        }
    }

    void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkRandom random;
        SkScalar y = 100;
        for (int i = 0; i < kNumShapeTypes; i++) {
            ShapeType shapeType = static_cast<ShapeType>(i);
            SkScalar x = 25;
            for (int style = 0; style < 3; style++) {
                for (int width = 0; width <= 1; width++) {
                    for (int alpha = 0; alpha <= 2; alpha++) {
                        for (int r = 0; r <= 5; r++) {
                            SkColor color = GetColor(&random, alpha);

                            SkPaint p;
                            p.setAntiAlias(true);
                            p.setColor(color);
                            p.setXfermodeMode(r % 3 == 0 ? SkXfermode::kHardLight_Mode :
                                                           SkXfermode::kSrcOver_Mode);
                            SetStyle(&p, style, width);
                            canvas->save();
                            canvas->translate(x, y);
                            canvas->rotate((SkScalar)(r < 3 ? 10 : 0));
                            this->drawShape(canvas, &p, shapeType);
                            canvas->restore();
                            x += 8;
                        }
                    }
                }
            }
            y += 50;
        }
    }

private:
    enum {
        kNumShapes = 100,
    };
    SkAutoTUnref<SkShader> fBG;
    SkPath                 fConcavePath;
    SkPath                 fConvexPath;
    static const int kWidth = 900;
    static const int kHeight = 400;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new DstReadShuffle; }
static GMRegistry reg(MyFactory);

}
