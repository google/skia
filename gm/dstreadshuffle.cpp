/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkBitmap.h"
#include "SkPath.h"
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

    SkString onShortName() override {
        return SkString("dstreadshuffle");
    }

    SkISize onISize() override {
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
                sk_tool_utils::set_portable_typeface(paint);
                canvas->drawText(text, strlen(text), 0, 0, *paint);
            }
            default:
                break;
        }
    }

    static SkColor GetColor(SkRandom* random, int i, int nextColor) {
        static SkColor colors[] = { SK_ColorRED,
                                    sk_tool_utils::color_to_565(0xFFFF7F00), // Orange
                                    SK_ColorYELLOW,
                                    SK_ColorGREEN,
                                    SK_ColorBLUE,
                                    sk_tool_utils::color_to_565(0xFF4B0082), // indigo
                                    sk_tool_utils::color_to_565(0xFF7F00FF) }; // violet
        SkColor color;
        int index = nextColor % SK_ARRAY_COUNT(colors);
        switch (i) {
            case 0:
                color = SK_ColorTRANSPARENT;
                break;
            case 1:
                color = SkColorSetARGB(0xff,
                                       SkColorGetR(colors[index]),
                                       SkColorGetG(colors[index]),
                                       SkColorGetB(colors[index]));
                break;
            default:
                uint8_t alpha = 0x80;
                color = SkColorSetARGB(alpha,
                                       SkColorGetR(colors[index]),
                                       SkColorGetG(colors[index]),
                                       SkColorGetB(colors[index]));
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

    void onDraw(SkCanvas* canvas) override {
        SkRandom random;
        SkScalar y = 100;
        for (int i = 0; i < kNumShapeTypes; i++) {
            ShapeType shapeType = static_cast<ShapeType>(i);
            SkScalar x = 25;
            for (int style = 0; style < 3; style++) {
                for (int width = 0; width <= 1; width++) {
                    for (int alpha = 0; alpha <= 2; alpha++) {
                        for (int r = 0; r <= 5; r++) {
                            SkColor color = GetColor(&random, alpha, style + width + alpha + r);

                            SkPaint p;
                            p.setAntiAlias(true);
                            p.setColor(color);
                            // In order to get some batching on the GPU backend we do 2 src over for
                            // each xfer mode which requires a dst read
                            p.setXfermodeMode(r % 3 == 0 ? SkXfermode::kLighten_Mode :
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
