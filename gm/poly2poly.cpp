/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"
#include "SkPath.h"
#include "SkTypeface.h"

class SkJSCanvas {
public:
    SkJSCanvas(SkCanvas* target);
    ~SkJSCanvas();

    void save();
    void restore();

    double lineWidth;
    void setLineWidth(double);

    void beginPath();
    void moveTo(double x, double y);
    void lineTo(double x, double y);
    void closePath();

    void fill();
    void stroke();

    void fillText(const char text[], double x, double y);

private:
    SkCanvas*   fTarget;
    SkPaint     fFillPaint;
    SkPaint     fStrokePaint;
    SkPath      fPath;
};

SkJSCanvas::SkJSCanvas(SkCanvas* target) : fTarget(target) {
    fFillPaint.setAntiAlias(true);
    sk_tool_utils::set_portable_typeface(&fFillPaint);
    fStrokePaint.setAntiAlias(true);
    fStrokePaint.setStyle(SkPaint::kStroke_Style);
    fStrokePaint.setStrokeWidth(SK_Scalar1);
}

SkJSCanvas::~SkJSCanvas() {}

void SkJSCanvas::save() { fTarget->save(); }
void SkJSCanvas::restore() { fTarget->restore(); }

void SkJSCanvas::beginPath() { fPath.reset(); }
void SkJSCanvas::moveTo(double x, double y) {
    fPath.moveTo(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void SkJSCanvas::lineTo(double x, double y) {
    fPath.lineTo(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void SkJSCanvas::closePath() { fPath.close(); }

void SkJSCanvas::fill() {
    fTarget->drawPath(fPath, fFillPaint);
}

void SkJSCanvas::stroke() {
    fStrokePaint.setStrokeWidth(SkDoubleToScalar(lineWidth));
    fTarget->drawPath(fPath, fStrokePaint);
}

void SkJSCanvas::fillText(const char text[], double x, double y) {
    fTarget->drawText(text, strlen(text),
                      SkDoubleToScalar(x), SkDoubleToScalar(y), fFillPaint);
}

///////////////////////////////////////////////////////////////////////////////

static void dump(const SkPath& path) {
    const SkRect& r = path.getBounds();
    SkDebugf("isEmpty %d, bounds [%g %g %g %g]\n", path.isEmpty(),
             r.fLeft, r.fTop, r.fRight, r.fBottom);
}

static void test_stroke(SkCanvas* canvas) {
    if (true) {
        SkPath path;
        dump(path);
        path.reset(); path.moveTo(0, 0);
        dump(path);
        path.reset(); path.moveTo(100, 100);
        dump(path);
        path.reset(); path.moveTo(0, 0); path.moveTo(100, 100);
        dump(path);
        path.reset(); path.moveTo(0, 0); path.lineTo(100, 100);
        dump(path);
        path.reset(); path.moveTo(0, 0); path.lineTo(100, 100); path.moveTo(200, 200);
        dump(path);
    }

#if 0
    // TEST 1 - The rectangle as it's expected to look
    var canvas = document.createElement('canvas');
    document.body.appendChild(canvas);
    var ctx = canvas.getContext("2d");
#else
    SkJSCanvas ctx(canvas);
#endif

    ctx.save();
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(10, 100);
    ctx.lineTo(150, 100);
    ctx.lineTo(150, 15);
    ctx.lineTo(10, 15);
    ctx.closePath();

    // no extra moveTo here
    // ctx.moveTo(175, 125);

    ctx.stroke();
    ctx.restore();

    ctx.fillText("As Expected", 10, 10);

#if 0
    // TEST 2 - Includes an extra moveTo call before stroke; the rectangle appears larger
    canvas = document.createElement('canvas');
    document.body.appendChild(canvas);
    ctx = canvas.getContext("2d");
#else
    canvas->translate(200, 0);
#endif

    ctx.save();
    ctx.lineWidth = 2;
    ctx.beginPath();
    ctx.moveTo(10, 100);
    ctx.lineTo(150, 100);
    ctx.lineTo(150, 15);
    ctx.lineTo(10, 15);
    ctx.closePath();

    ctx.moveTo(175, 125);

    ctx.stroke();
    ctx.restore();

    ctx.fillText("Larger Rectangle", 10, 10);

#if 0
    // TEST 3 - Identical to test 2 except the line width is 1
    canvas = document.createElement('canvas');
    document.body.appendChild(canvas);
    ctx = canvas.getContext("2d");
#else
    canvas->translate(200, 0);
#endif

    ctx.save();
    ctx.lineWidth = 1;
    ctx.beginPath();
    ctx.moveTo(10, 100);
    ctx.lineTo(150, 100);
    ctx.lineTo(150, 15);
    ctx.lineTo(10, 15);
    ctx.closePath();

    ctx.moveTo(175, 125);

    ctx.stroke();
    ctx.restore();

    ctx.fillText("As Expected - line width 1", 10, 10);
}

class Poly2PolyGM : public skiagm::GM {
public:
    Poly2PolyGM() {}

protected:

    SkString onShortName() override {
        return SkString("poly2poly");
    }

    SkISize onISize() override {
        return SkISize::Make(835, 840);
    }

    static void doDraw(SkCanvas* canvas, SkPaint* paint, const int isrc[],
                       const int idst[], int count) {
        SkMatrix matrix;
        SkPoint src[4], dst[4];

        for (int i = 0; i < count; i++) {
            src[i].set(SkIntToScalar(isrc[2*i+0]), SkIntToScalar(isrc[2*i+1]));
            dst[i].set(SkIntToScalar(idst[2*i+0]), SkIntToScalar(idst[2*i+1]));
        }

        canvas->save();
        matrix.setPolyToPoly(src, dst, count);
        canvas->concat(matrix);

        paint->setColor(sk_tool_utils::color_to_565(SK_ColorGRAY));
        paint->setStyle(SkPaint::kStroke_Style);
        const SkScalar D = SkIntToScalar(64);
        canvas->drawRectCoords(0, 0, D, D, *paint);
        canvas->drawLine(0, 0, D, D, *paint);
        canvas->drawLine(0, D, D, 0, *paint);

        SkPaint::FontMetrics fm;
        paint->getFontMetrics(&fm);
        paint->setColor(SK_ColorRED);
        paint->setStyle(SkPaint::kFill_Style);
        SkScalar x = D/2;
        SkScalar y = D/2 - (fm.fAscent + fm.fDescent)/2;
        uint16_t glyphID = 3; // X
        canvas->drawText((void*) &glyphID, sizeof(glyphID), x, y, *paint);
        canvas->restore();
    }

    void onOnceBeforeDraw() override {
        fEmFace = MakeResourceAsTypeface("/fonts/Em.ttf");
    }

    void onDraw(SkCanvas* canvas) override {
        if (false) { test_stroke(canvas); return; }

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTypeface(fEmFace);
        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        paint.setStrokeWidth(SkIntToScalar(4));
        paint.setTextSize(SkIntToScalar(40));
        paint.setTextAlign(SkPaint::kCenter_Align);

        canvas->save();
        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));
        // translate (1 point)
        const int src1[] = { 0, 0 };
        const int dst1[] = { 5, 5 };
        doDraw(canvas, &paint, src1, dst1, 1);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(160), SkIntToScalar(10));
        // rotate/uniform-scale (2 points)
        const int src2[] = { 32, 32, 64, 32 };
        const int dst2[] = { 32, 32, 64, 48 };
        doDraw(canvas, &paint, src2, dst2, 2);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(10), SkIntToScalar(110));
        // rotate/skew (3 points)
        const int src3[] = { 0, 0, 64, 0, 0, 64 };
        const int dst3[] = { 0, 0, 96, 0, 24, 64 };
        doDraw(canvas, &paint, src3, dst3, 3);
        canvas->restore();

        canvas->save();
        canvas->translate(SkIntToScalar(160), SkIntToScalar(110));
        // perspective (4 points)
        const int src4[] = { 0, 0, 64, 0, 64, 64, 0, 64 };
        const int dst4[] = { 0, 0, 96, 0, 64, 96, 0, 64 };
        doDraw(canvas, &paint, src4, dst4, 4);
        canvas->restore();
    }

private:
    typedef skiagm::GM INHERITED;
    sk_sp<SkTypeface> fEmFace;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new Poly2PolyGM; )
