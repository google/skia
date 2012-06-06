/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

static SkCanvas* MakeCanvas(const SkIRect& bounds) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, bounds.width(), bounds.height());
    bm.allocPixels();
    bm.eraseColor(0);

    SkCanvas* canvas = new SkCanvas(bm);
    canvas->translate(-SkIntToScalar(bounds.fLeft), -SkIntToScalar(bounds.fTop));
    return canvas;
}

static void GetBitmap(const SkCanvas* canvas, SkBitmap* bm) {
    *bm = canvas->getDevice()->accessBitmap(false);
}

static void compare_canvas(const SkCanvas* a, const SkCanvas* b) {
    SkBitmap bma, bmb;
    GetBitmap(a, &bma);
    GetBitmap(b, &bmb);

    SkASSERT(bma.width() == bmb.width());
    SkASSERT(bma.height() == bmb.height());

    bma.lockPixels();
    bmb.lockPixels();
    for (int y = 0; y < bma.height(); ++y) {
        const SkPMColor* rowa = bma.getAddr32(0, y);
        const SkPMColor* rowb = bmb.getAddr32(0, y);
        SkASSERT(!memcmp(rowa, rowb, bma.width() << 2));
        
        for (int x = 1; x < bma.width() - 1; ++x) {
            SkASSERT(0xFF000000 == rowa[x]);
            SkASSERT(0xFF000000 == rowb[x]);
        }
    }
}

static void drawRectAsPath(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    SkPath path;
    path.addRect(r);
    canvas->drawPath(path, p);
}

static void test_maskFromPath(const SkPath& path) {
    SkIRect bounds;
    path.getBounds().roundOut(&bounds);

    SkPaint paint;
    paint.setAntiAlias(true);

    SkAutoTUnref<SkCanvas> path_canvas(MakeCanvas(bounds));
    path_canvas->drawPath(path, paint);

    SkAutoTUnref<SkCanvas> rect_canvas(MakeCanvas(bounds));
    drawRectAsPath(rect_canvas, path.getBounds(), paint);

    compare_canvas(path_canvas, rect_canvas);
}

static void test_mask() {
    for (int i = 1; i <= 20; ++i) {
        const SkScalar dx = SK_Scalar1 / i;
        const SkRect constr = SkRect::MakeWH(dx, SkIntToScalar(2));
        for (int n = 2; n < 20; ++n) {
            SkPath path;
            path.setFillType(SkPath::kEvenOdd_FillType);
            SkRect r = constr;
            while (r.fRight < SkIntToScalar(4)) {
                path.addRect(r);
                r.offset(dx, 0);
            }
            test_maskFromPath(path);
        }
    }
}

namespace skiagm {

/** Draw a 2px border around the target, then red behind the target;
    set the clip to match the target, then draw >> the target in blue.
*/

static void draw (SkCanvas* canvas, SkRect& target, int x, int y) {
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

static void draw_square (SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(10 * SK_Scalar1, 10 * SK_Scalar1));
    draw(canvas, target, x, y);
}

static void draw_column (SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(1 * SK_Scalar1, 10 * SK_Scalar1));
    draw(canvas, target, x, y);
}

static void draw_bar (SkCanvas* canvas, int x, int y) {
    SkRect target (SkRect::MakeWH(10 * SK_Scalar1, 1 * SK_Scalar1));
    draw(canvas, target, x, y);
}

static void draw_rect_tests (SkCanvas* canvas) {
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
        if (false) { // avoid bit rot, suppress warning
            test_mask();
        }

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
