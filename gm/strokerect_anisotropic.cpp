/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"

static void draw_sqooshed_rect(SkCanvas* canvas, SkVector xlate, const SkPaint& p) {
    canvas->save();
        canvas->translate(xlate.fX, xlate.fY);
        canvas->scale(0.03f, 2.0f);
        canvas->drawRect(SkRect::MakeLTRB(-500, -10, 500, 10), p);
    canvas->restore();
}

/*
 * This GM is intended to wring out any lingering anisotropic
 * stroke rect bugs. It contains a repro case for crbug.com/935303
 * The pattern is:
 *
 *         miter @       miter @      bevel @       bevel @
 *         whole pixels  half pixels  whole pixels  half pixels
 *
 *   AA
 *
 * non-AA
 *
 */
class StrokeRectAnisotropicGM : public skiagm::GM {
public:
    StrokeRectAnisotropicGM() {}

protected:
    SkString getName() const override { return SkString("strokerect_anisotropic"); }

    SkISize getISize() override { return SkISize::Make(160, 160); }

    void onDraw(SkCanvas* canvas) override {

        SkPaint aaPaint;
        aaPaint.setColor(SkColorSetARGB(255, 0, 0, 0));
        aaPaint.setAntiAlias(true);
        aaPaint.setStrokeWidth(10);
        aaPaint.setStyle(SkPaint::kStroke_Style);

        SkPaint bwPaint;
        bwPaint.setColor(SkColorSetARGB(255, 0, 0, 0));
        bwPaint.setStrokeWidth(10);
        bwPaint.setStyle(SkPaint::kStroke_Style);

        // The two miter columns
        draw_sqooshed_rect(canvas, {  20.0f, 40.5f }, aaPaint);  // whole pixels
        draw_sqooshed_rect(canvas, {  20.0f, 110.5f }, bwPaint); // whole pixels

        draw_sqooshed_rect(canvas, {  60.5f, 40.0f }, aaPaint);  // half pixels
        draw_sqooshed_rect(canvas, {  60.5f, 110.0f }, bwPaint); // half pixels

        aaPaint.setStrokeJoin(SkPaint::kBevel_Join);
        bwPaint.setStrokeJoin(SkPaint::kBevel_Join);

        // The two bevel columns
        draw_sqooshed_rect(canvas, { 100.0f, 40.5f }, aaPaint);  // whole pixels
        draw_sqooshed_rect(canvas, { 100.0f, 110.5f }, bwPaint); // whole pixels

        draw_sqooshed_rect(canvas, { 140.5f, 40.0f }, aaPaint);  // half pixels
        draw_sqooshed_rect(canvas, { 140.5f, 110.0f }, bwPaint); // half pixels
    }

private:
    using INHERITED = GM;
};
DEF_GM(return new StrokeRectAnisotropicGM;)

