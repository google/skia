/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkDashPathEffect.h"
#include "SkParsePath.h"

#define W   400
#define H   400
#define N   50

static const SkScalar SW = SkIntToScalar(W);
static const SkScalar SH = SkIntToScalar(H);

static void rnd_rect(SkRect* r, SkPaint* paint, SkRandom& rand) {
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

    SkString onShortName() override {
        return SkString("strokes_round");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H*2);
    }

    void onDraw(SkCanvas* canvas) override {
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

            SkRandom rand;
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

/* See
   https://code.google.com/p/chromium/issues/detail?id=422974          and
   http://jsfiddle.net/1xnku3sg/2/
 */
class ZeroLenStrokesGM : public skiagm::GM {
    SkPath fMoveHfPath, fMoveZfPath, fDashedfPath, fRefPath[4];
    SkPath fCubicPath, fQuadPath, fLinePath;
protected:
    void onOnceBeforeDraw() override {

        SkAssertResult(SkParsePath::FromSVGString("M0,0h0M10,0h0M20,0h0", &fMoveHfPath));
        SkAssertResult(SkParsePath::FromSVGString("M0,0zM10,0zM20,0z", &fMoveZfPath));
        SkAssertResult(SkParsePath::FromSVGString("M0,0h25", &fDashedfPath));
        SkAssertResult(SkParsePath::FromSVGString("M 0 0 C 0 0 0 0 0 0", &fCubicPath));
        SkAssertResult(SkParsePath::FromSVGString("M 0 0 Q 0 0 0 0", &fQuadPath));
        SkAssertResult(SkParsePath::FromSVGString("M 0 0 L 0 0", &fLinePath));

        for (int i = 0; i < 3; ++i) {
            fRefPath[0].addCircle(i * 10.f, 0, 5);
            fRefPath[1].addCircle(i * 10.f, 0, 10);
            fRefPath[2].addRect(i * 10.f - 4, -2, i * 10.f + 4, 6);
            fRefPath[3].addRect(i * 10.f - 10, -10, i * 10.f + 10, 10);
        }
    }

    SkString onShortName() override {
        return SkString("zeroPath");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H*2);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint fillPaint, strokePaint, dashPaint;
        fillPaint.setAntiAlias(true);
        strokePaint = fillPaint;
        strokePaint.setStyle(SkPaint::kStroke_Style);
        for (int i = 0; i < 2; ++i) {
            fillPaint.setAlpha(255);
            strokePaint.setAlpha(255);
            strokePaint.setStrokeWidth(i ? 8.f : 10.f);
            strokePaint.setStrokeCap(i ? SkPaint::kSquare_Cap : SkPaint::kRound_Cap);
            canvas->save();
            canvas->translate(10 + i * 100.f, 10);
            canvas->drawPath(fMoveHfPath, strokePaint);
            canvas->translate(0, 20);
            canvas->drawPath(fMoveZfPath, strokePaint);
            dashPaint = strokePaint;
            const SkScalar intervals[] = { 0, 10 };
            dashPaint.setPathEffect(SkDashPathEffect::Create(intervals, 2, 0))->unref();
            SkPath fillPath;
            dashPaint.getFillPath(fDashedfPath, &fillPath);
            canvas->translate(0, 20);
            canvas->drawPath(fDashedfPath, dashPaint);
            canvas->translate(0, 20);
            canvas->drawPath(fRefPath[i * 2], fillPaint);
            strokePaint.setStrokeWidth(20);
            strokePaint.setAlpha(127);
            canvas->translate(0, 50);
            canvas->drawPath(fMoveHfPath, strokePaint);
            canvas->translate(0, 30);
            canvas->drawPath(fMoveZfPath, strokePaint);
            canvas->translate(0, 30);
            fillPaint.setAlpha(127);
            canvas->drawPath(fRefPath[1 + i * 2], fillPaint);
            canvas->translate(0, 30);
            canvas->drawPath(fCubicPath, strokePaint);
            canvas->translate(0, 30);
            canvas->drawPath(fQuadPath, strokePaint);
            canvas->translate(0, 30);
            canvas->drawPath(fLinePath, strokePaint);
            canvas->restore();
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

class TeenyStrokesGM : public skiagm::GM {

    SkString onShortName() override {
        return SkString("teenyStrokes");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H*2);
    }

    static void line(SkScalar scale, SkCanvas* canvas, SkColor color) {
        SkPaint p;
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setColor(color);
        canvas->translate(50, 0);
	    canvas->save();
        p.setStrokeWidth(scale * 5);
	    canvas->scale(1 / scale, 1 / scale);
        canvas->drawLine(20 * scale, 20 * scale, 20 * scale, 100 * scale, p);
        canvas->drawLine(20 * scale, 20 * scale, 100 * scale, 100 * scale, p);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        line(0.00005f, canvas, SK_ColorBLACK);
        line(0.000045f, canvas, SK_ColorRED);
        line(0.0000035f, canvas, SK_ColorGREEN);
        line(0.000003f, canvas, SK_ColorBLUE);
        line(0.000002f, canvas, SK_ColorBLACK);
    }
private:
    typedef skiagm::GM INHERITED;
};

DEF_SIMPLE_GM(CubicStroke, canvas, 384, 384) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(1.0720f);
	SkPath path;
    path.moveTo(-6000,-6000);
    path.cubicTo(-3500,5500,-500,5500,2500,-6500);
    canvas->drawPath(path, p);
    p.setStrokeWidth(1.0721f);
    canvas->translate(10, 10);
    canvas->drawPath(path, p);
    p.setStrokeWidth(1.0722f);
    canvas->translate(10, 10);
    canvas->drawPath(path, p);
}

DEF_SIMPLE_GM(zerolinestroke, canvas, 90, 120) {
    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(20);
    paint.setAntiAlias(true);
    paint.setStrokeCap(SkPaint::kRound_Cap);

    SkPath path;
    path.moveTo(30, 90);
    path.lineTo(30, 90);
    path.lineTo(60, 90);
    path.lineTo(60, 90);
    canvas->drawPath(path, paint);

    path.reset();
    path.moveTo(30, 30);
    path.lineTo(60, 30);
    canvas->drawPath(path, paint);

    path.reset();
    path.moveTo(30, 60);
    path.lineTo(30, 60);
    path.lineTo(60, 60);
    canvas->drawPath(path, paint);
}

DEF_SIMPLE_GM(quadcap, canvas, 200, 200) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kStroke_Style);
    p.setStrokeWidth(0);
    SkPath path;
    SkPoint pts[] = {{105.738571f,13.126318f},
            {105.738571f,13.126318f}, 
            {123.753784f,1.f}};
    SkVector tangent = pts[1] - pts[2];
    tangent.normalize();
    SkPoint pts2[3];
    memcpy(pts2, pts, sizeof(pts));
    const SkScalar capOutset = SK_ScalarPI / 8;
    pts2[0].fX += tangent.fX * capOutset;
    pts2[0].fY += tangent.fY * capOutset;
    pts2[1].fX += tangent.fX * capOutset;
    pts2[1].fY += tangent.fY * capOutset;
    pts2[2].fX += -tangent.fX * capOutset;
    pts2[2].fY += -tangent.fY * capOutset;
    path.moveTo(pts2[0]);
    path.quadTo(pts2[1], pts2[2]);
    canvas->drawPath(path, p);

    path.reset();
    path.moveTo(pts[0]);
    path.quadTo(pts[1], pts[2]);
    p.setStrokeCap(SkPaint::kRound_Cap);
    canvas->translate(30, 0);
    canvas->drawPath(path, p);
}

class Strokes2GM : public skiagm::GM {
    SkPath fPath;
protected:
    void onOnceBeforeDraw() override {
        SkRandom rand;
        fPath.moveTo(0, 0);
        for (int i = 0; i < 13; i++) {
            SkScalar x = rand.nextUScalar1() * (W >> 1);
            SkScalar y = rand.nextUScalar1() * (H >> 1);
            fPath.lineTo(x, y);
        }
    }


    SkString onShortName() override {
        return SkString("strokes_poly");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H*2);
    }

    static void rotate(SkScalar angle, SkScalar px, SkScalar py, SkCanvas* canvas) {
        SkMatrix matrix;
        matrix.setRotate(angle, px, py);
        canvas->concat(matrix);
    }

    void onDraw(SkCanvas* canvas) override {
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

            SkRandom rand;
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

    SkString onShortName() override {
        return SkString("strokes3");
    }

    SkISize onISize() override {
        return SkISize::Make(1500, 1500);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint origPaint;
        origPaint.setAntiAlias(true);
        origPaint.setStyle(SkPaint::kStroke_Style);
        SkPaint fillPaint(origPaint);
        fillPaint.setColor(SK_ColorRED);
        SkPaint strokePaint(origPaint);
        strokePaint.setColor(sk_tool_utils::color_to_565(0xFF4444FF));

        void (*procs[])(SkPath*, const SkRect&, SkString*) = {
            make0, make1, make2, make3, make4, make5
        };

        canvas->translate(SkIntToScalar(20), SkIntToScalar(80));

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

class Strokes4GM : public skiagm::GM {
public:
    Strokes4GM() {}

protected:

    SkString onShortName() override {
        return SkString("strokes_zoomed");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H*2);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(0.055f);
    
        canvas->scale(1000, 1000);
        canvas->drawCircle(0, 2, 1.97f, paint);
    }

private:
    typedef skiagm::GM INHERITED;
};

// Test stroking for curves that produce degenerate tangents when t is 0 or 1 (see bug 4191)
class Strokes5GM : public skiagm::GM {
public:
    Strokes5GM() {}

protected:

    SkString onShortName() override {
        return SkString("zero_control_stroke");
    }

    SkISize onISize() override {
        return SkISize::Make(W, H*2);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint p;
        p.setColor(SK_ColorRED);
        p.setAntiAlias(true);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(40);
        p.setStrokeCap(SkPaint::kButt_Cap);

        SkPath path;
        path.moveTo(157.474f,111.753f);
        path.cubicTo(128.5f,111.5f,35.5f,29.5f,35.5f,29.5f);
        canvas->drawPath(path, p);
        path.reset();
        path.moveTo(250, 50);
        path.quadTo(280, 80, 280, 80);
        canvas->drawPath(path, p);
        path.reset();
        path.moveTo(150, 50);
        path.conicTo(180, 80, 180, 80, 0.707f);
        canvas->drawPath(path, p);

        path.reset();
        path.moveTo(157.474f,311.753f);
        path.cubicTo(157.474f,311.753f,85.5f,229.5f,35.5f,229.5f);
        canvas->drawPath(path, p);
        path.reset();
        path.moveTo(280, 250);
        path.quadTo(280, 250, 310, 280);
        canvas->drawPath(path, p);
        path.reset();
        path.moveTo(180, 250);
        path.conicTo(180, 250, 210, 280, 0.707f);
        canvas->drawPath(path, p);
    }

private:
    typedef skiagm::GM INHERITED;
};


//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new StrokesGM; )
DEF_GM( return new Strokes2GM; )
DEF_GM( return new Strokes3GM; )
DEF_GM( return new Strokes4GM; )
DEF_GM( return new Strokes5GM; )

DEF_GM( return new ZeroLenStrokesGM; )
DEF_GM( return new TeenyStrokesGM; )
