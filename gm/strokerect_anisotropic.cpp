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
 *         whole pixel   half pixel   whole pixel   half pixel
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

    SkString onShortName() override {
        return SkString("strokerect_anisotropic");
    }

    SkISize onISize() override {
        return SkISize::Make(512, 512);
    }

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

        SkPaint::Join j = aaPaint.getStrokeJoin();
        SkDebugf("-------%d\n", j);

        draw_sqooshed_rect(canvas, {  20.5f, 35.0f }, aaPaint);
        draw_sqooshed_rect(canvas, {  55.0f, 35.0f }, aaPaint);
        draw_sqooshed_rect(canvas, {  20.5f, 105.0f }, bwPaint);
        draw_sqooshed_rect(canvas, {  55.0f, 105.0f }, bwPaint);

        aaPaint.setStrokeJoin(SkPaint::kBevel_Join);
        bwPaint.setStrokeJoin(SkPaint::kBevel_Join);

        draw_sqooshed_rect(canvas, {  90.5f, 35.0f }, aaPaint);
        draw_sqooshed_rect(canvas, { 125.0f, 35.0f }, aaPaint);
        draw_sqooshed_rect(canvas, {  90.5f, 105.0f }, bwPaint);
        draw_sqooshed_rect(canvas, { 125.0f, 105.0f }, bwPaint);

    }

private:
    typedef GM INHERITED;
};
DEF_GM(return new StrokeRectAnisotropicGM;)

