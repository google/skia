/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"

static constexpr float kStrokeWidth = 100;

class WideButtCaps : public skiagm::GM {
public:
    WideButtCaps() {}

protected:

    SkString onShortName() override {
        return SkString("widebuttcaps");
    }

    SkISize onISize() override {
        return SkISize::Make(120 * 4, 120 * 3 + 140);
    }

    void onOnceBeforeDraw() override {
        fStrokePaint.setAntiAlias(true);
        fStrokePaint.setStrokeWidth(kStrokeWidth);
        fStrokePaint.setColor(SK_ColorGREEN);
        fStrokePaint.setStyle(SkPaint::kStroke_Style);
    }

    void onDraw(SkCanvas* canvas) override {
        SkAutoCanvasRestore arc(canvas, true);
        canvas->translate(60, 60);
        canvas->clear(SK_ColorBLACK);

        this->drawStrokes(canvas,
                SkPath().lineTo(10,0).lineTo(10,10),
                SkPath().cubicTo(10,0, 10,0, 10,10));
        canvas->translate(0, 120);

        this->drawStrokes(canvas,
                SkPath().lineTo(0,-10).lineTo(0,10),
                SkPath().cubicTo(0,-10, 0,-10, 0,10));
        canvas->translate(0, 120);

        this->drawStrokes(canvas,
                SkPath().lineTo(0,-10).lineTo(10,-10).lineTo(10,10).lineTo(0,10),
                SkPath().cubicTo(0,-10, 10,10, 0,10));
        canvas->translate(0, 140);

        this->drawStrokes(canvas,
                SkPath().lineTo(0,-10).lineTo(10,-10).lineTo(10,0).lineTo(0,0),
                SkPath().cubicTo(0,-10, 10,0, 0,0));
        canvas->translate(0, 120);
    }

    void drawStrokes(SkCanvas* canvas, const SkPath& path, const SkPath& cubic) {
        SkAutoCanvasRestore arc(canvas, true);
        fStrokePaint.setStrokeJoin(SkPaint::kBevel_Join);
        canvas->drawPath(path, fStrokePaint);

        canvas->translate(120, 0);
        fStrokePaint.setStrokeJoin(SkPaint::kRound_Join);
        canvas->drawPath(path, fStrokePaint);

        canvas->translate(120, 0);
        fStrokePaint.setStrokeJoin(SkPaint::kMiter_Join);
        canvas->drawPath(path, fStrokePaint);

        canvas->translate(120, 0);
        canvas->drawPath(cubic, fStrokePaint);
    }

private:
    SkPaint fStrokePaint;
    using INHERITED = GM;
};

DEF_GM( return new WideButtCaps; )
