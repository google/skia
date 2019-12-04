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

struct SkHalfPlane {
    SkScalar fA, fB, fC;

    SkScalar eval(SkScalar x, SkScalar y) const {
        return fA * x + fB * y + fC;
    }
    SkScalar operator()(SkScalar x, SkScalar y) const { return this->eval(x, y); }

    bool twoPts(SkPoint pts[2]) const {
        // normalize plane to help with the perpendicular step, below
        SkScalar len = SkScalarSqrt(fA*fA + fB*fB);
        if (!len) {
            return false;
        }
        SkScalar denom = SkScalarInvert(len);
        SkScalar a = fA * denom;
        SkScalar b = fB * denom;
        SkScalar c = fC * denom;

        // We compute p0 on the half-plane by setting one of the components to 0
        // We compute p1 by stepping from p0 along a perpendicular to the normal
        if (b) {
            pts[0] = { 0, -c / b };
            pts[1] = { b, pts[0].fY - a};
        } else if (a) {
            pts[0] = { -c / a,        0 };
            pts[1] = { pts[0].fX + b, -a };
        } else {
            return false;
        }

        SkASSERT(SkScalarNearlyZero(this->operator()(pts[0].fX, pts[0].fY)));
        SkASSERT(SkScalarNearlyZero(this->operator()(pts[1].fX, pts[1].fY)));
        return true;
    }

    enum Result {
        kAllNegative,
        kAllPositive,
        kMixed
    };
    Result test(const SkRect& bounds) const {
        SkPoint diagMin, diagMax;
        if (fA >= 0) {
            diagMin.fX = bounds.fLeft;
            diagMax.fX = bounds.fRight;
        } else {
            diagMin.fX = bounds.fRight;
            diagMax.fX = bounds.fLeft;
        }
        if (fB >= 0) {
            diagMin.fY = bounds.fTop;
            diagMax.fY = bounds.fBottom;
        } else {
            diagMin.fY = bounds.fBottom;
            diagMax.fY = bounds.fTop;
        }
        SkScalar test = this->eval(diagMin.fX, diagMin.fY);
        SkScalar sign = test*this->eval(diagMax.fX, diagMin.fY);
        if (sign > 0) {
            // the path is either all on one side of the half-plane or the other
            if (test < 0) {
                return kAllNegative;
            } else {
                return kAllPositive;
            }
        }
        return kMixed;
    }
};

#include "src/core/SkEdgeClipper.h"

static void clip(const SkPath& path, SkPoint p0, SkPoint p1, SkPath* clippedPath) {
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
        SkPath* fResult;
        SkPoint fPrev;
    } rec = { clippedPath, {0, 0} };

    SkEdgeClipper::ClipPath(rotated, clip, false,
                            [](SkEdgeClipper* clipper, bool newCtr, void* ctx) {
        Rec* rec = (Rec*)ctx;

        bool addLineTo = false;
        SkPoint      pts[4];
        SkPath::Verb verb;
        while ((verb = clipper->next(pts)) != SkPath::kDone_Verb) {
            if (newCtr) {
                rec->fResult->moveTo(pts[0]);
                rec->fPrev = pts[0];
                newCtr = false;
            }

            if (addLineTo || pts[0] != rec->fPrev) {
                rec->fResult->lineTo(pts[0]);
            }

            switch (verb) {
                case SkPath::kLine_Verb:
                    rec->fResult->lineTo(pts[1]);
                    rec->fPrev = pts[1];
                    break;
                case SkPath::kQuad_Verb:
                    rec->fResult->quadTo(pts[1], pts[2]);
                    rec->fPrev = pts[2];
                    break;
                case SkPath::kCubic_Verb:
                    rec->fResult->cubicTo(pts[1], pts[2], pts[3]);
                    rec->fPrev = pts[3];
                    break;
                default: break;
            }
            addLineTo = true;
        }
    }, &rec);

    rec.fResult->transform(mx);
}

// true means use clippedPath.
// false means there was no clipping -- use the original path
static bool clip(const SkPath& path, const SkHalfPlane& plane, SkPath* clippedPath) {
    switch (plane.test(path.getBounds())) {
        case SkHalfPlane::kAllPositive:
            return false;
        case SkHalfPlane::kMixed: {
            SkPoint pts[2];
            if (plane.twoPts(pts)) {
                clip(path, pts[0], pts[1], clippedPath);
                return true;
            }
        } break;
        default: break; // handled outside of the switch
    }
    // clipped out (or failed)
    clippedPath->reset();
    return true;
}

static void draw_halfplane(SkCanvas* canvas, SkPoint p0, SkPoint p1, SkColor c) {
    SkVector v = p1 - p0;
    p0 = p0 - v * 1000;
    p1 = p1 + v * 1000;

    SkPaint paint;
    paint.setColor(c);
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

        SkPath clippedPath;
        clip(fPath, fPts[0], fPts[1], &clippedPath);
        canvas->drawPath(clippedPath, paint);

        draw_halfplane(canvas, fPts[0], fPts[1], SK_ColorRED);
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

static void draw_halfplane(SkCanvas* canvas, const SkHalfPlane& p, SkColor c) {
    SkPoint pts[2];
    p.twoPts(pts);
    draw_halfplane(canvas, pts[0], pts[1], c);
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
            draw_halfplane(canvas, p, SK_ColorRED);
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

#include "include/core/SkMatrix44.h"
#include "include/utils/Sk3D.h"
#include "tools/Resources.h"

static SkMatrix44 inv(const SkMatrix44& m) {
    SkMatrix44 inverse;
    SkAssertResult(m.invert(&inverse));
    return inverse;
}

#if 0   // Jim's general half-planes math
static void half_planes(const SkMatrix44& m44, SkScalar W, SkScalar H, SkHalfPlane planes[6]) {
    float mx[16];
    m44.asColMajorf(mx);

    SkScalar a = mx[0], b = mx[4], /* c = mx[ 8], */ d = mx[12],
             e = mx[1], f = mx[5], /* g = mx[ 9], */ h = mx[13],
             i = mx[2], j = mx[6], /* k = mx[10], */ l = mx[14],
             m = mx[3], n = mx[7], /* o = mx[11], */ p = mx[15];

    a = 2*a/W - m;  b = 2*b/W - n;  d = 2*d/W - p;
    e = 2*e/H - m;  f = 2*f/H - n;  h = 2*h/H - p;
//    i = 2*i   - m;  j = 2*j   - n;  l = 2*l   - p;

    planes[0] = { m - a, n - b, p - d }; // w - x
    planes[1] = { m + a, n + b, p + d }; // w + x
    planes[2] = { m - e, n - f, p - h }; // w - y
    planes[3] = { m + e, n + f, p + h }; // w + y
    planes[4] = { m - i, n - j, p - l }; // w - z
    planes[5] = { m + i, n + j, p + l }; // w + z
}
#endif

static SkHalfPlane half_plane_w0(const SkMatrix& m) {
    return { m[SkMatrix::kMPersp0], m[SkMatrix::kMPersp1], m[SkMatrix::kMPersp2] - 0.05f };
}

class HalfPlaneView3 : public Sample {
    float   fNear = 0.05f;
    float   fFar = 4;
    float   fAngle = SK_ScalarPI / 4;

    SkPoint3    fEye { 0, 0, 1.0f/tan(fAngle/2) - 1 };
    SkPoint3    fCOA { 0, 0, 0 };
    SkPoint3    fUp  { 0, 1, 0 };

    SkMatrix44  fRot;
    SkPoint3    fTrans;

    SkPath fPath;
    sk_sp<SkShader> fShader;
    bool fShowUnclipped = false;

    SkString name() override { return SkString("halfplane3"); }

    void onOnceBeforeDraw() override {
        fPath = make_path();
        fShader = GetResourceAsImage("images/mandrill_128.png")
                        ->makeShader(SkMatrix::MakeScale(3, 3));
    }

    void rotate(float x, float y, float z) {
        SkMatrix44 r;
        if (x) {
            r.setRotateAboutUnit(1, 0, 0, x);
        } else if (y) {
            r.setRotateAboutUnit(0, 1, 0, y);
        } else {
            r.setRotateAboutUnit(0, 0, 1, z);
        }
        fRot.postConcat(r);
    }

    SkMatrix44 get44() const {
        SkMatrix44  camera,
                    perspective,
                    translate,
                    viewport;

        Sk3Perspective(&perspective, fNear, fFar, fAngle);
        Sk3LookAt(&camera, fEye, fCOA, fUp);
        translate.setTranslate(fTrans.fX, fTrans.fY, fTrans.fZ);
        viewport.setScale(200, 200, 1).postTranslate( 200,  200, 0);

        return viewport * perspective * camera * translate * fRot * inv(viewport);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkMatrix mx = this->get44();

        SkPaint paint;
        paint.setColor({0.75, 0.75, 0.75, 1});
        canvas->drawPath(fPath, paint);

        paint.setShader(fShader);

        if (fShowUnclipped) {
            canvas->save();
            canvas->concat(mx);
            paint.setAlphaf(0.33f);
            canvas->drawPath(fPath, paint);
            paint.setAlphaf(1.f);
            canvas->restore();
        }

        SkHalfPlane hpw = half_plane_w0(mx);

        SkColor planeColor = SK_ColorBLUE;
        SkPath clippedPath, *path = &fPath;
        if (clip(fPath, hpw, &clippedPath)) {
            path = &clippedPath;
            planeColor = SK_ColorRED;
        }
        canvas->save();
        canvas->concat(mx);
        canvas->drawPath(*path, paint);
        canvas->restore();

        draw_halfplane(canvas, hpw, planeColor);
    }

    bool onChar(SkUnichar uni) override {
        float delta = SK_ScalarPI / 30;
        switch (uni) {
            case '8': this->rotate( delta, 0, 0); return true;
            case '2': this->rotate(-delta, 0, 0); return true;
            case '4': this->rotate(0,  delta, 0); return true;
            case '6': this->rotate(0, -delta, 0); return true;
            case '-': this->rotate(0, 0,  delta); return true;
            case '+': this->rotate(0, 0, -delta); return true;

            case 'i': fTrans.fZ += 0.1f; SkDebugf("z %g\n", fTrans.fZ); return true;
            case 'k': fTrans.fZ -= 0.1f; SkDebugf("z %g\n", fTrans.fZ); return true;

            case 'n': fNear += 0.1f; SkDebugf("near %g\n", fNear); return true;
            case 'N': fNear -= 0.1f; SkDebugf("near %g\n", fNear); return true;
            case 'f': fFar  += 0.1f; SkDebugf("far  %g\n", fFar); return true;
            case 'F': fFar  -= 0.1f; SkDebugf("far  %g\n", fFar); return true;

            case 'u': fShowUnclipped = !fShowUnclipped; return true;
            default: break;
        }
        return false;
    }
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        return nullptr;
    }

    bool onClick(Click* click) override {
        return false;
    }
};
DEF_SAMPLE( return new HalfPlaneView3(); )
