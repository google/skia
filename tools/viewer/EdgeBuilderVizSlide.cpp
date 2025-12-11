/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkSurface.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkDebugUtils.h"
#include "src/core/SkEdge.h"
#include "src/core/SkEdgeBuilder.h"
#include "tools/viewer/Slide.h"
#include "tools/viewer/ZoomInSlide.h"

#include <vector>

/* This slide visualizes the edges created from bezier curves in SkPath. These edges are the ones
   used by the aliased scan converter (e.g. SkScan_Path.cpp).

   Hold control to move the control point (which can be used in the path creation).
*/
class EdgeBuilderSlide : public ZoomInSlide {
public:
    EdgeBuilderSlide() : ZoomInSlide(kScale, kMaskWidth, kMaskHeight, SkString("EdgeBuilderViz")) {}

    SkPath makePath() {
        return SkPathBuilder()
                .moveTo(0, 0)
                //.quadTo({fControl, {10, 20})
                .cubicTo(fControl, {8, 6}, {10, 20})
                .lineTo({10, 10})
                .lineTo({12, 0})
                .close()
                .detach();
    }

    void drawUnderGrid(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setStyle(SkPaint::Style::kFill_Style);
        paint.setColor(SK_ColorBLACK);
        this->drawScaledPath(canvas, makePath(), paint);
    }

    void drawOverGrid(SkCanvas* canvas) override {
        SkPath path = makePath();
        drawHighRezOverlay(canvas, path);
        drawEdges(canvas, path);
        drawControlPoint(canvas);
    }

    void drawHighRezOverlay(SkCanvas* canvas, const SkPath& path) {
        SkPaint truthPaint;
        truthPaint.setStyle(SkPaint::Style::kStroke_Style);
        truthPaint.setColor(SK_ColorRED);
        truthPaint.setAntiAlias(true);
        truthPaint.setStrokeWidth(2.f / kScale);

        canvas->drawPath(path, truthPaint);
    }

    void drawEdges(SkCanvas* canvas, const SkPath& path) {
        SkBasicEdgeBuilder builder;
        int num = builder.buildEdges(path, nullptr);
        SkEdge** edgeList = builder.edgeList();

        SkPaint edgePaint;
        edgePaint.setColor(SkColorSetRGB(0xFF, 0x8C, 0x00));
        edgePaint.setStyle(SkPaint::Style::kStroke_Style);
        edgePaint.setStrokeWidth(4.f / kScale);

        for (int i = 0; i < num; ++i) {
            SkEdge* e = edgeList[i];
            while (true) {
                float x1 = SkFixedToFloat(e->fX);
                float y1 = e->fFirstY;
                float y2 = e->fLastY;
                float x2 = x1 + SkFixedToFloat(e->fDxDy) * (y2 - y1);
                if (x1 == x2 && y1 == y2) {
                    x2 += 0.2f;  // Make "zero height" edges visible.
                }
                // The y coordinates are implied to be at the half pixel values for y
                canvas->drawLine(x1, y1 + 0.5, x2, y2 + 0.5, edgePaint);
                if (!e->hasNextSegment()) {
                    break;
                }
                e->fX = x2;
                if (!e->nextSegment()) {
                    break;
                }
            }
        }
    }

    void drawControlPoint(SkCanvas* canvas) {
        SkPaint controlPaint;
        controlPaint.setStyle(SkPaint::Style::kStroke_Style);
        controlPaint.setColor(SK_ColorBLUE);
        controlPaint.setAntiAlias(true);
        controlPaint.setStrokeWidth(1.f / kScale);

        canvas->drawCircle(fControl, 4.f / kScale, controlPaint);
    }

    void handleClick(const ScaledClick* click) override { fControl = click->currScaled(); }

private:
    static constexpr size_t kScale = 32;
    // Defines the region of the screen that will be zoomed in on.
    // The path under scrutiny will be drawn in this region.
    static constexpr int kMaskWidth = 15;
    static constexpr int kMaskHeight = 25;

    SkPoint fControl = {2.f, 13.f};
};
DEF_SLIDE(return new EdgeBuilderSlide();)
