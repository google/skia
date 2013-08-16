/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"

static void test_quadstroke(SkCanvas* canvas) {
    SkPath path;
    path.moveTo(6, 0);
    path.quadTo(150, 150, 0, 6);

    SkPaint paint;

    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    canvas->translate(20, 20);

#if 1
    canvas->drawPath(path, paint);
    canvas->translate(100, 0);
#endif

    paint.setStrokeWidth(1.01f);
    canvas->drawPath(path, paint);
}

static void draw_conic(SkCanvas* canvas, SkScalar weight, const SkPaint& paint) {
    SkPath path;
    path.moveTo(100, 100);
    path.conicTo(300, 100, 300, 300, weight);
    canvas->drawPath(path, paint);
}

static void test_conic(SkCanvas* canvas) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);

    static const struct {
        SkScalar fWeight;
        SkColor  fColor;
    } gRec[] = {
        { 2   , SK_ColorRED },
        { 1   , SK_ColorGREEN },
        { 0.5f, SK_ColorBLUE },
    };

    for (SkScalar width = 0; width <= 20; width += 20) {
        canvas->save();
        paint.setStrokeWidth(width);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
            paint.setColor(gRec[i].fColor);
            draw_conic(canvas, gRec[i].fWeight, paint);
            canvas->translate(-30, 30);
        }
        canvas->restore();
        canvas->translate(300, 0);
    }
}

#include "SkGradientShader.h"
static void test_shallow_gradient(SkCanvas* canvas, SkScalar width, SkScalar height) {
    SkColor colors[] = { 0xFF7F7F7F, 0xFF7F7F7F, 0xFF000000 };
    SkScalar pos[] = { 0, 0.35f, SK_Scalar1 };
    SkPoint pts[] = { { 0, 0 }, { width, height } };
    SkShader* s = SkGradientShader::CreateLinear(pts, colors, pos,
                                                 SK_ARRAY_COUNT(colors),
                                                 SkShader::kClamp_TileMode);
    SkPaint paint;
    paint.setShader(s)->unref();
    canvas->drawPaint(paint);
}

#include "SkDashPathEffect.h"
static void test_giant_dash(SkCanvas* canvas) {
    SkPaint paint;
    const SkScalar intervals[] = { SK_Scalar1, SK_Scalar1 };

    paint.setStrokeWidth(2);
    paint.setPathEffect(new SkDashPathEffect(intervals, 2, 0))->unref();

    SkScalar big = 500 * 1000;

    canvas->drawLine(10, 10, big, 10, paint);
    canvas->drawLine(-big, 20, 500, 20, paint);
    canvas->drawLine(-big, 30, big, 30, paint);

    const SkScalar intervals2[] = { 20, 5, 10, 5 };
    paint.setPathEffect(new SkDashPathEffect(intervals2, 4, 17))->unref();

    canvas->translate(0, 40);
    SkScalar x = -500;
    SkScalar width = 3173;
    for (int i = 0; i < 40; ++i) {
        if (i > 10)
        canvas->drawLine(x, 0, x + width, 0, paint);
        x += 1;
        canvas->translate(0, 4);
    }
}



// Reproduces bug found here: http://jsfiddle.net/R8Cu5/1/
//
#include "SkGradientShader.h"
static void test_grad(SkCanvas* canvas) {
    SkPoint pts[] = {
        { 478.544067f, -84.2041016f },
        { 602.455933f, 625.204102f },
    };
    SkColor colors[] = { SK_ColorBLACK, SK_ColorBLACK, SK_ColorRED, SK_ColorRED };
    SkScalar pos[] = { 0, 0.3f, 0.3f, 1.0f };
    SkShader* s = SkGradientShader::CreateLinear(pts, colors, pos, 4, SkShader::kClamp_TileMode);
    SkPaint p;
    p.setShader(s)->unref();
    canvas->drawPaint(p);
}

static SkCanvas* MakeCanvas(const SkIRect& bounds) {
    SkBitmap bm;
    bm.setConfig(SkBitmap::kARGB_8888_Config, bounds.width(), bounds.height());
    bm.allocPixels();
    bm.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas* canvas = new SkCanvas(bm);
    canvas->translate(-SkIntToScalar(bounds.fLeft), -SkIntToScalar(bounds.fTop));
    return canvas;
}

#ifdef SK_DEBUG
static void GetBitmap(const SkCanvas* canvas, SkBitmap* bm) {
    *bm = canvas->getDevice()->accessBitmap(false);
}
#endif

static void compare_canvas(const SkCanvas* a, const SkCanvas* b) {
#ifdef SK_DEBUG
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
#endif
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

class AAClipGM : public skiagm::GM {
public:
    AAClipGM() {

    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("aaclip");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        if (false) { test_quadstroke(canvas); return; }
        if (false) { test_conic(canvas); return; }
        if (false) {
            SkRect bounds;
            canvas->getClipBounds(&bounds);
            test_shallow_gradient(canvas, bounds.width(), bounds.height()); return;
        }
        if (false) {
            test_giant_dash(canvas); return;
        }
        if (false) {
            test_grad(canvas); return;
        }
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

    virtual uint32_t onGetFlags() const { return kSkipPipe_Flag; }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM( return SkNEW(AAClipGM); )
