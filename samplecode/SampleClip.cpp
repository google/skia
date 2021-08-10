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
#include "include/core/SkPathBuilder.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/core/SkPathPriv.h"
#include "tools/Resources.h"

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

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        canvas->drawRect(r, paint);

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        canvas->drawOval(r, paint);
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

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        canvas->drawRect(r, paint);

        r.setXYWH(rand.nextSScalar1() * W, rand.nextSScalar1() * H,
                  rand.nextUScalar1() * W, rand.nextUScalar1() * H);
        paint.setColor(rand.nextU());
        canvas->drawOval(r, paint);

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
        r.inset(SK_Scalar1 / 4, SK_Scalar1 / 4);
        SkPath clipPath = SkPathBuilder().addRRect(SkRRect::MakeRectXY(r, 20, 20)).detach();

//        clipPath.toggleInverseFillType();

        for (int aa = 0; aa <= 1; ++aa) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(gProc); ++i) {
                canvas->save();
                canvas->clipPath(clipPath, SkClipOp::kIntersect, SkToBool(aa));
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
        SkPathBuilder   fResult;
        SkPoint         fPrev = {0, 0};
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
                    rec->fResult.lineTo(pts[1]);
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

    return rec.fResult.detach().makeTransform(mx);
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
    auto rand_pt = [&rand]() {
        auto x = rand.nextF();
        auto y = rand.nextF();
        return SkPoint{x * 400, y * 400};
    };

    SkPathBuilder path;
    for (int i = 0; i < 4; ++i) {
        SkPoint pts[6];
        for (auto& p : pts) {
            p = rand_pt();
        }
        path.moveTo(pts[0]).quadTo(pts[1], pts[2]).quadTo(pts[3], pts[4]).lineTo(pts[5]);
    }
    return path.detach();
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

static void compute_half_planes(const SkMatrix& mx, SkScalar width, SkScalar height,
                                SkHalfPlane planes[4]) {
    SkScalar a = mx[0], b = mx[1], c = mx[2],
             d = mx[3], e = mx[4], f = mx[5],
             g = mx[6], h = mx[7], i = mx[8];

    planes[0] = { 2*g - 2*a/width,  2*h - 2*b/width,  2*i - 2*c/width };
    planes[1] = { 2*a/width,        2*b/width,        2*c/width };
    planes[2] = { 2*g - 2*d/height, 2*h - 2*e/height, 2*i - 2*f/height };
    planes[3] = { 2*d/height,       2*e/height,       2*f/height };
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

static SkM44 inv(const SkM44& m) {
    SkM44 inverse;
    SkAssertResult(m.invert(&inverse));
    return inverse;
}

static SkHalfPlane half_plane_w0(const SkMatrix& m) {
    return { m[SkMatrix::kMPersp0], m[SkMatrix::kMPersp1], m[SkMatrix::kMPersp2] - 0.05f };
}

class SampleCameraView : public Sample {
    float   fNear = 0.05f;
    float   fFar = 4;
    float   fAngle = SK_ScalarPI / 4;

    SkV3    fEye { 0, 0, 1.0f/tan(fAngle/2) - 1 };
    SkV3    fCOA { 0, 0, 0 };
    SkV3    fUp  { 0, 1, 0 };

    SkM44  fRot;
    SkV3   fTrans;

    void rotate(float x, float y, float z) {
        SkM44 r;
        if (x) {
            r.setRotateUnit({1, 0, 0}, x);
        } else if (y) {
            r.setRotateUnit({0, 1, 0}, y);
        } else {
            r.setRotateUnit({0, 0, 1}, z);
        }
        fRot = r * fRot;
    }

public:
    SkM44 get44(const SkRect& r) const {
        SkScalar w = r.width();
        SkScalar h = r.height();

        SkM44 camera = SkM44::LookAt(fEye, fCOA, fUp),
              perspective = SkM44::Perspective(fNear, fFar, fAngle),
              translate = SkM44::Translate(fTrans.x, fTrans.y, fTrans.z),
              viewport = SkM44::Translate(r.centerX(), r.centerY(), 0) *
                         SkM44::Scale(w*0.5f, h*0.5f, 1);

        return viewport * perspective * camera * translate * fRot * inv(viewport);
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

            case 'i': fTrans.z += 0.1f; SkDebugf("z %g\n", fTrans.z); return true;
            case 'k': fTrans.z -= 0.1f; SkDebugf("z %g\n", fTrans.z); return true;

            case 'n': fNear += 0.1f; SkDebugf("near %g\n", fNear); return true;
            case 'N': fNear -= 0.1f; SkDebugf("near %g\n", fNear); return true;
            case 'f': fFar  += 0.1f; SkDebugf("far  %g\n", fFar); return true;
            case 'F': fFar  -= 0.1f; SkDebugf("far  %g\n", fFar); return true;
            default: break;
        }
        return false;
    }
};

class HalfPlaneView3 : public SampleCameraView {
    SkPath fPath;
    sk_sp<SkShader> fShader;
    bool fShowUnclipped = false;

    SkString name() override { return SkString("halfplane3"); }

    void onOnceBeforeDraw() override {
        fPath = make_path();
        fShader = GetResourceAsImage("images/mandrill_128.png")
                        ->makeShader(SkSamplingOptions(), SkMatrix::Scale(3, 3));
    }

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case 'u': fShowUnclipped = !fShowUnclipped; return true;
            default: break;
        }
        return this->SampleCameraView::onChar(uni);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkM44 mx = this->get44({0, 0, 400, 400});

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


        SkColor planeColor = SK_ColorBLUE;
        SkPath clippedPath, *path = &fPath;
        if (SkPathPriv::PerspectiveClip(fPath, mx.asM33(), &clippedPath)) {
            path = &clippedPath;
            planeColor = SK_ColorRED;
        }
        canvas->save();
        canvas->concat(mx);
        canvas->drawPath(*path, paint);
        canvas->restore();

        SkHalfPlane hpw = half_plane_w0(mx.asM33());
        draw_halfplane(canvas, hpw, planeColor);
    }
};
DEF_SAMPLE( return new HalfPlaneView3(); )

class HalfPlaneCoons : public SampleCameraView {
    SkPoint fPatch[12];
    SkColor fColors[4] = { SK_ColorRED, SK_ColorGREEN, SK_ColorBLUE, SK_ColorBLACK };
    SkPoint fTex[4]    = {{0, 0}, {256, 0}, {256, 256}, {0, 256}};
    sk_sp<SkShader> fShader;

    bool fShowHandles = false;
    bool fShowSkeleton = false;
    bool fShowTex = false;

    SkString name() override { return SkString("halfplane-coons"); }

    void onOnceBeforeDraw() override {
        fPatch[0] = {   0, 0 };
        fPatch[1] = { 100, 0 };
        fPatch[2] = { 200, 0 };
        fPatch[3] = { 300, 0 };
        fPatch[4] = { 300, 100 };
        fPatch[5] = { 300, 200 };
        fPatch[6] = { 300, 300 };
        fPatch[7] = { 200, 300 };
        fPatch[8] = { 100, 300 };
        fPatch[9] = {   0, 300 };
        fPatch[10] = {  0, 200 };
        fPatch[11] = {  0, 100 };

        fShader = GetResourceAsImage("images/mandrill_256.png")->makeShader(SkSamplingOptions());
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;

        canvas->save();
        canvas->concat(this->get44({0, 0, 300, 300}));

        const SkPoint* tex = nullptr;
        const SkColor* col = nullptr;
        if (!fShowSkeleton) {
            if (fShowTex) {
                paint.setShader(fShader);
                tex = fTex;
            } else {
                col = fColors;
            }
        }
        canvas->drawPatch(fPatch, col, tex, SkBlendMode::kSrc, paint);
        paint.setShader(nullptr);

        if (fShowHandles) {
            paint.setAntiAlias(true);
            paint.setStrokeCap(SkPaint::kRound_Cap);
            paint.setStrokeWidth(8);
            canvas->drawPoints(SkCanvas::kPoints_PointMode, 12, fPatch, paint);
            paint.setColor(SK_ColorWHITE);
            paint.setStrokeWidth(6);
            canvas->drawPoints(SkCanvas::kPoints_PointMode, 12, fPatch, paint);
        }

        canvas->restore();
    }

    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        auto dist = [](SkPoint a, SkPoint b) { return (b - a).length(); };

        const float tol = 15;
        for (int i = 0; i < 12; ++i) {
            if (dist({x,y}, fPatch[i]) <= tol) {
                return new Click([this, i](Click* c) {
                    fPatch[i] = c->fCurr;
                    return true;
                });
            }
        }
        return nullptr;
    }

    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case 'h': fShowHandles = !fShowHandles; return true;
            case 'k': fShowSkeleton = !fShowSkeleton; return true;
            case 't': fShowTex = !fShowTex; return true;
            default: break;
        }
        return this->SampleCameraView::onChar(uni);
    }

};
DEF_SAMPLE( return new HalfPlaneCoons(); )
