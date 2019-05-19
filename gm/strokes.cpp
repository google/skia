/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/private/SkFloatBits.h"
#include "include/utils/SkParsePath.h"
#include "include/utils/SkRandom.h"
#include "tools/ToolUtils.h"

#include <string.h>

#define W   400
#define H   400
#define N   50

constexpr SkScalar SW = SkIntToScalar(W);
constexpr SkScalar SH = SkIntToScalar(H);

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
    paint->setAlphaf(1.0f);
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
            fillPaint.setAlphaf(1.0f);
            strokePaint.setAlphaf(1.0f);
            strokePaint.setStrokeWidth(i ? 8.f : 10.f);
            strokePaint.setStrokeCap(i ? SkPaint::kSquare_Cap : SkPaint::kRound_Cap);
            canvas->save();
            canvas->translate(10 + i * 100.f, 10);
            canvas->drawPath(fMoveHfPath, strokePaint);
            canvas->translate(0, 20);
            canvas->drawPath(fMoveZfPath, strokePaint);
            dashPaint = strokePaint;
            const SkScalar intervals[] = { 0, 10 };
            dashPaint.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0));
            SkPath fillPath;
            dashPaint.getFillPath(fDashedfPath, &fillPath);
            canvas->translate(0, 20);
            canvas->drawPath(fDashedfPath, dashPaint);
            canvas->translate(0, 20);
            canvas->drawPath(fRefPath[i * 2], fillPaint);
            strokePaint.setStrokeWidth(20);
            strokePaint.setAlphaf(0.5f);
            canvas->translate(0, 50);
            canvas->drawPath(fMoveHfPath, strokePaint);
            canvas->translate(0, 30);
            canvas->drawPath(fMoveZfPath, strokePaint);
            canvas->translate(0, 30);
            fillPaint.setAlphaf(0.5f);
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
                canvas->rotate(SkIntToScalar(15), SW/2, SH/2);
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
        strokePaint.setColor(ToolUtils::color_to_565(0xFF4444FF));

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

DEF_SIMPLE_GM(zerolinedash, canvas, 256, 256) {
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SkColorSetARGB(255, 0, 0, 0));
    paint.setStrokeWidth(11);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeJoin(SkPaint::kBevel_Join);

    SkScalar dash_pattern[] = {1, 5};
    paint.setPathEffect(SkDashPathEffect::Make(dash_pattern, 2, 0));

    canvas->drawLine(100, 100, 100, 100, paint);
}

#ifdef PDF_IS_FIXED_SO_THIS_DOESNT_BREAK_IT
DEF_SIMPLE_GM(longrect_dash, canvas, 250, 250) {
    canvas->clear(SK_ColorWHITE);

    SkPaint paint;
    paint.setColor(SkColorSetARGB(255, 0, 0, 0));
    paint.setStrokeWidth(5);
    paint.setStrokeCap(SkPaint::kRound_Cap);
    paint.setStrokeJoin(SkPaint::kBevel_Join);
    paint.setStyle(SkPaint::kStroke_Style);
    SkScalar dash_pattern[] = {1, 5};
    paint.setPathEffect(SkDashPathEffect::Make(dash_pattern, 2, 0));
    // try all combinations of stretching bounds
    for (auto left : { 20.f, -100001.f } ) {
        for (auto top : { 20.f, -100001.f } ) {
            for (auto right : { 40.f, 100001.f } ) {
                for (auto bottom : { 40.f, 100001.f } ) {
                    canvas->save();
                    canvas->clipRect({10, 10, 50, 50});
                    canvas->drawRect({left, top, right, bottom}, paint);
                    canvas->restore();
                    canvas->translate(60, 0);
               }
            }
            canvas->translate(-60 * 4, 60);
        }
    }
}
#endif

DEF_SIMPLE_GM(bigconic, canvas, 250, 250) {
    SkPaint paint;
SkPath path;
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x7f07a5af), SkBits2Float(0xff07ff1d));  // 1.80306e+38f, -1.8077e+38f
path.lineTo(SkBits2Float(0x7edf4b2d), SkBits2Float(0xfedffe0a));  // 1.48404e+38f, -1.48868e+38f
path.lineTo(SkBits2Float(0x7edf4585), SkBits2Float(0xfee003b2));  // 1.48389e+38f, -1.48883e+38f
path.lineTo(SkBits2Float(0x7ef348e9), SkBits2Float(0xfef403c6));  // 1.6169e+38f, -1.62176e+38f
path.lineTo(SkBits2Float(0x7ef74c4e), SkBits2Float(0xfef803cb));  // 1.64358e+38f, -1.64834e+38f
path.conicTo(SkBits2Float(0x7ef74f23), SkBits2Float(0xfef8069e), SkBits2Float(0x7ef751f6), SkBits2Float(0xfef803c9), SkBits2Float(0x3f3504f3));  // 1.64365e+38f, -1.64841e+38f, 1.64372e+38f, -1.64834e+38f, 0.707107f
path.conicTo(SkBits2Float(0x7ef754c8), SkBits2Float(0xfef800f5), SkBits2Float(0x7ef751f5), SkBits2Float(0xfef7fe22), SkBits2Float(0x3f353472));  // 1.6438e+38f, -1.64827e+38f, 1.64372e+38f, -1.64819e+38f, 0.707832f
path.lineTo(SkBits2Float(0x7edb57a9), SkBits2Float(0xfedbfe06));  // 1.45778e+38f, -1.4621e+38f
path.lineTo(SkBits2Float(0x7e875976), SkBits2Float(0xfe87fdb3));  // 8.99551e+37f, -9.03815e+37f
path.lineTo(SkBits2Float(0x7ded5c2b), SkBits2Float(0xfdeff59e));  // 3.94382e+37f, -3.98701e+37f
path.lineTo(SkBits2Float(0x7d7a78a7), SkBits2Float(0xfd7fda0f));  // 2.08083e+37f, -2.12553e+37f
path.lineTo(SkBits2Float(0x7d7a6403), SkBits2Float(0xfd7fe461));  // 2.08016e+37f, -2.12587e+37f
path.conicTo(SkBits2Float(0x7d7a4764), SkBits2Float(0xfd7ff2b0), SkBits2Float(0x7d7a55b4), SkBits2Float(0xfd8007a8), SkBits2Float(0x3f3504f3));  // 2.07924e+37f, -2.12633e+37f, 2.0797e+37f, -2.12726e+37f, 0.707107f
path.conicTo(SkBits2Float(0x7d7a5803), SkBits2Float(0xfd8009f7), SkBits2Float(0x7d7a5ba9), SkBits2Float(0xfd800bcc), SkBits2Float(0x3f7cba66));  // 2.07977e+37f, -2.12741e+37f, 2.07989e+37f, -2.12753e+37f, 0.987219f
path.lineTo(SkBits2Float(0x7d8d2067), SkBits2Float(0xfd900bdb));  // 2.34487e+37f, -2.39338e+37f
path.lineTo(SkBits2Float(0x7ddd137a), SkBits2Float(0xfde00c2d));  // 3.67326e+37f, -3.72263e+37f
path.lineTo(SkBits2Float(0x7ddd2a1b), SkBits2Float(0xfddff58e));  // 3.67473e+37f, -3.72116e+37f
path.lineTo(SkBits2Float(0x7c694ae5), SkBits2Float(0xfc7fa67c));  // 4.8453e+36f, -5.30965e+36f
path.lineTo(SkBits2Float(0xfc164a8b), SkBits2Float(0x7c005af5));  // -3.12143e+36f, 2.66584e+36f
path.lineTo(SkBits2Float(0xfc8ae983), SkBits2Float(0x7c802da7));  // -5.77019e+36f, 5.32432e+36f
path.lineTo(SkBits2Float(0xfc8b16d9), SkBits2Float(0x7c80007b));  // -5.77754e+36f, 5.31699e+36f
path.lineTo(SkBits2Float(0xfc8b029c), SkBits2Float(0x7c7f8788));  // -5.77426e+36f, 5.30714e+36f
path.lineTo(SkBits2Float(0xfc8b0290), SkBits2Float(0x7c7f8790));  // -5.77425e+36f, 5.30714e+36f
path.lineTo(SkBits2Float(0xfc8b16cd), SkBits2Float(0x7c80007f));  // -5.77753e+36f, 5.31699e+36f
path.lineTo(SkBits2Float(0xfc8b4409), SkBits2Float(0x7c7fa672));  // -5.78487e+36f, 5.30965e+36f
path.lineTo(SkBits2Float(0x7d7aa2ba), SkBits2Float(0xfd800bd1));  // 2.0822e+37f, -2.12753e+37f
path.lineTo(SkBits2Float(0x7e8757ee), SkBits2Float(0xfe88035b));  // 8.99512e+37f, -9.03962e+37f
path.lineTo(SkBits2Float(0x7ef7552d), SkBits2Float(0xfef803ca));  // 1.64381e+38f, -1.64834e+38f
path.lineTo(SkBits2Float(0x7f0fa653), SkBits2Float(0xff1001f9));  // 1.90943e+38f, -1.91419e+38f
path.lineTo(SkBits2Float(0x7f0fa926), SkBits2Float(0xff0fff24));  // 1.90958e+38f, -1.91404e+38f
path.lineTo(SkBits2Float(0x7f0da75c), SkBits2Float(0xff0dff22));  // 1.8829e+38f, -1.88746e+38f
path.lineTo(SkBits2Float(0x7f07a5af), SkBits2Float(0xff07ff1d));  // 1.80306e+38f, -1.8077e+38f
path.close();
path.moveTo(SkBits2Float(0x7f07a2db), SkBits2Float(0xff0801f1));  // 1.80291e+38f, -1.80785e+38f
path.lineTo(SkBits2Float(0x7f0da48a), SkBits2Float(0xff0e01f8));  // 1.88275e+38f, -1.88761e+38f
path.lineTo(SkBits2Float(0x7f0fa654), SkBits2Float(0xff1001fa));  // 1.90943e+38f, -1.91419e+38f
path.lineTo(SkBits2Float(0x7f0fa7bd), SkBits2Float(0xff10008f));  // 1.90951e+38f, -1.91412e+38f
path.lineTo(SkBits2Float(0x7f0fa927), SkBits2Float(0xff0fff25));  // 1.90958e+38f, -1.91404e+38f
path.lineTo(SkBits2Float(0x7ef75ad5), SkBits2Float(0xfef7fe22));  // 1.64395e+38f, -1.64819e+38f
path.lineTo(SkBits2Float(0x7e875d96), SkBits2Float(0xfe87fdb3));  // 8.99659e+37f, -9.03815e+37f
path.lineTo(SkBits2Float(0x7d7acff6), SkBits2Float(0xfd7fea5b));  // 2.08367e+37f, -2.12606e+37f
path.lineTo(SkBits2Float(0xfc8b0588), SkBits2Float(0x7c8049b7));  // -5.77473e+36f, 5.32887e+36f
path.lineTo(SkBits2Float(0xfc8b2b16), SkBits2Float(0x7c803d32));  // -5.78083e+36f, 5.32684e+36f
path.conicTo(SkBits2Float(0xfc8b395c), SkBits2Float(0x7c803870), SkBits2Float(0xfc8b4405), SkBits2Float(0x7c802dd1), SkBits2Float(0x3f79349d));  // -5.78314e+36f, 5.32607e+36f, -5.78487e+36f, 5.32435e+36f, 0.973459f
path.conicTo(SkBits2Float(0xfc8b715b), SkBits2Float(0x7c8000a5), SkBits2Float(0xfc8b442f), SkBits2Float(0x7c7fa69e), SkBits2Float(0x3f3504f3));  // -5.79223e+36f, 5.31702e+36f, -5.7849e+36f, 5.30966e+36f, 0.707107f
path.lineTo(SkBits2Float(0xfc16ffaa), SkBits2Float(0x7bff4c12));  // -3.13612e+36f, 2.65116e+36f
path.lineTo(SkBits2Float(0x7c6895e0), SkBits2Float(0xfc802dc0));  // 4.83061e+36f, -5.32434e+36f
path.lineTo(SkBits2Float(0x7ddd137b), SkBits2Float(0xfde00c2e));  // 3.67326e+37f, -3.72263e+37f
path.lineTo(SkBits2Float(0x7ddd1ecb), SkBits2Float(0xfde000de));  // 3.67399e+37f, -3.72189e+37f
path.lineTo(SkBits2Float(0x7ddd2a1c), SkBits2Float(0xfddff58f));  // 3.67473e+37f, -3.72116e+37f
path.lineTo(SkBits2Float(0x7d8d3711), SkBits2Float(0xfd8ff543));  // 2.34634e+37f, -2.39191e+37f
path.lineTo(SkBits2Float(0x7d7a88fe), SkBits2Float(0xfd7fea69));  // 2.08136e+37f, -2.12606e+37f
path.lineTo(SkBits2Float(0x7d7a7254), SkBits2Float(0xfd800080));  // 2.08063e+37f, -2.1268e+37f
path.lineTo(SkBits2Float(0x7d7a80a4), SkBits2Float(0xfd800ed0));  // 2.08109e+37f, -2.12773e+37f
path.lineTo(SkBits2Float(0x7d7a80a8), SkBits2Float(0xfd800ecf));  // 2.08109e+37f, -2.12773e+37f
path.lineTo(SkBits2Float(0x7d7a7258), SkBits2Float(0xfd80007f));  // 2.08063e+37f, -2.1268e+37f
path.lineTo(SkBits2Float(0x7d7a5bb9), SkBits2Float(0xfd800bd0));  // 2.0799e+37f, -2.12753e+37f
path.lineTo(SkBits2Float(0x7ded458b), SkBits2Float(0xfdf00c3e));  // 3.94235e+37f, -3.98848e+37f
path.lineTo(SkBits2Float(0x7e8753ce), SkBits2Float(0xfe88035b));  // 8.99405e+37f, -9.03962e+37f
path.lineTo(SkBits2Float(0x7edb5201), SkBits2Float(0xfedc03ae));  // 1.45763e+38f, -1.46225e+38f
path.lineTo(SkBits2Float(0x7ef74c4d), SkBits2Float(0xfef803ca));  // 1.64358e+38f, -1.64834e+38f
path.lineTo(SkBits2Float(0x7ef74f21), SkBits2Float(0xfef800f6));  // 1.64365e+38f, -1.64827e+38f
path.lineTo(SkBits2Float(0x7ef751f4), SkBits2Float(0xfef7fe21));  // 1.64372e+38f, -1.64819e+38f
path.lineTo(SkBits2Float(0x7ef34e91), SkBits2Float(0xfef3fe1e));  // 1.61705e+38f, -1.62161e+38f
path.lineTo(SkBits2Float(0x7edf4b2d), SkBits2Float(0xfedffe0a));  // 1.48404e+38f, -1.48868e+38f
path.lineTo(SkBits2Float(0x7edf4859), SkBits2Float(0xfee000de));  // 1.48397e+38f, -1.48876e+38f
path.lineTo(SkBits2Float(0x7edf4585), SkBits2Float(0xfee003b2));  // 1.48389e+38f, -1.48883e+38f
path.lineTo(SkBits2Float(0x7f07a2db), SkBits2Float(0xff0801f1));  // 1.80291e+38f, -1.80785e+38f
path.close();
path.moveTo(SkBits2Float(0xfab120db), SkBits2Float(0x77b50b4f));  // -4.59851e+35f, 7.34402e+33f
path.lineTo(SkBits2Float(0xfd6597e5), SkBits2Float(0x7d60177f));  // -1.90739e+37f, 1.86168e+37f
path.lineTo(SkBits2Float(0xfde2cea1), SkBits2Float(0x7de00c2e));  // -3.76848e+37f, 3.72263e+37f
path.lineTo(SkBits2Float(0xfe316511), SkBits2Float(0x7e300657));  // -5.89495e+37f, 5.84943e+37f
path.lineTo(SkBits2Float(0xfe415da1), SkBits2Float(0x7e400666));  // -6.42568e+37f, 6.38112e+37f
path.lineTo(SkBits2Float(0xfe41634a), SkBits2Float(0x7e4000be));  // -6.42641e+37f, 6.38039e+37f
path.lineTo(SkBits2Float(0xfe41634a), SkBits2Float(0x7e3ff8be));  // -6.42641e+37f, 6.37935e+37f
path.lineTo(SkBits2Float(0xfe416349), SkBits2Float(0x7e3ff8be));  // -6.42641e+37f, 6.37935e+37f
path.lineTo(SkBits2Float(0xfe415f69), SkBits2Float(0x7e3ff8be));  // -6.42591e+37f, 6.37935e+37f
path.lineTo(SkBits2Float(0xfe415bc9), SkBits2Float(0x7e3ff8be));  // -6.42544e+37f, 6.37935e+37f
path.lineTo(SkBits2Float(0xfe415bc9), SkBits2Float(0x7e4000be));  // -6.42544e+37f, 6.38039e+37f
path.lineTo(SkBits2Float(0xfe416171), SkBits2Float(0x7e3ffb16));  // -6.42617e+37f, 6.37966e+37f
path.lineTo(SkBits2Float(0xfe016131), SkBits2Float(0x7dfff5ae));  // -4.29938e+37f, 4.25286e+37f
path.lineTo(SkBits2Float(0xfe0155e2), SkBits2Float(0x7e000628));  // -4.29791e+37f, 4.25433e+37f
path.lineTo(SkBits2Float(0xfe0958ea), SkBits2Float(0x7e080630));  // -4.56415e+37f, 4.52018e+37f
path.lineTo(SkBits2Float(0xfe115c92), SkBits2Float(0x7e100638));  // -4.83047e+37f, 4.78603e+37f
path.conicTo(SkBits2Float(0xfe11623c), SkBits2Float(0x7e100bdf), SkBits2Float(0xfe1167e2), SkBits2Float(0x7e100636), SkBits2Float(0x3f3504f3));  // -4.8312e+37f, 4.78676e+37f, -4.83194e+37f, 4.78603e+37f, 0.707107f
path.conicTo(SkBits2Float(0xfe116d87), SkBits2Float(0x7e10008e), SkBits2Float(0xfe1167e2), SkBits2Float(0x7e0ffae8), SkBits2Float(0x3f35240a));  // -4.83267e+37f, 4.78529e+37f, -4.83194e+37f, 4.78456e+37f, 0.707581f
path.lineTo(SkBits2Float(0xfe016b92), SkBits2Float(0x7dfff5af));  // -4.30072e+37f, 4.25286e+37f
path.lineTo(SkBits2Float(0xfdc2d963), SkBits2Float(0x7dbff56e));  // -3.23749e+37f, 3.18946e+37f
path.lineTo(SkBits2Float(0xfd65ae25), SkBits2Float(0x7d5fea3d));  // -1.90811e+37f, 1.86021e+37f
path.lineTo(SkBits2Float(0xfab448de), SkBits2Float(0xf7b50a19));  // -4.68046e+35f, -7.34383e+33f
path.lineTo(SkBits2Float(0xfab174d9), SkBits2Float(0x43480000));  // -4.60703e+35f, 200
path.lineTo(SkBits2Float(0xfab174d9), SkBits2Float(0x7800007f));  // -4.60703e+35f, 1.03848e+34f
path.lineTo(SkBits2Float(0xfab3f4db), SkBits2Float(0x7800007f));  // -4.67194e+35f, 1.03848e+34f
path.lineTo(SkBits2Float(0xfab3f4db), SkBits2Float(0x43480000));  // -4.67194e+35f, 200
path.lineTo(SkBits2Float(0xfab120db), SkBits2Float(0x77b50b4f));  // -4.59851e+35f, 7.34402e+33f
path.close();
path.moveTo(SkBits2Float(0xfab59cf2), SkBits2Float(0xf800007e));  // -4.71494e+35f, -1.03847e+34f
path.lineTo(SkBits2Float(0xfaa7cc52), SkBits2Float(0xf800007f));  // -4.35629e+35f, -1.03848e+34f
path.lineTo(SkBits2Float(0xfd6580e5), SkBits2Float(0x7d60177f));  // -1.90664e+37f, 1.86168e+37f
path.lineTo(SkBits2Float(0xfdc2c2c1), SkBits2Float(0x7dc00c0f));  // -3.23602e+37f, 3.19093e+37f
path.lineTo(SkBits2Float(0xfe016040), SkBits2Float(0x7e000626));  // -4.29925e+37f, 4.25433e+37f
path.lineTo(SkBits2Float(0xfe115c90), SkBits2Float(0x7e100636));  // -4.83047e+37f, 4.78603e+37f
path.lineTo(SkBits2Float(0xfe116239), SkBits2Float(0x7e10008f));  // -4.8312e+37f, 4.78529e+37f
path.lineTo(SkBits2Float(0xfe1167e0), SkBits2Float(0x7e0ffae6));  // -4.83194e+37f, 4.78456e+37f
path.lineTo(SkBits2Float(0xfe096438), SkBits2Float(0x7e07fade));  // -4.56562e+37f, 4.51871e+37f
path.lineTo(SkBits2Float(0xfe016130), SkBits2Float(0x7dfff5ac));  // -4.29938e+37f, 4.25286e+37f
path.lineTo(SkBits2Float(0xfe015b89), SkBits2Float(0x7e00007f));  // -4.29864e+37f, 4.25359e+37f
path.lineTo(SkBits2Float(0xfe0155e1), SkBits2Float(0x7e000627));  // -4.29791e+37f, 4.25433e+37f
path.lineTo(SkBits2Float(0xfe415879), SkBits2Float(0x7e4008bf));  // -6.42501e+37f, 6.38143e+37f
path.lineTo(SkBits2Float(0xfe415f69), SkBits2Float(0x7e4008bf));  // -6.42591e+37f, 6.38143e+37f
path.lineTo(SkBits2Float(0xfe416349), SkBits2Float(0x7e4008bf));  // -6.42641e+37f, 6.38143e+37f
path.lineTo(SkBits2Float(0xfe41634a), SkBits2Float(0x7e4008bf));  // -6.42641e+37f, 6.38143e+37f
path.conicTo(SkBits2Float(0xfe416699), SkBits2Float(0x7e4008bf), SkBits2Float(0xfe4168f1), SkBits2Float(0x7e400668), SkBits2Float(0x3f6c8ed9));  // -6.42684e+37f, 6.38143e+37f, -6.42715e+37f, 6.38113e+37f, 0.924055f
path.conicTo(SkBits2Float(0xfe416e9a), SkBits2Float(0x7e4000c2), SkBits2Float(0xfe4168f3), SkBits2Float(0x7e3ffb17), SkBits2Float(0x3f3504f3));  // -6.42788e+37f, 6.38039e+37f, -6.42715e+37f, 6.37966e+37f, 0.707107f
path.lineTo(SkBits2Float(0xfe317061), SkBits2Float(0x7e2ffb07));  // -5.89642e+37f, 5.84796e+37f
path.lineTo(SkBits2Float(0xfde2e542), SkBits2Float(0x7ddff58e));  // -3.76995e+37f, 3.72116e+37f
path.lineTo(SkBits2Float(0xfd65c525), SkBits2Float(0x7d5fea3d));  // -1.90886e+37f, 1.86021e+37f
path.lineTo(SkBits2Float(0xfab6c8db), SkBits2Float(0xf7b50b4f));  // -4.74536e+35f, -7.34402e+33f
path.lineTo(SkBits2Float(0xfab59cf2), SkBits2Float(0xf800007e));  // -4.71494e+35f, -1.03847e+34f
path.close();
path.moveTo(SkBits2Float(0xfab3f4db), SkBits2Float(0x43480000));  // -4.67194e+35f, 200
path.lineTo(SkBits2Float(0xfab174d9), SkBits2Float(0x43480000));  // -4.60703e+35f, 200
path.quadTo(SkBits2Float(0xfd0593a5), SkBits2Float(0x7d00007f), SkBits2Float(0xfd659785), SkBits2Float(0x7d6000de));  // -1.10971e+37f, 1.0634e+37f, -1.90737e+37f, 1.86095e+37f
path.quadTo(SkBits2Float(0xfda2cdf2), SkBits2Float(0x7da0009f), SkBits2Float(0xfdc2ce12), SkBits2Float(0x7dc000be));  // -2.70505e+37f, 2.6585e+37f, -3.23675e+37f, 3.1902e+37f
path.quadTo(SkBits2Float(0xfde2ce31), SkBits2Float(0x7de000de), SkBits2Float(0xfe0165e9), SkBits2Float(0x7e00007f));  // -3.76845e+37f, 3.72189e+37f, -4.29999e+37f, 4.25359e+37f
path.quadTo(SkBits2Float(0xfe1164b9), SkBits2Float(0x7e10008f), SkBits2Float(0xfe116239), SkBits2Float(0x7e10008f));  // -4.83153e+37f, 4.78529e+37f, -4.8312e+37f, 4.78529e+37f
path.quadTo(SkBits2Float(0xfe116039), SkBits2Float(0x7e10008f), SkBits2Float(0xfe095e91), SkBits2Float(0x7e080087));  // -4.83094e+37f, 4.78529e+37f, -4.56488e+37f, 4.51944e+37f
path.quadTo(SkBits2Float(0xfe015d09), SkBits2Float(0x7e00007f), SkBits2Float(0xfe015b89), SkBits2Float(0x7e00007f));  // -4.29884e+37f, 4.25359e+37f, -4.29864e+37f, 4.25359e+37f
path.lineTo(SkBits2Float(0xfe415bc9), SkBits2Float(0x7e4000be));  // -6.42544e+37f, 6.38039e+37f
path.quadTo(SkBits2Float(0xfe415da9), SkBits2Float(0x7e4000be), SkBits2Float(0xfe415f69), SkBits2Float(0x7e4000be));  // -6.42568e+37f, 6.38039e+37f, -6.42591e+37f, 6.38039e+37f
path.quadTo(SkBits2Float(0xfe416149), SkBits2Float(0x7e4000be), SkBits2Float(0xfe416349), SkBits2Float(0x7e4000be));  // -6.42615e+37f, 6.38039e+37f, -6.42641e+37f, 6.38039e+37f
path.quadTo(SkBits2Float(0xfe416849), SkBits2Float(0x7e4000be), SkBits2Float(0xfe316ab9), SkBits2Float(0x7e3000af));  // -6.42706e+37f, 6.38039e+37f, -5.89569e+37f, 5.84869e+37f
path.quadTo(SkBits2Float(0xfe216d29), SkBits2Float(0x7e20009f), SkBits2Float(0xfde2d9f2), SkBits2Float(0x7de000de));  // -5.36431e+37f, 5.31699e+37f, -3.76921e+37f, 3.72189e+37f
path.quadTo(SkBits2Float(0xfda2d9b2), SkBits2Float(0x7da0009f), SkBits2Float(0xfd65ae85), SkBits2Float(0x7d6000de));  // -2.70582e+37f, 2.6585e+37f, -1.90812e+37f, 1.86095e+37f
path.quadTo(SkBits2Float(0xfd05a9a6), SkBits2Float(0x7d00007f), SkBits2Float(0xfab3f4db), SkBits2Float(0x43480000));  // -1.11043e+37f, 1.0634e+37f, -4.67194e+35f, 200
path.close();
path.moveTo(SkBits2Float(0x7f07a445), SkBits2Float(0xff080087));  // 1.80299e+38f, -1.80778e+38f
path.quadTo(SkBits2Float(0x7f0ba519), SkBits2Float(0xff0c008b), SkBits2Float(0x7f0da5f3), SkBits2Float(0xff0e008d));  // 1.8562e+38f, -1.86095e+38f, 1.88283e+38f, -1.88753e+38f
path.quadTo(SkBits2Float(0x7f0fa6d5), SkBits2Float(0xff10008f), SkBits2Float(0x7f0fa7bd), SkBits2Float(0xff10008f));  // 1.90946e+38f, -1.91412e+38f, 1.90951e+38f, -1.91412e+38f
path.quadTo(SkBits2Float(0x7f0faa7d), SkBits2Float(0xff10008f), SkBits2Float(0x7ef75801), SkBits2Float(0xfef800f6));  // 1.90965e+38f, -1.91412e+38f, 1.64388e+38f, -1.64827e+38f
path.quadTo(SkBits2Float(0x7ecf5b09), SkBits2Float(0xfed000ce), SkBits2Float(0x7e875ac2), SkBits2Float(0xfe880087));  // 1.37811e+38f, -1.38242e+38f, 8.99585e+37f, -9.03889e+37f
path.quadTo(SkBits2Float(0x7e0eb505), SkBits2Float(0xfe10008f), SkBits2Float(0x7d7ab958), SkBits2Float(0xfd80007f));  // 4.74226e+37f, -4.78529e+37f, 2.08293e+37f, -2.1268e+37f
path.quadTo(SkBits2Float(0xfc8ac1cd), SkBits2Float(0x7c80007f), SkBits2Float(0xfc8b16cd), SkBits2Float(0x7c80007f));  // -5.76374e+36f, 5.31699e+36f, -5.77753e+36f, 5.31699e+36f
path.quadTo(SkBits2Float(0xfc8b36cd), SkBits2Float(0x7c80007f), SkBits2Float(0xfc16a51a), SkBits2Float(0x7c00007f));  // -5.78273e+36f, 5.31699e+36f, -3.12877e+36f, 2.6585e+36f
path.quadTo(SkBits2Float(0xfab6e4de), SkBits2Float(0x43480000), SkBits2Float(0x7c68f062), SkBits2Float(0xfc80007f));  // -4.7482e+35f, 200, 4.83795e+36f, -5.31699e+36f
path.lineTo(SkBits2Float(0x7ddd1ecb), SkBits2Float(0xfde000de));  // 3.67399e+37f, -3.72189e+37f
path.quadTo(SkBits2Float(0x7d9d254b), SkBits2Float(0xfda0009f), SkBits2Float(0x7d8d2bbc), SkBits2Float(0xfd90008f));  // 2.61103e+37f, -2.6585e+37f, 2.3456e+37f, -2.39265e+37f
path.quadTo(SkBits2Float(0x7d7a64d8), SkBits2Float(0xfd80007f), SkBits2Float(0x7d7a7258), SkBits2Float(0xfd80007f));  // 2.08019e+37f, -2.1268e+37f, 2.08063e+37f, -2.1268e+37f
path.quadTo(SkBits2Float(0x7d7a9058), SkBits2Float(0xfd80007f), SkBits2Float(0x7ded50db), SkBits2Float(0xfdf000ee));  // 2.0816e+37f, -2.1268e+37f, 3.94309e+37f, -3.98774e+37f
path.quadTo(SkBits2Float(0x7e2eace5), SkBits2Float(0xfe3000af), SkBits2Float(0x7e8756a2), SkBits2Float(0xfe880087));  // 5.80458e+37f, -5.84869e+37f, 8.99478e+37f, -9.03889e+37f
path.quadTo(SkBits2Float(0x7ebf56d9), SkBits2Float(0xfec000be), SkBits2Float(0x7edb54d5), SkBits2Float(0xfedc00da));  // 1.27167e+38f, -1.27608e+38f, 1.45771e+38f, -1.46217e+38f
path.quadTo(SkBits2Float(0x7ef752e1), SkBits2Float(0xfef800f6), SkBits2Float(0x7ef74f21), SkBits2Float(0xfef800f6));  // 1.64375e+38f, -1.64827e+38f, 1.64365e+38f, -1.64827e+38f
path.quadTo(SkBits2Float(0x7ef74d71), SkBits2Float(0xfef800f6), SkBits2Float(0x7ef34bbd), SkBits2Float(0xfef400f2));  // 1.64361e+38f, -1.64827e+38f, 1.61698e+38f, -1.62168e+38f
path.quadTo(SkBits2Float(0x7eef4a19), SkBits2Float(0xfef000ee), SkBits2Float(0x7edf4859), SkBits2Float(0xfee000de));  // 1.59035e+38f, -1.5951e+38f, 1.48397e+38f, -1.48876e+38f
path.lineTo(SkBits2Float(0x7f07a445), SkBits2Float(0xff080087));  // 1.80299e+38f, -1.80778e+38f
path.close();
canvas->drawPath(path, paint);
}
