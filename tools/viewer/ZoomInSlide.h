/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef ZoomInSlide_DEFINED
#define ZoomInSlide_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSurface.h"
#include "tools/viewer/ClickHandlerSlide.h"
#include "tools/viewer/Slide.h"

class SkString;

/**
 * A utility slide for visualizing small paths scaled up to show pixel rasterization.
 *
 * This slide renders a scene into a small offscreen surface (representing the "pixels"),
 * and then draws that surface scaled up onto the screen. This allows inspecting
 * anti-aliasing and rasterization details.
 *
 * It provides a grid overlay and handles coordinate mapping for mouse interactions (only
 * when control is pressed).
 */
class ZoomInSlide : public ClickHandlerSlide {
public:
    ZoomInSlide(size_t scale, size_t width, size_t height, SkString name)
            : fScale(scale), fWidth(width), fHeight(height) {
        fName = name;
    }

protected:
    const size_t fScale;
    const size_t fWidth;
    const size_t fHeight;

    /**
     * Helper to rasterize a path into a small surface and draw it scaled up.
     * This visualizes how the path is rasterized into pixels.
     *
     * @param canvas    The canvas to draw onto (already scaled by fScale).
     * @param path      The path to rasterize.
     * @param pathPaint The paint to use for rasterization.
     */
    void drawScaledPath(SkCanvas* canvas, const SkPath& path, const SkPaint& pathPaint) {
        auto ii = SkImageInfo::MakeN32Premul(fWidth, fHeight);
        auto surface = SkSurfaces::Raster(ii);
        SkASSERT_RELEASE(surface);

        surface->getCanvas()->clear(SK_ColorTRANSPARENT);
        surface->getCanvas()->drawPath(path, pathPaint);
        auto pathImg = surface->makeImageSnapshot();

        canvas->drawImage(pathImg, 0, 0);
    }

    class ScaledClick : public ClickHandlerSlide::Click {
    public:
        ScaledClick(size_t scale) : fScale(scale) {}

        // Returns the click coordinates in the small (unscaled) space.
        SkPoint currScaled() const { return {fCurr.fX / fScale, fCurr.fY / fScale}; }

    private:
        const size_t fScale;
    };

private:
    void draw(SkCanvas* canvas) override {
        canvas->scale(fScale, fScale);
        canvas->clear(SK_ColorWHITE);

        this->drawUnderGrid(canvas);
        this->drawGrid(canvas);
        this->drawOverGrid(canvas);
    }

    /**
     * Draw content that should be subject to "pixel inspection".
     */
    virtual void drawUnderGrid(SkCanvas* canvas) = 0;

    /**
     * Draw overlay content (like vector paths or handles) on top of the grid.
     * Coordinates are in the small (unscaled) space.
     */
    virtual void drawOverGrid(SkCanvas* canvas) = 0;

    void drawGrid(SkCanvas* canvas) {
        SkPaint gridPaint;
        gridPaint.setColor(SK_ColorDKGRAY);
        gridPaint.setStyle(SkPaint::Style::kStroke_Style);
        // Stroke width 0 means "hairline" (1 pixel on device).
        // Since the canvas is scaled, this will be a very thin line relative to the "big pixels".
        gridPaint.setStrokeWidth(0);

        for (int y = 0; y <= (int)fHeight; ++y) {
            canvas->drawLine(0, y, fWidth, y, gridPaint);
        }
        for (int x = 0; x <= (int)fWidth; ++x) {
            canvas->drawLine(x, 0, x, fHeight, gridPaint);
        }
    }

    /**
     * Convenient callback for handling clicks in either the zoomed-out or zoomed-in space
     */
    virtual void handleClick(const ScaledClick*) {}

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modifiers) override {
        // Only activate when Control is held to avoid interfering with normal viewer panning.
        if (modifiers != skui::ModifierKey::kControl) {
            return nullptr;
        }
        return new ScaledClick(fScale);
    }

    bool onClick(ClickHandlerSlide::Click* click) override {
        auto myClick = static_cast<ScaledClick*>(click);
        this->handleClick(myClick);
        return true;
    }
};

#endif
