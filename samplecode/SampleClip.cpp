/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/core/SkClipOpPriv.h"

constexpr int W = 150;
constexpr int H = 200;

static void show_text(SkCanvas* canvas, bool doAA) {
    SkRandom rand;
    SkPaint paint;
    SkFont font(nullptr, 20);
    font.setEdging(doAA ? SkFont::Edging::kSubpixelAntiAlias : SkFont::Edging::kAlias);

    for (int i = 0; i < 200; ++i) {
        paint.setColor((SK_A32_MASK << SK_A32_SHIFT) | rand.nextU());
        canvas->drawString("Hamburgefons", rand.nextSScalar1() * W, rand.nextSScalar1() * H + 20,
                           font, paint);
    }
}

static void show_fill(SkCanvas* canvas, bool doAA) {
    SkRandom rand;
    SkPaint paint;
    paint.setAntiAlias(doAA);

    for (int i = 0; i < 50; ++i) {
        SkRect r;
        SkPath p;

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        canvas->drawRect(r, paint);

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        p.addOval(r);
        canvas->drawPath(p, paint);
    }
}

static SkScalar randRange(SkRandom& rand, SkScalar min, SkScalar max) {
    SkASSERT(min <= max);
    return min + rand.nextUScalar1() * (max - min);
}

static void show_stroke(SkCanvas* canvas, bool doAA, SkScalar strokeWidth, int n) {
    SkRandom rand;
    SkPaint paint;
    paint.setAntiAlias(doAA);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(strokeWidth);

    for (int i = 0; i < n; ++i) {
        SkRect r;
        SkPath p;

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        canvas->drawRect(r, paint);

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        p.addOval(r);
        canvas->drawPath(p, paint);

        const SkScalar minx = -SkIntToScalar(W)/4;
        const SkScalar maxx = 5*SkIntToScalar(W)/4;
        const SkScalar miny = -SkIntToScalar(H)/4;
        const SkScalar maxy = 5*SkIntToScalar(H)/4;
        paint.setColor(rand.nextU());
        canvas->drawLine(randRange(rand, minx, maxx), randRange(rand, miny, maxy),
                         randRange(rand, minx, maxx), randRange(rand, miny, maxy),
                         paint);
    }
}

static void show_hair(SkCanvas* canvas, bool doAA) {
    show_stroke(canvas, doAA, 0, 150);
}

static void show_thick(SkCanvas* canvas, bool doAA) {
    show_stroke(canvas, doAA, SkIntToScalar(5), 50);
}

typedef void (*CanvasProc)(SkCanvas*, bool);

class ClipView : public Sample {
    SkString name() override { return SkString("Clip"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(SK_ColorWHITE);
        canvas->translate(SkIntToScalar(20), SkIntToScalar(20));

        static const CanvasProc gProc[] = {
            show_text, show_thick, show_hair, show_fill
        };

        SkRect r = { 0, 0, SkIntToScalar(W), SkIntToScalar(H) };
        SkPath clipPath;
        r.inset(SK_Scalar1 / 4, SK_Scalar1 / 4);
        clipPath.addRoundRect(r, SkIntToScalar(20), SkIntToScalar(20));

//        clipPath.toggleInverseFillType();

        for (int aa = 0; aa <= 1; ++aa) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(gProc); ++i) {
                canvas->save();
                canvas->clipPath(clipPath, kIntersect_SkClipOp, SkToBool(aa));
//                canvas->drawColor(SK_ColorWHITE);
                gProc[i](canvas, SkToBool(aa));
                canvas->restore();
                canvas->translate(W * SK_Scalar1 * 8 / 7, 0);
            }
            canvas->restore();
            canvas->translate(0, H * SK_Scalar1 * 8 / 7);
        }
    }
};

DEF_SAMPLE( return new ClipView(); )

///////////////////////////////////////////////////////////////////////////////

#include "src/core/SkEdgeClipper.h"

static SkPath clip(const SkPath& path, SkPoint p0, SkPoint p1) {
    SkMatrix mx, inv;
    SkVector v = p1 - p0;
    mx.setAll(v.fX, -v.fY, p0.fX,
              v.fY,  v.fX, p0.fY,
                 0,     0,     1);
    SkAssertResult(mx.invert(&inv));

    SkPath rotated;
    path.transform(inv, &rotated);

    SkScalar big = 1e28f;
    SkRect clip = {-big, 0, big, big };

    struct Rec {
        SkPath  fResult;
        SkPoint fPrev;
    } rec;

    SkEdgeClipper::ClipPath(rotated, clip, false,
                            [](SkEdgeClipper* clipper, bool newCtr, void* ctx) {
        Rec* rec = (Rec*)ctx;

        bool addLineTo = false;
        SkPoint      pts[4];
        SkPath::Verb verb;
        while ((verb = clipper->next(pts)) != SkPath::kDone_Verb) {
            if (newCtr) {
                rec->fResult.moveTo(pts[0]);
                rec->fPrev = pts[0];
                newCtr = false;
            }

            if (addLineTo || pts[0] != rec->fPrev) {
                rec->fResult.lineTo(pts[0]);
            }

            switch (verb) {
                case SkPath::kLine_Verb:
                    rec->fResult.lineTo (pts[1]);
                    rec->fPrev = pts[1];
                    break;
                case SkPath::kQuad_Verb:
                    rec->fResult.quadTo(pts[1], pts[2]);
                    rec->fPrev = pts[2];
                    break;
                case SkPath::kCubic_Verb:
                    rec->fResult.cubicTo(pts[1], pts[2], pts[3]);
                    rec->fPrev = pts[3];
                    break;
                default: break;
            }
            addLineTo = true;
        }
    }, &rec);

    rec.fResult.transform(mx);
    return rec.fResult;
}

static void draw_halfplane(SkCanvas* canvas, SkPoint p0, SkPoint p1) {
    SkVector v = p1 - p0;
    p0 = p0 - v * 1000;
    p1 = p1 + v * 1000;

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawLine(p0, p1, paint);
}

static SkPath make_path() {
    SkRandom rand;
    auto rand_pt = [&rand]() { return SkPoint{rand.nextF() * 400, rand.nextF() * 400}; };

    SkPath path;
    for (int i = 0; i < 4; ++i) {
        path.moveTo(rand_pt()).quadTo(rand_pt(), rand_pt())
            .quadTo(rand_pt(), rand_pt()).lineTo(rand_pt());
    }
    return path;
}

class HalfPlaneView : public Sample {
    SkPoint fPts[2];
    SkPath fPath;

    SkString name() override { return SkString("halfplane"); }

    void onOnceBeforeDraw() override {
        fPts[0] = {0, 0};
        fPts[1] = {3, 2};
        fPath = make_path();
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;

        paint.setColor({0.5f, 0.5f, 0.5f, 1.0f}, nullptr);
        canvas->drawPath(fPath, paint);

        paint.setColor({0, 0, 0, 1}, nullptr);
        canvas->drawPath(clip(fPath, fPts[0], fPts[1]), paint);

        draw_halfplane(canvas, fPts[0], fPts[1]);
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        return new Click;
    }

    bool onClick(Click* click) override {
        fPts[0] = click->fCurr;
        fPts[1] = fPts[0] + SkPoint{3, 2};
        return true;
    }
};
DEF_SAMPLE( return new HalfPlaneView(); )

struct SkHalfPlane {
    SkScalar fA, fB, fC;

    SkScalar operator()(SkScalar x, SkScalar y) const {
        return fA * x + fB * y + fC;
    }

    void twoPts(SkPoint pts[2]) const {
        if (fB) {
            pts[0] = { 0, -fC / fB };
            pts[1] = { 1, (-fC - fA) / fB };
        } else {
            pts[0] = { -fC / fA,        0 };
            pts[1] = { (-fC - fB) / fA, 1 };
        }
    }
};

static void draw_halfplane(SkCanvas* canvas, const SkHalfPlane& p) {
    SkPoint pts[2];
    p.twoPts(pts);
    draw_halfplane(canvas, pts[0], pts[1]);
}

static void compute_half_planes(const SkMatrix& mx, SkScalar W, SkScalar H,
                                SkHalfPlane planes[4]) {
    SkScalar a = mx[0], b = mx[1], c = mx[2],
             d = mx[3], e = mx[4], f = mx[5],
             g = mx[6], h = mx[7], i = mx[8];

    planes[0] = { 2*g - 2*a/W,  2*h - 2*b/W,  2*i - 2*c/W };
    planes[1] = { 2*a/W,        2*b/W,        2*c/W };
    planes[2] = { 2*g - 2*d/H,  2*h - 2*e/H,  2*i - 2*f/H };
    planes[3] = { 2*d/H,        2*e/H,        2*f/H };
}

class HalfPlaneView2 : public Sample {
    SkPoint fPts[4];
    SkPath fPath;

    SkString name() override { return SkString("halfplane2"); }

    void onOnceBeforeDraw() override {
        fPath = make_path();
        SkRect r = fPath.getBounds();
        r.toQuad(fPts);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkMatrix mx;
        {
            SkRect r = fPath.getBounds();
            SkPoint src[4];
            r.toQuad(src);
            mx.setPolyToPoly(src, fPts, 4);
        }

        SkPaint paint;
        canvas->drawPath(fPath, paint);

        canvas->save();
        canvas->concat(mx);
        paint.setColor(0x40FF0000);
        canvas->drawPath(fPath, paint);
        canvas->restore();

        // draw the frame
        paint.setStrokeWidth(10);
        paint.setColor(SK_ColorGREEN);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 4, fPts, paint);

        // draw the half-planes
        SkHalfPlane planes[4];
        compute_half_planes(mx, 400, 400, planes);
        for (auto& p : planes) {
            draw_halfplane(canvas, p);
        }
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        SkScalar r = 8;
        SkRect rect = SkRect::MakeXYWH(x - r, y - r, 2*r, 2*r);
        for (int i = 0; i < 4; ++i) {
            if (rect.contains(fPts[i].fX, fPts[i].fY)) {
                Click* c = new Click;
                c->fMeta.setS32("index", i);
                return c;
            }
        }
        return nullptr;
    }

    bool onClick(Click* click) override {
        int32_t index;
        SkAssertResult(click->fMeta.findS32("index", &index));
        SkASSERT(index >= 0 && index < 4);
        fPts[index] = click->fCurr;
        return true;
    }
};
DEF_SAMPLE( return new HalfPlaneView2(); )
