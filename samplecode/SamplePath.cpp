/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFont.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkParsePath.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"
#include "tools/timer/TimeUtils.h"

#include "src/core/SkGeometry.h"

#include <stdlib.h>

// http://code.google.com/p/skia/issues/detail?id=32
static void test_cubic() {
    SkPoint src[4] = {
        { 556.25000f, 523.03003f },
        { 556.23999f, 522.96002f },
        { 556.21997f, 522.89001f },
        { 556.21997f, 522.82001f }
    };
    SkPoint dst[11];
    dst[10].set(42, -42);   // one past the end, that we don't clobber these
    SkScalar tval[] = { 0.33333334f, 0.99999994f };

    SkChopCubicAt(src, dst, tval, 2);

#if 0
    for (int i = 0; i < 11; i++) {
        SkDebugf("--- %d [%g %g]\n", i, dst[i].fX, dst[i].fY);
    }
#endif
}

static void test_cubic2() {
    const char* str = "M2242 -590088L-377758 9.94099e+07L-377758 9.94099e+07L2242 -590088Z";
    SkPath path;
    SkParsePath::FromSVGString(str, &path);

    {
        SkRect r = path.getBounds();
        SkIRect ir;
        r.round(&ir);
        SkDebugf("[%g %g %g %g] [%x %x %x %x]\n",
                SkScalarToDouble(r.fLeft), SkScalarToDouble(r.fTop),
                SkScalarToDouble(r.fRight), SkScalarToDouble(r.fBottom),
                ir.fLeft, ir.fTop, ir.fRight, ir.fBottom);
    }

    SkBitmap bitmap;
    bitmap.allocN32Pixels(300, 200);

    SkCanvas canvas(bitmap);
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas.drawPath(path, paint);
}

class PathView : public Sample {
    SkScalar fPrevSecs;
public:
    SkScalar fDStroke, fStroke, fMinStroke, fMaxStroke;
    SkPath fPath[6];
    bool fShowHairline;
    bool fOnce;

    PathView() {
        fPrevSecs = 0;
        fOnce = false;
    }

    void init() {
        if (fOnce) {
            return;
        }
        fOnce = true;

        test_cubic();
        test_cubic2();

        fShowHairline = false;

        fDStroke = 1;
        fStroke = 10;
        fMinStroke = 10;
        fMaxStroke = 180;

        const SkScalar V = 85;

        fPath[0].moveTo(40, 70);
        fPath[0].lineTo(70, 70 + SK_ScalarHalf);
        fPath[0].lineTo(110, 70);

        fPath[1].moveTo(40, 70);
        fPath[1].lineTo(70, 70 - SK_ScalarHalf);
        fPath[1].lineTo(110, 70);

        fPath[2].moveTo(V, V);
        fPath[2].lineTo(50, V);
        fPath[2].lineTo(50, 50);

        fPath[3].moveTo(50, 50);
        fPath[3].lineTo(50, V);
        fPath[3].lineTo(V, V);

        fPath[4].moveTo(50, 50);
        fPath[4].lineTo(50, V);
        fPath[4].lineTo(52, 50);

        fPath[5].moveTo(52, 50);
        fPath[5].lineTo(50, V);
        fPath[5].lineTo(50, 50);

        this->setBGColor(0xFFDDDDDD);
    }

protected:
    SkString name() override { return SkString("Paths"); }

    void drawPath(SkCanvas* canvas, const SkPath& path, SkPaint::Join j) {
        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeJoin(j);
        paint.setStrokeWidth(fStroke);

        if (fShowHairline) {
            SkPath  fill;

            paint.getFillPath(path, &fill);
            paint.setStrokeWidth(0);
            canvas->drawPath(fill, paint);
        } else {
            canvas->drawPath(path, paint);
        }

        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(0);
        canvas->drawPath(path, paint);
    }

    void onDrawContent(SkCanvas* canvas) override {
        this->init();
        canvas->translate(50, 50);

        static const SkPaint::Join gJoins[] = {
            SkPaint::kBevel_Join,
            SkPaint::kMiter_Join,
            SkPaint::kRound_Join
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(gJoins); i++) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(fPath); j++) {
                this->drawPath(canvas, fPath[j], gJoins[i]);
                canvas->translate(200, 0);
            }
            canvas->restore();

            canvas->translate(0, 200);
        }
    }

    bool onAnimate(double nanos) override {
        SkScalar currSecs = TimeUtils::Scaled(1e-9 * nanos, 100);
        SkScalar delta = currSecs - fPrevSecs;
        fPrevSecs = currSecs;

        fStroke += fDStroke * delta;
        if (fStroke > fMaxStroke || fStroke < fMinStroke) {
            fDStroke = -fDStroke;
        }
        return true;
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, ModifierKey modi) override {
        fShowHairline = !fShowHairline;
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new PathView; )

//////////////////////////////////////////////////////////////////////////////

#include "include/effects/SkCornerPathEffect.h"
#include "include/utils/SkRandom.h"

class ArcToView : public Sample {
    bool fDoFrame, fDoCorner, fDoConic;
    SkPaint fPtsPaint, fSkeletonPaint, fCornerPaint;
public:
    enum {
        N = 4
    };
    SkPoint fPts[N];

    ArcToView()
        : fDoFrame(false), fDoCorner(false), fDoConic(false)
    {
        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            fPts[i].fX = 20 + rand.nextUScalar1() * 640;
            fPts[i].fY = 20 + rand.nextUScalar1() * 480;
        }

        const SkScalar rad = 50;

        fPtsPaint.setAntiAlias(true);
        fPtsPaint.setStrokeWidth(15);
        fPtsPaint.setStrokeCap(SkPaint::kRound_Cap);

        fCornerPaint.setAntiAlias(true);
        fCornerPaint.setStyle(SkPaint::kStroke_Style);
        fCornerPaint.setStrokeWidth(13);
        fCornerPaint.setColor(SK_ColorGREEN);
        fCornerPaint.setPathEffect(SkCornerPathEffect::Make(rad*2));

        fSkeletonPaint.setAntiAlias(true);
        fSkeletonPaint.setStyle(SkPaint::kStroke_Style);
        fSkeletonPaint.setColor(SK_ColorRED);
    }

    void toggle(bool& value) {
        value = !value;
    }

protected:
    SkString name() override { return SkString("ArcTo"); }

    bool onChar(SkUnichar uni) override {
            switch (uni) {
                case '1': this->toggle(fDoFrame); return true;
                case '2': this->toggle(fDoCorner); return true;
                case '3': this->toggle(fDoConic); return true;
                default: break;
            }
            return false;
    }

    void makePath(SkPath* path) {
        path->moveTo(fPts[0]);
        for (int i = 1; i < N; ++i) {
            path->lineTo(fPts[i]);
        }
        if (!fDoFrame) {
            path->close();
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawPoints(SkCanvas::kPoints_PointMode, N, fPts, fPtsPaint);

        SkPath path;
        this->makePath(&path);

        if (fDoCorner) {
            canvas->drawPath(path, fCornerPaint);
        }

        canvas->drawPath(path, fSkeletonPaint);
    }

    bool onClick(Click* click) override {
        int32_t index;
        if (click->fMeta.findS32("index", &index)) {
            SkASSERT((unsigned)index < N);
            fPts[index] = click->fCurr;
            return true;
        }
        return false;
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, ModifierKey modi) override {
        const SkScalar tol = 4;
        const SkRect r = SkRect::MakeXYWH(x - tol, y - tol, tol * 2, tol * 2);
        for (int i = 0; i < N; ++i) {
            if (r.intersects(SkRect::MakeXYWH(fPts[i].fX, fPts[i].fY, 1, 1))) {
                Click* click = new Click(this);
                click->fMeta.setS32("index", i);
                return click;
            }
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new ArcToView; )

/////////////

class FatStroke : public Sample {
    bool fClosed, fShowStroke, fShowHidden, fShowSkeleton;
    int  fJoinType, fCapType;
    float fWidth = 30;
    SkPaint fPtsPaint, fHiddenPaint, fSkeletonPaint, fStrokePaint;
public:
    enum {
        N = 4
    };
    SkPoint fPts[N];

    FatStroke() : fClosed(false), fShowStroke(true), fShowHidden(false), fShowSkeleton(true),
                  fJoinType(0), fCapType(0)
    {
        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            fPts[i].fX = 20 + rand.nextUScalar1() * 640;
            fPts[i].fY = 20 + rand.nextUScalar1() * 480;
        }

        fPtsPaint.setAntiAlias(true);
        fPtsPaint.setStrokeWidth(10);
        fPtsPaint.setStrokeCap(SkPaint::kRound_Cap);

        fHiddenPaint.setAntiAlias(true);
        fHiddenPaint.setStyle(SkPaint::kStroke_Style);
        fHiddenPaint.setColor(0xFF0000FF);

        fStrokePaint.setAntiAlias(true);
        fStrokePaint.setStyle(SkPaint::kStroke_Style);
        fStrokePaint.setStrokeWidth(50);
        fStrokePaint.setColor(0x8000FF00);

        fSkeletonPaint.setAntiAlias(true);
        fSkeletonPaint.setStyle(SkPaint::kStroke_Style);
        fSkeletonPaint.setColor(SK_ColorRED);
    }

    void toggle(bool& value) {
        value = !value;
    }

    void toggle3(int& value) {
        value = (value + 1) % 3;
    }

protected:
    SkString name() override { return SkString("FatStroke"); }

    bool onChar(SkUnichar uni) override {
            switch (uni) {
                case '1': this->toggle(fShowSkeleton); return true;
                case '2': this->toggle(fShowStroke); return true;
                case '3': this->toggle(fShowHidden); return true;
                case '4': this->toggle3(fJoinType); return true;
                case '5': this->toggle3(fCapType); return true;
                case '6': this->toggle(fClosed); return true;
                case '-': fWidth -= 5; return true;
                case '=': fWidth += 5; return true;
                default: break;
            }
            return false;
    }

    void makePath(SkPath* path) {
        path->moveTo(fPts[0]);
        for (int i = 1; i < N; ++i) {
            path->lineTo(fPts[i]);
        }
        if (fClosed) {
            path->close();
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(0xFFEEEEEE);

        SkPath path;
        this->makePath(&path);

        fStrokePaint.setStrokeWidth(fWidth);
        fStrokePaint.setStrokeJoin((SkPaint::Join)fJoinType);
        fStrokePaint.setStrokeCap((SkPaint::Cap)fCapType);

        if (fShowStroke) {
            canvas->drawPath(path, fStrokePaint);
        }
        if (fShowHidden) {
            SkPath hidden;
            fStrokePaint.getFillPath(path, &hidden);
            canvas->drawPath(hidden, fHiddenPaint);
        }
        if (fShowSkeleton) {
            canvas->drawPath(path, fSkeletonPaint);
        }
        canvas->drawPoints(SkCanvas::kPoints_PointMode, N, fPts, fPtsPaint);
    }

    bool onClick(Click* click) override {
        int32_t index;
        if (click->fMeta.findS32("index", &index)) {
            SkASSERT((unsigned)index < N);
            fPts[index] = click->fCurr;
            return true;
        }
        return false;
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, ModifierKey modi) override {
        const SkScalar tol = 4;
        const SkRect r = SkRect::MakeXYWH(x - tol, y - tol, tol * 2, tol * 2);
        for (int i = 0; i < N; ++i) {
            if (r.intersects(SkRect::MakeXYWH(fPts[i].fX, fPts[i].fY, 1, 1))) {
                Click* click = new Click(this);
                click->fMeta.setS32("index", i);
                return click;
            }
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new FatStroke; )

static int compute_parallel_to_base(const SkPoint pts[4], SkScalar t[2]) {
    // F = At^3 + Bt^2 + Ct + D
    SkVector A = pts[3] - pts[0] + (pts[1] - pts[2]) * 3.0f;
    SkVector B = (pts[0] - pts[1] - pts[1] + pts[2]) * 3.0f;
    SkVector C = (pts[1] - pts[0]) * 3.0f;
    SkVector DA = pts[3] - pts[0];

    // F' = 3At^2 + 2Bt + C
    SkScalar a = 3 * A.cross(DA);
    SkScalar b = 2 * B.cross(DA);
    SkScalar c = C.cross(DA);

    int n = SkFindUnitQuadRoots(a, b, c, t);
    SkString str;
    for (int i = 0; i < n; ++i) {
        str.appendf(" %g", t[i]);
    }
    SkDebugf("roots %s\n", str.c_str());
    return n;
}

class CubicCurve : public Sample {
public:
    enum {
        N = 4
    };
    SkPoint fPts[N];

    CubicCurve() {
        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            fPts[i].fX = 20 + rand.nextUScalar1() * 640;
            fPts[i].fY = 20 + rand.nextUScalar1() * 480;
        }
    }

protected:
    SkString name() override { return SkString("CubicCurve"); }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);

        {
            SkPath path;
            path.moveTo(fPts[0]);
            path.cubicTo(fPts[1], fPts[2], fPts[3]);
            paint.setStyle(SkPaint::kStroke_Style);
            canvas->drawPath(path, paint);
        }

        {
            paint.setColor(SK_ColorRED);
            SkScalar t[2];
            int n = compute_parallel_to_base(fPts, t);
            SkPoint loc;
            SkVector tan;
            for (int i = 0; i < n; ++i) {
                SkEvalCubicAt(fPts, t[i], &loc, &tan, nullptr);
                tan.setLength(30);
                canvas->drawLine(loc - tan, loc + tan, paint);
            }
            paint.setStrokeWidth(0.5f);
            canvas->drawLine(fPts[0], fPts[3], paint);

            paint.setColor(SK_ColorBLUE);
            paint.setStrokeWidth(6);
            SkEvalCubicAt(fPts, 0.5f, &loc, nullptr, nullptr);
            canvas->drawPoint(loc, paint);

            paint.setColor(0xFF008800);
            SkEvalCubicAt(fPts, 1.0f/3, &loc, nullptr, nullptr);
            canvas->drawPoint(loc, paint);
            SkEvalCubicAt(fPts, 2.0f/3, &loc, nullptr, nullptr);
            canvas->drawPoint(loc, paint);

       //     n = SkFindCubicInflections(fPts, t);
       //     printf("inflections %d %g %g\n", n, t[0], t[1]);
        }

        {
            paint.setStyle(SkPaint::kFill_Style);
            paint.setColor(SK_ColorRED);
            for (SkPoint p : fPts) {
                canvas->drawCircle(p.fX, p.fY, 8, paint);
            }
        }
    }

    bool onClick(Click* click) override {
        int32_t index;
        if (click->fMeta.findS32("index", &index)) {
            SkASSERT((unsigned)index < N);
            fPts[index] = click->fCurr;
            return true;
        }
        return false;
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, ModifierKey modi) override {
        const SkScalar tol = 8;
        const SkRect r = SkRect::MakeXYWH(x - tol, y - tol, tol * 2, tol * 2);
        for (int i = 0; i < N; ++i) {
            if (r.intersects(SkRect::MakeXYWH(fPts[i].fX, fPts[i].fY, 1, 1))) {
                Click* click = new Click(this);
                click->fMeta.setS32("index", i);
                return click;
            }
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new CubicCurve; )

static SkPoint lerp(SkPoint a, SkPoint b, float t) {
    return a * (1 - t) + b * t;
}

static int find_max_deviation_cubic(const SkPoint src[4], SkScalar ts[2]) {
    // deviation = F' x (d - a) == 0, solve for t(s)
    // F = At^3 + Bt^2 + Ct + D
    // F' = 3At^2 + 2Bt + C
    // Z = d - a
    // F' x Z = 3(A x Z)t^2 + 2(B x Z)t + (C x Z)
    //
    SkVector A = src[3] + (src[1] - src[2]) * 3 - src[0];
    SkVector B = (src[2] - src[1] - src[1] + src[0]) * 3;
    SkVector C = (src[1] - src[0]) * 3;
    SkVector Z = src[3] - src[0];
    // now forumlate the quadratic coefficients we need to solve for t : F' x Z
    return SkFindUnitQuadRoots(3 * A.cross(Z), 2 * B.cross(Z), C.cross(Z), ts);
}

class CubicCurve2 : public Sample {
public:
    enum {
        N = 7
    };
    SkPoint fPts[N];
    SkPoint* fQuad = fPts + 4;
    SkScalar fT = 0.5f;
    bool fShowSub = false;
    bool fShowFlatness = false;
    SkScalar fScale = 0.75;

    CubicCurve2() {
        fPts[0] = { 90, 300 };
        fPts[1] = { 30, 60 };
        fPts[2] = { 250, 30 };
        fPts[3] = { 350, 200 };

        fQuad[0] = fPts[0] + SkVector{ 300, 0};
        fQuad[1] = fPts[1] + SkVector{ 300, 0};
        fQuad[2] = fPts[2] + SkVector{ 300, 0};
    }

protected:
    SkString name() override { return SkString("CubicCurve2"); }

    bool onChar(SkUnichar uni) override {
            switch (uni) {
                case 's': fShowSub = !fShowSub; break;
                case 'f': fShowFlatness = !fShowFlatness; break;
                case '-': fT -= 1.0f / 32; break;
                case '=': fT += 1.0f / 32; break;
                default: return false;
            }
            fT = std::min(1.0f, std::max(0.0f, fT));
            return true;
    }

    void showFrame(SkCanvas* canvas, const SkPoint pts[], int count, const SkPaint& p) {
        SkPaint paint(p);
        SkPoint storage[3 + 2 + 1];
        SkPoint* tmp = storage;
        const SkPoint* prev = pts;
        int n = count;
        for (int n = count; n > 0; --n) {
            for (int i = 0; i < n; ++i) {
                canvas->drawLine(prev[i], prev[i+1], paint);
                tmp[i] = lerp(prev[i], prev[i+1], fT);
            }
            prev = tmp;
            tmp += n;
        }

        paint.setColor(SK_ColorBLUE);
        paint.setStyle(SkPaint::kFill_Style);
        n = tmp - storage;
        for (int i = 0; i < n; ++i) {
            canvas->drawCircle(storage[i].fX, storage[i].fY, 4, paint);
        }
    }

    void showFlattness(SkCanvas* canvas) {
        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setAntiAlias(true);

        SkPaint paint2(paint);
        paint2.setColor(0xFF008800);

        paint.setColor(0xFF888888);
        canvas->drawLine(fPts[0], fPts[3], paint);
        canvas->drawLine(fQuad[0], fQuad[2], paint);

        paint.setColor(0xFF0000FF);
        SkPoint pts[2];
        pts[0] = (fQuad[0] + fQuad[1] + fQuad[1] + fQuad[2])*0.25;
        pts[1] = (fQuad[0] + fQuad[2]) * 0.5;
        canvas->drawLine(pts[0], pts[1], paint);

        // cubic

        SkVector v0 = (fPts[0] - fPts[1] - fPts[1] + fPts[2]) * fScale;
        SkVector v1 = (fPts[1] - fPts[2] - fPts[2] + fPts[3]) * fScale;
        SkVector v = (v0 + v1) * 0.5f;

        SkPoint anchor;
        SkScalar ts[2];
        int n = find_max_deviation_cubic(fPts, ts);
        if (n > 0) {
            SkEvalCubicAt(fPts, ts[0], &anchor, nullptr, nullptr);
            canvas->drawLine(anchor, anchor + v, paint2);
            canvas->drawLine(anchor, anchor + v0, paint);
            if (n == 2) {
                SkEvalCubicAt(fPts, ts[1], &anchor, nullptr, nullptr);
                canvas->drawLine(anchor, anchor + v, paint2);
            }
            canvas->drawLine(anchor, anchor + v1, paint);
        }
        // not sure we can get here
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);

        {
            paint.setStyle(SkPaint::kStroke_Style);
            SkPath path;
            path.moveTo(fPts[0]);
            path.cubicTo(fPts[1], fPts[2], fPts[3]);
            path.moveTo(fQuad[0]);
            path.quadTo(fQuad[1], fQuad[2]);
            canvas->drawPath(path, paint);
        }

        if (fShowSub) {
            paint.setColor(SK_ColorRED);
            paint.setStrokeWidth(1.7f);
            this->showFrame(canvas, fPts, 3, paint);
            this->showFrame(canvas, fQuad, 2, paint);

            paint.setColor(SK_ColorBLACK);
            paint.setStyle(SkPaint::kFill_Style);
            SkFont font(nullptr, 20);
            canvas->drawString(SkStringPrintf("t = %g", fT), 20, 20, font, paint);
        }

        if (fShowFlatness) {
            this->showFlattness(canvas);
        }

        paint.setStyle(SkPaint::kFill_Style);
        paint.setColor(SK_ColorRED);
        for (SkPoint p : fPts) {
            canvas->drawCircle(p.fX, p.fY, 7, paint);
        }

        {
            SkScalar ts[2];
            int n = SkFindCubicInflections(fPts, ts);
            for (int i = 0; i < n; ++i) {
                SkPoint p;
                SkEvalCubicAt(fPts, ts[i], &p, nullptr, nullptr);
                canvas->drawCircle(p.fX, p.fY, 3, paint);
            }
        }

    }

    bool onClick(Click* click) override {
        int32_t index;
        if (click->fMeta.findS32("index", &index)) {
            SkASSERT((unsigned)index < N);
            fPts[index] = click->fCurr;
            return true;
        }
        return false;
    }

    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, ModifierKey modi) override {
        const SkScalar tol = 8;
        const SkRect r = SkRect::MakeXYWH(x - tol, y - tol, tol * 2, tol * 2);
        for (int i = 0; i < N; ++i) {
            if (r.intersects(SkRect::MakeXYWH(fPts[i].fX, fPts[i].fY, 1, 1))) {
                Click* click = new Click(this);
                click->fMeta.setS32("index", i);
                return click;
            }
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    typedef Sample INHERITED;
};
DEF_SAMPLE( return new CubicCurve2; )

