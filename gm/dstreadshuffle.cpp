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
#include "SkSurface.h"

namespace skiagm {

/**
 * Renders overlapping shapes with colorburn against a checkerboard.
 */
class DstReadShuffle : public GM {
public:
    DstReadShuffle() { this->setBGColor(SK_ColorLTGRAY); }

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

    void drawShape(SkCanvas* canvas, SkPaint* paint, ShapeType type) {
        const SkRect kRect = SkRect::MakeXYWH(SkIntToScalar(0), SkIntToScalar(0),
                                              SkIntToScalar(45), SkIntToScalar(55));
        switch (type) {
            case kCircle_ShapeType:
                canvas->drawCircle(kRect.centerX(), kRect.centerY(), 25, *paint);
                break;
            case kRoundRect_ShapeType:
                canvas->drawRoundRect(kRect, SkIntToScalar(14), SkIntToScalar(14), *paint);
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
                    SkPoint points[5] = {{0, 0} };
                    SkMatrix rot;
                    rot.setRotate(SkIntToScalar(360) / 5, 0, 40);
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
                const char* text = "N";
                paint->setTextSize(50);
                paint->setFakeBoldText(true);
                sk_tool_utils::set_portable_typeface(paint);
                canvas->drawText(text, strlen(text), 0, 50, *paint);
            }
            default:
                break;
        }
    }

    static SkColor GetColor(SkRandom* random, uint8_t alpha) {
        int rgb[3];
        rgb[0] = random->nextULessThan(256);
        rgb[1] = random->nextULessThan(256);
        rgb[2] = random->nextULessThan(256);
        SkColor color = sk_tool_utils::color_to_565(SkColorSetRGB(rgb[0], rgb[1], rgb[2]));
        return SkColorSetA(color, alpha);
    }

    static void DrawHairlines(SkCanvas* canvas) {
        canvas->clear(SK_ColorTRANSPARENT);
        SkPaint hairPaint;
        hairPaint.setAlpha(0x80);
        hairPaint.setStyle(SkPaint::kStroke_Style);
        hairPaint.setStrokeWidth(0);
        hairPaint.setAntiAlias(true);
        static constexpr int kNumHairlines = 12;
        SkPoint pts[] = {{3.f, 7.f}, {29.f, 7.f}};
        SkRandom colorRandom;
        SkMatrix rot;
        rot.setRotate(360.f / kNumHairlines, 15.5, 12.f);
        rot.postTranslate(3.f, 0);
        for (int i = 0; i < 12; ++i) {
            hairPaint.setColor(GetColor(&colorRandom, 0xFF));
            canvas->drawLine(pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY, hairPaint);
            rot.mapPoints(pts, 2);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        //sk_tool_utils::draw_checkerboard(canvas, SK_ColorLTGRAY, SK_ColorDKGRAY, 12);
        SkScalar y = 5;
        for (int i = 0; i < kNumShapeTypes; i++) {
            SkRandom colorRandom;
            ShapeType shapeType = static_cast<ShapeType>(i);
            SkScalar x = 5;
            for (int style = 0; style < 3; style++) {
                for (int r = 0; r <= 5; r++) {
                    for (uint8_t alpha : {0x00, 0x60, 0xA0, 0xFF}) {
                        SkPaint p;
                        p.setAntiAlias(true);
                        p.setColor(GetColor(&colorRandom, alpha));
                        // In order to get some op combining on the GPU backend we do 2 src over
                        // for each xfer mode which requires a dst read
                        p.setBlendMode(r % 3 == 0 ? SkBlendMode::kLighten : SkBlendMode::kSrcOver);
                        canvas->save();
                        canvas->translate(x, y);
                        this->drawShape(canvas, &p, shapeType);
                        canvas->restore();
                        x += 5;
                    }
                }
            }
            y += 70;
        }
        // Draw hairlines to a surface and then draw that to the main canvas with a zoom so that
        // it is easier to see how they blend.
        SkImageInfo info;
        if (SkColorType::kUnknown_SkColorType != canvas->imageInfo().colorType()) {
            info = SkImageInfo::Make(35, 35,
                                     canvas->imageInfo().colorType(),
                                     canvas->imageInfo().alphaType(),
                                     canvas->imageInfo().refColorSpace());
        } else {
            info = SkImageInfo::MakeN32Premul(20, 20);
        }
        auto surf = canvas->makeSurface(info);
        canvas->scale(5.f, 5.f);
        canvas->translate(100, 5);
        DrawHairlines(surf->getCanvas());
        canvas->drawImage(surf->makeImageSnapshot(), 0, 0);
    }

private:
    SkPath               fConcavePath;
    SkPath               fConvexPath;
    static constexpr int kWidth = 900;
    static constexpr int kHeight = 400;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new DstReadShuffle; }
static GMRegistry reg(MyFactory);

}
