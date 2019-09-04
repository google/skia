/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "samplecode/Sample.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"

#include "tools/ToolUtils.h"

namespace {

} // anonymous namespace

static constexpr float kLineHeight = 16.f;
static constexpr float kLineInset = 8.f;

static float print_size(SkCanvas* canvas, const char* prefix, const SkIRect& rect,
                       float x, float y, const SkFont& font, const SkPaint& paint) {
    canvas->drawString(prefix, x, y, font, paint);
    y += kLineHeight;
    SkString sz;
    sz.appendf("%d x %d", rect.width(), rect.height());
    canvas->drawString(sz, x, y, font, paint);
    return y + kLineHeight;
}

static float print_info(SkCanvas* canvas, const SkIRect& origLayerBounds, const SkIRect& localLayerBounds,
                        const SkIRect& filterInputBounds, const SkIRect& devLayerBounds) {
    SkFont font(nullptr, 12);
    SkPaint text;
    text.setAntiAlias(true);

    float y = kLineHeight;

    text.setColor(SK_ColorBLACK);
    y = print_size(canvas, "Orig layer", origLayerBounds, kLineInset, y, font, text);
    text.setColor(SK_ColorRED);
    y = print_size(canvas, "Filter layer", localLayerBounds, kLineInset, y, font, text);
    text.setColor(SK_ColorBLUE);
    y = print_size(canvas, "Filter input", filterInputBounds, kLineInset, y, font, text);
    text.setColor(SK_ColorMAGENTA);
    y = print_size(canvas, "Backdrop size", devLayerBounds, kLineInset, y, font, text);

    return y;
}

static SkPaint line_paint(SkScalar width, SkColor color) {
    SkPaint paint;
    paint.setColor(color);
    paint.setStrokeWidth(width);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setAntiAlias(true);
    return paint;
}

class BackdropBoundsSample : public Sample {
public:
    BackdropBoundsSample() {}

    void onDrawContent(SkCanvas* canvas) override {
        SkMatrix ctm = canvas->getTotalMatrix();

        // This decomposition is for the backdrop filtering, and does not represent the CTM that
        // the layer actually uses (unless it also has a filter during restore).
        SkMatrix toGlobal, layerMatrix;
        SkSize scale;
        if (ctm.isScaleTranslate()) {
            // No decomposition needed
            toGlobal = SkMatrix::I();
            layerMatrix = ctm;
        } else if (ctm.decomposeScale(&scale, &toGlobal)) {
            layerMatrix = SkMatrix::MakeScale(scale.fWidth, scale.fHeight);
        } else {
            toGlobal = ctm;
            layerMatrix = SkMatrix::I();
        }

        SkMatrix fromGlobal;
        if (!toGlobal.invert(&fromGlobal)) {
            SkDebugf("Unable to invert CTM\n");
            return;
        }

        // The local content, e.g. what would be submitted to drawRect
        const SkRect localContentRect = SkRect::MakeLTRB(45.5f, 23.123f, 150.f, 140.45f);
        canvas->drawRect(localContentRect, line_paint(0.f, SK_ColorBLACK));

        canvas->save();
        // The layer bounds of the content, this is the size of the actual layer and does not
        // reflect the backdrop specific decomposition.
        canvas->setMatrix(SkMatrix::I());
        SkIRect origLayerBounds = ctm.mapRect(localContentRect).roundOut();
        canvas->drawRect(SkRect::Make(origLayerBounds), line_paint(1.f, SK_ColorBLACK));

        // Have to undo the full CTM transform on the layer bounds to get the layer bounds
        // for the specific backdrop filter decomposition
        canvas->setMatrix(toGlobal);
        SkIRect layerBounds = fromGlobal.mapRect(SkRect::Make(origLayerBounds)).roundOut();
        canvas->drawRect(SkRect::Make(layerBounds), line_paint(0.5f, SK_ColorRED));

        // Input bounds for the backdrop filter to cover the actual layer bounds (emulate some
        // blur that must outset by 5px for reading on the edge).
        SkIRect filterInputBounds = layerBounds;
        filterInputBounds.outset(5, 5);
        canvas->drawRect(SkRect::Make(filterInputBounds), line_paint(1.f, SK_ColorBLUE));


        // The destination bounds that must be snapped in order to transform and fill the
        // filterInputBounds
        canvas->setMatrix(SkMatrix::I());
        SkIRect devLayerBounds = toGlobal.mapRect(SkRect::Make(filterInputBounds)).roundOut();
        canvas->drawRect(SkRect::Make(devLayerBounds), line_paint(2.f, SK_ColorMAGENTA));

        // The destination bounds mapped back into the layer space, which should cover 'layerBounds'
        SkPath backdropCoveringBounds;

        // Add axis lines, to show perspective distortion
        SkIRect local = fromGlobal.mapRect(SkRect::Make(devLayerBounds)).roundOut();
        static int kAxisSpace = 10;
        for (int y = local.fTop + kAxisSpace; y <= local.fBottom - kAxisSpace; y += kAxisSpace) {
            backdropCoveringBounds.moveTo(local.fLeft, y);
            backdropCoveringBounds.lineTo(local.fRight, y);
        }
        for (int x = local.fLeft + kAxisSpace; x <= local.fRight - kAxisSpace; x += kAxisSpace) {
            backdropCoveringBounds.moveTo(x, local.fTop);
            backdropCoveringBounds.lineTo(x, local.fBottom);
        }

        canvas->setMatrix(toGlobal);
        canvas->drawPath(backdropCoveringBounds, line_paint(0.f, SK_ColorGREEN));

        canvas->resetMatrix();
        print_info(canvas, origLayerBounds, layerBounds, filterInputBounds, devLayerBounds);

        canvas->restore();
    }

    SkString name() override { return SkString("BackdropBounds"); }

private:

    typedef Sample INHERITED;
};

DEF_SAMPLE(return new BackdropBoundsSample();)
