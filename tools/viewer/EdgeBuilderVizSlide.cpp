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
#include "tools/viewer/ClickHandlerSlide.h"
#include "tools/viewer/Slide.h"

#include <vector>

/* This slide visualizes the edges created from bezier curves in SkPath. These edges are the ones
   used by the aliased scan converter (e.g. SkScan_Path.cpp).

   Hold control to move the control point (which can be used in the path creation).
*/
class EdgeBuilderSlide : public ClickHandlerSlide {
public:
    EdgeBuilderSlide() { fName = "EdgeBuilderViz"; }

    void draw(SkCanvas* canvas) override {
        canvas->scale(kScale, kScale);
        canvas->clear(SK_ColorWHITE);

        SkPathBuilder pb;
        SkPath path = pb.moveTo(0, 0)
                              //.quadTo({fControlX, fControlY}, {10, 20})
                              .cubicTo({fControlX, fControlY}, {8, 6}, {10, 20})
                              .lineTo({10, 10})
                              .lineTo({12, 0})
                              .close()
                              .detach();

        drawScaledPath(canvas, path);
        drawGrid(canvas);
        drawHighRezOverlay(canvas, path);
        drawEdges(canvas, path);
        drawControlPoint(canvas);
    }

    // Draw path at normal size (typically very small) and then draw it into the scaled canvas.
    void drawScaledPath(SkCanvas* canvas, const SkPath& path) {
        uint8_t buffer[kMaskWidth * kMaskHeight * 4];
        auto ii = SkImageInfo::MakeN32Premul(kMaskWidth, kMaskHeight);
        SkPixmap pm1(ii, buffer, ii.minRowBytes());
        auto surface = SkSurfaces::WrapPixels(pm1);
        SkPaint pathPaint;
        pathPaint.setStyle(SkPaint::Style::kFill_Style);
        pathPaint.setColor(SK_ColorBLACK);
        surface->getCanvas()->clear(SK_ColorTRANSPARENT);
        surface->getCanvas()->drawPath(path, pathPaint);
        auto pathImg = surface->makeImageSnapshot();

        // Remember kScale applies to this canvas.
        canvas->drawImage(pathImg, 0, 0);
    }

    void drawGrid(SkCanvas* canvas) {
        SkPaint gridPaint;
        gridPaint.setColor(SK_ColorDKGRAY);
        gridPaint.setStyle(SkPaint::Style::kStroke_Style);
        gridPaint.setStrokeWidth(0);
        for (int y = 0; y <= kMaskHeight; y++) {
            canvas->drawLine(0, y, kMaskWidth, y, gridPaint);
        }
        for (int x = 0; x <= kMaskWidth; x++) {
            canvas->drawLine(x, 0, x, kMaskHeight, gridPaint);
        }
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

        canvas->drawCircle(fControlX, fControlY, 4.f / kScale, controlPaint);
    }

private:
    static constexpr size_t kScale = 32;
    // Defines the region of the screen that will be zoomed in on.
    // The path under scrutiny will be drawn in this region.
    static constexpr int kMaskWidth = 15;
    static constexpr int kMaskHeight = 25;

    class Click : public ClickHandlerSlide::Click {
    public:
        Click(float* x, float* y) : fX(x), fY(y) {}

        void doClick(EdgeBuilderSlide* that) {
            *fX = fCurr.fX / kScale;
            *fY = fCurr.fY / kScale;
        }

    private:
        float* fX;
        float* fY;
    };

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modifiers) override {
        if (modifiers != skui::ModifierKey::kControl) {
            return nullptr;
        }
        return new Click(&fControlX, &fControlY);
    }

    bool onClick(ClickHandlerSlide::Click* click) override {
        Click* myClick = (Click*)click;
        myClick->doClick(this);
        return true;
    }
    float fControlX = 2.f, fControlY = 13.f;
};
DEF_SLIDE(return new EdgeBuilderSlide();)
