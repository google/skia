/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkPath.h"
#include "include/core/SkShader.h"
#include "include/core/SkSurface.h"
#include "include/utils/SkRandom.h"
#include "tools/ToolUtils.h"

namespace skiagm {

/**
 * Renders overlapping shapes with colorburn against a checkerboard.
 */
class DstReadShuffle : public GM {
public:
    DstReadShuffle() { this->setBGColor(kBackground); }

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
        return SkISize::Make(530, 680);
    }

    void drawShape(SkCanvas* canvas, SkPaint* paint, ShapeType type) {
        const SkRect kRect = SkRect::MakeXYWH(0, 0, 75.f, 85.f);
        switch (type) {
            case kCircle_ShapeType:
                canvas->drawCircle(kRect.centerX(), kRect.centerY(), kRect.width() / 2.f, *paint);
                break;
            case kRoundRect_ShapeType:
                canvas->drawRoundRect(kRect, 15.f, 15.f, *paint);
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
                    SkPoint points[5] = {{50.f, 0.f}};
                    SkMatrix rot;
                    rot.setRotate(360.f / 5, 50.f, 70.f);
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
                SkFont      font(ToolUtils::create_portable_typeface(), 100);
                font.setEmbolden(true);
                canvas->drawString(text, 0.f, 100.f, font, *paint);
            }
            default:
                break;
        }
    }

    static SkColor GetColor(SkRandom* random) {
        SkColor color = ToolUtils::color_to_565(random->nextU() | 0xFF000000);
        return SkColorSetA(color, 0x80);
    }

    static void DrawHairlines(SkCanvas* canvas) {
        if (canvas->imageInfo().alphaType() == kOpaque_SkAlphaType) {
            canvas->clear(kBackground);
        } else {
            canvas->clear(SK_ColorTRANSPARENT);
        }
        SkPaint hairPaint;
        hairPaint.setStyle(SkPaint::kStroke_Style);
        hairPaint.setStrokeWidth(0);
        hairPaint.setAntiAlias(true);
        static constexpr int kNumHairlines = 12;
        SkPoint pts[] = {{3.f, 7.f}, {29.f, 7.f}};
        SkRandom colorRandom;
        SkMatrix rot;
        rot.setRotate(360.f / kNumHairlines, 15.5f, 12.f);
        rot.postTranslate(3.f, 0);
        for (int i = 0; i < 12; ++i) {
            hairPaint.setColor(GetColor(&colorRandom));
            canvas->drawLine(pts[0], pts[1], hairPaint);
            rot.mapPoints(pts, 2);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar y = 5;
        for (int i = 0; i < kNumShapeTypes; i++) {
            SkRandom colorRandom;
            ShapeType shapeType = static_cast<ShapeType>(i);
            SkScalar x = 5;
            for (int r = 0; r <= 15; r++) {
                SkPaint p;
                p.setAntiAlias(true);
                p.setColor(GetColor(&colorRandom));
                // In order to get some op combining on the GPU backend we do 2 src over
                // for each xfer mode which requires a dst read
                p.setBlendMode(r % 3 == 0 ? SkBlendMode::kColorBurn : SkBlendMode::kSrcOver);
                canvas->save();
                canvas->translate(x, y);
                this->drawShape(canvas, &p, shapeType);
                canvas->restore();
                x += 15;
            }
            y += 110;
        }
        // Draw hairlines to a surface and then draw that to the main canvas with a zoom so that
        // it is easier to see how they blend.
        SkImageInfo info;
        // Recording canvases don't have a color type.
        if (SkColorType::kUnknown_SkColorType == canvas->imageInfo().colorType()) {
            info = SkImageInfo::MakeN32Premul(35, 35);
        } else {
            info = SkImageInfo::Make(35, 35,
                                     canvas->imageInfo().colorType(),
                                     canvas->imageInfo().alphaType(),
                                     canvas->imageInfo().refColorSpace());
        }
        auto surf = canvas->makeSurface(info);
        if (!surf) {
            // Fall back to raster. Raster supports only one of the 8 bit per-channel RGBA or BGRA
            // formats. This fall back happens when running with --preAbandonGpuContext.
            if ((info.colorType() == kRGBA_8888_SkColorType ||
                 info.colorType() == kBGRA_8888_SkColorType) &&
                info.colorType() != kN32_SkColorType) {
                info = SkImageInfo::Make(35, 35,
                                         kN32_SkColorType,
                                         canvas->imageInfo().alphaType(),
                                         canvas->imageInfo().refColorSpace());
            }
            surf = SkSurface::MakeRaster(info);
            SkASSERT(surf);
        }
        canvas->scale(5.f, 5.f);
        canvas->translate(67.f, 10.f);
        DrawHairlines(surf->getCanvas());
        canvas->drawImage(surf->makeImageSnapshot(), 0.f, 0.f);
    }

private:
    static constexpr SkColor kBackground = SK_ColorLTGRAY;
    SkPath fConcavePath;
    SkPath fConvexPath;
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new DstReadShuffle; )

}
