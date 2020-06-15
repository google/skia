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

        SkPath p;
        p.moveTo(0, 0);
        p.lineTo(10, 0);
        p.lineTo(10, 10);
        SkPath c;
        c.moveTo(0, 0);
        c.cubicTo(10, 0, 10, 0, 10, 10);
        this->drawStrokes(canvas, p, c);
        canvas->translate(0, 120);

        p.reset();
        p.moveTo(0, 0);
        p.lineTo(0, -10);
        p.lineTo(0, 10);
        c.reset();
        c.moveTo(0, 0);
        c.cubicTo(0, -10, 0, -10, 0, 10);
        this->drawStrokes(canvas, p, c);
        canvas->translate(0, 120);

        p.reset();
        p.moveTo(0, 0);
        p.lineTo(0, -10);
        p.lineTo(10, -10);
        p.lineTo(10, 10);
        p.lineTo(0, 10);
        c.reset();
        c.moveTo(0, 0);
        c.cubicTo(0, -10, 10, 10, 0, 10);
        this->drawStrokes(canvas, p, c);
        canvas->translate(0, 140);

        p.reset();
        p.moveTo(0, 0);
        p.lineTo(0, -10);
        p.lineTo(10, -10);
        p.lineTo(10, 0);
        p.lineTo(0, 0);
        c.reset();
        c.moveTo(0, 0);
        c.cubicTo(0, -10, 10, 0, 0, 0);
        this->drawStrokes(canvas, p, c);
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
    typedef GM INHERITED;
};

DEF_GM( return new WideButtCaps; )
