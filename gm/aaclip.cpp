/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

namespace skiagm {

/** Draw a 2px border around the target, then red behind the target;
    set the clip to match the target, then draw >> the target in blue.
*/

void draw (SkCanvas* canvas, SkRect& target, int x, int y) {
    SkPaint borderPaint;
    borderPaint.setColor(SkColorSetRGB(0x0, 0xDD, 0x0));
    borderPaint.setAntiAlias(true);
    SkPaint backgroundPaint;
    backgroundPaint.setColor(SkColorSetRGB(0xDD, 0x0, 0x0));
    backgroundPaint.setAntiAlias(true);
    SkPaint foregroundPaint;
    foregroundPaint.setColor(SkColorSetRGB(0x0, 0x0, 0xDD));
    foregroundPaint.setAntiAlias(true);

    canvas->save();
    canvas->translate(SkIntToScalar(x), SkIntToScalar(y));
    target.inset(SkIntToScalar(-2), SkIntToScalar(-2));
    canvas->drawRect(target, borderPaint);
    target.inset(SkIntToScalar(2), SkIntToScalar(2));
    canvas->drawRect(target, backgroundPaint);
    canvas->clipRect(target, SkRegion::kIntersect_Op, true);
    target.inset(SkIntToScalar(-4), SkIntToScalar(-4));
    canvas->drawRect(target, foregroundPaint);
    canvas->restore();
}

void draw_square (SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(10 * SK_Scalar1, 10 * SK_Scalar1));
    draw(canvas, target, x, y);
}

void draw_column (SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(1 * SK_Scalar1, 10 * SK_Scalar1));
    draw(canvas, target, x, y);
}

void draw_bar (SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(10 * SK_Scalar1, 1 * SK_Scalar1));
    draw(canvas, target, x, y);
}

void draw_rect_tests (SkCanvas* canvas) {
    draw_square(canvas, 10, 10);
    draw_column(canvas, 30, 10);
    draw_bar(canvas, 10, 30);
}

/**
   Test a set of clipping problems discovered while writing blitAntiRect,
   and test all the code paths through the clipping blitters.
   Each region should show as a blue center surrounded by a 2px green
   border, with no red.
*/

class AAClipGM : public GM {
public:
    AAClipGM() {

    }

protected:
    virtual SkString onShortName() {
        return SkString("aaclip");
    }

    virtual SkISize onISize() {
        return make_isize(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) {
        // Initial pixel-boundary-aligned draw
        draw_rect_tests(canvas);

        // Repeat 4x with .2, .4, .6, .8 px offsets
        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);

        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);

        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);

        canvas->translate(SK_Scalar1 / 5, SK_Scalar1 / 5);
        canvas->translate(SkIntToScalar(50), 0);
        draw_rect_tests(canvas);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new AAClipGM; }
static GMRegistry reg(MyFactory);

}
