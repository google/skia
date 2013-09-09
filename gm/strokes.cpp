
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "gm.h"
#include "SkRandom.h"

#define W   400
#define H   400
#define N   50

static const SkScalar SW = SkIntToScalar(W);
static const SkScalar SH = SkIntToScalar(H);

static void rnd_rect(SkRect* r, SkPaint* paint, SkLCGRandom& rand) {
    SkScalar x = rand.nextUScalar1() * W;
    SkScalar y = rand.nextUScalar1() * H;
    SkScalar w = rand.nextUScalar1() * (W >> 2);
    SkScalar h = rand.nextUScalar1() * (H >> 2);
    SkScalar hoffset = rand.nextSScalar1();
    SkScalar woffset = rand.nextSScalar1();

    r->set(x, y, x + w, y + h);
    r->offset(-w/2 + woffset, -h/2 + hoffset);

    paint->setColor(rand.nextU());
    paint->setAlpha(0xFF);
}


class StrokesGM : public skiagm::GM {
public:
    StrokesGM() {}

protected:
    virtual SkString onShortName() {
        return SkString("strokes_round");
    }

    virtual SkISize onISize() {
        return SkISize::Make(W, H*2);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(9)/2);

        for (int y = 0; y < 2; y++) {
            paint.setAntiAlias(!!y);
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(0, SH * y);
            canvas->clipRect(SkRect::MakeLTRB(
                                              SkIntToScalar(2), SkIntToScalar(2)
                                              , SW - SkIntToScalar(2), SH - SkIntToScalar(2)
                                              ));

            SkLCGRandom rand;
            for (int i = 0; i < N; i++) {
                SkRect r;
                rnd_rect(&r, &paint, rand);
                canvas->drawOval(r, paint);
                rnd_rect(&r, &paint, rand);
                canvas->drawRoundRect(r, r.width()/4, r.height()/4, paint);
                rnd_rect(&r, &paint, rand);
            }
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

class Strokes2GM : public skiagm::GM {
    SkPath fPath;
public:
    Strokes2GM() {
        SkLCGRandom rand;
        fPath.moveTo(0, 0);
        for (int i = 0; i < 13; i++) {
            SkScalar x = rand.nextUScalar1() * (W >> 1);
            SkScalar y = rand.nextUScalar1() * (H >> 1);
            fPath.lineTo(x, y);
        }
    }

protected:
    virtual SkString onShortName() {
        return SkString("strokes_poly");
    }

    virtual SkISize onISize() {
        return SkISize::Make(W, H*2);
    }

    static void rotate(SkScalar angle, SkScalar px, SkScalar py, SkCanvas* canvas) {
        SkMatrix matrix;
        matrix.setRotate(angle, px, py);
        canvas->concat(matrix);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->drawColor(SK_ColorWHITE);

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(9)/2);

        for (int y = 0; y < 2; y++) {
            paint.setAntiAlias(!!y);
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(0, SH * y);
            canvas->clipRect(SkRect::MakeLTRB(SkIntToScalar(2),
                                              SkIntToScalar(2),
                                              SW - SkIntToScalar(2),
                                              SH - SkIntToScalar(2)));

            SkLCGRandom rand;
            for (int i = 0; i < N/2; i++) {
                SkRect r;
                rnd_rect(&r, &paint, rand);
                rotate(SkIntToScalar(15), SW/2, SH/2, canvas);
                canvas->drawPath(fPath, paint);
            }
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkRect inset(const SkRect& r) {
    SkRect rr(r);
    rr.inset(r.width()/10, r.height()/10);
    return rr;
}

class Strokes3GM : public skiagm::GM {
    static void make0(SkPath* path, const SkRect& bounds, SkString* title) {
        path->addRect(bounds, SkPath::kCW_Direction);
        path->addRect(inset(bounds), SkPath::kCW_Direction);
        title->set("CW CW");
    }

    static void make1(SkPath* path, const SkRect& bounds, SkString* title) {
        path->addRect(bounds, SkPath::kCW_Direction);
        path->addRect(inset(bounds), SkPath::kCCW_Direction);
        title->set("CW CCW");
    }

    static void make2(SkPath* path, const SkRect& bounds, SkString* title) {
        path->addOval(bounds, SkPath::kCW_Direction);
        path->addOval(inset(bounds), SkPath::kCW_Direction);
        title->set("CW CW");
    }

    static void make3(SkPath* path, const SkRect& bounds, SkString* title) {
        path->addOval(bounds, SkPath::kCW_Direction);
        path->addOval(inset(bounds), SkPath::kCCW_Direction);
        title->set("CW CCW");
    }

    static void make4(SkPath* path, const SkRect& bounds, SkString* title) {
        path->addRect(bounds, SkPath::kCW_Direction);
        SkRect r = bounds;
        r.inset(bounds.width() / 10, -bounds.height() / 10);
        path->addOval(r, SkPath::kCW_Direction);
        title->set("CW CW");
    }

    static void make5(SkPath* path, const SkRect& bounds, SkString* title) {
        path->addRect(bounds, SkPath::kCW_Direction);
        SkRect r = bounds;
        r.inset(bounds.width() / 10, -bounds.height() / 10);
        path->addOval(r, SkPath::kCCW_Direction);
        title->set("CW CCW");
    }

public:
    Strokes3GM() {}

protected:
    virtual SkString onShortName() {
        return SkString("strokes3");
    }

    virtual SkISize onISize() {
        return SkISize::Make(W, H*2);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint origPaint;
        origPaint.setAntiAlias(true);
        origPaint.setStyle(SkPaint::kStroke_Style);
        SkPaint fillPaint(origPaint);
        fillPaint.setColor(SK_ColorRED);
        SkPaint strokePaint(origPaint);
        strokePaint.setColor(0xFF4444FF);

        void (*procs[])(SkPath*, const SkRect&, SkString*) = {
            make0, make1, make2, make3, make4, make5
        };

        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        SkRect bounds = SkRect::MakeWH(SkIntToScalar(50), SkIntToScalar(50));
        SkScalar dx = bounds.width() * 4/3;
        SkScalar dy = bounds.height() * 5;

        for (size_t i = 0; i < SK_ARRAY_COUNT(procs); ++i) {
            SkPath orig;
            SkString str;
            procs[i](&orig, bounds, &str);

            canvas->save();
            for (int j = 0; j < 13; ++j) {
                strokePaint.setStrokeWidth(SK_Scalar1 * j * j);
                canvas->drawPath(orig, strokePaint);
                canvas->drawPath(orig, origPaint);
                SkPath fill;
                strokePaint.getFillPath(orig, &fill);
                canvas->drawPath(fill, fillPaint);
                canvas->translate(dx + strokePaint.getStrokeWidth(), 0);
            }
            canvas->restore();
            canvas->translate(0, dy);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* F0(void*) { return new StrokesGM; }
static skiagm::GM* F1(void*) { return new Strokes2GM; }
static skiagm::GM* F2(void*) { return new Strokes3GM; }

static skiagm::GMRegistry R0(F0);
static skiagm::GMRegistry R1(F1);
static skiagm::GMRegistry R2(F2);
