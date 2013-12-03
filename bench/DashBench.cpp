
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkDashPathEffect.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkString.h"
#include "SkTDArray.h"


/*
 *  Cases to consider:
 *
 *  1. antialiasing on/off (esp. width <= 1)
 *  2. strokewidth == 0, 1, 2
 *  3. hline, vline, diagonal, rect, oval
 *  4. dots [1,1] ([N,N] where N=strokeWidth?) or arbitrary (e.g. [2,1] or [1,2,3,2])
 */
static void path_hline(SkPath* path) {
    path->moveTo(SkIntToScalar(10), SkIntToScalar(10));
    path->lineTo(SkIntToScalar(600), SkIntToScalar(10));
}

class DashBench : public SkBenchmark {
protected:
    SkString            fName;
    SkTDArray<SkScalar> fIntervals;
    int                 fWidth;
    SkPoint             fPts[2];
    bool                fDoClip;

public:
    DashBench(const SkScalar intervals[], int count, int width,
              bool doClip = false)  {
        fIntervals.append(count, intervals);
        for (int i = 0; i < count; ++i) {
            fIntervals[i] *= width;
        }
        fWidth = width;
        fName.printf("dash_%d_%s", width, doClip ? "clipped" : "noclip");
        fDoClip = doClip;

        fPts[0].set(SkIntToScalar(10), SkIntToScalar(10));
        fPts[1].set(SkIntToScalar(600), SkIntToScalar(10));
    }

    virtual void makePath(SkPath* path) {
        path_hline(path);
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        this->setupPaint(&paint);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(SkIntToScalar(fWidth));
        paint.setAntiAlias(false);

        SkPath path;
        this->makePath(&path);

        paint.setPathEffect(new SkDashPathEffect(fIntervals.begin(),
                                                 fIntervals.count(), 0))->unref();

        if (fDoClip) {
            SkRect r = path.getBounds();
            r.inset(-SkIntToScalar(20), -SkIntToScalar(20));
            // now move it so we don't intersect
            r.offset(0, r.height() * 3 / 2);
            canvas->clipRect(r);
        }

        this->handlePath(canvas, path, paint, loops);
    }

    virtual void handlePath(SkCanvas* canvas, const SkPath& path,
                            const SkPaint& paint, int N) {
        for (int i = 0; i < N; ++i) {
//            canvas->drawPoints(SkCanvas::kLines_PointMode, 2, fPts, paint);
            canvas->drawPath(path, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class RectDashBench : public DashBench {
public:
    RectDashBench(const SkScalar intervals[], int count, int width)
    : INHERITED(intervals, count, width) {
        fName.append("_rect");
    }

protected:
    virtual void handlePath(SkCanvas* canvas, const SkPath& path,
                            const SkPaint& paint, int N) SK_OVERRIDE {
        SkPoint pts[2];
        if (!path.isLine(pts) || pts[0].fY != pts[1].fY) {
            this->INHERITED::handlePath(canvas, path, paint, N);
        } else {
            SkRect rect;
            rect.fLeft = pts[0].fX;
            rect.fTop = pts[0].fY - paint.getStrokeWidth() / 2;
            rect.fRight = rect.fLeft + SkIntToScalar(fWidth);
            rect.fBottom = rect.fTop + paint.getStrokeWidth();

            SkPaint p(paint);
            p.setStyle(SkPaint::kFill_Style);
            p.setPathEffect(NULL);

            int count = SkScalarRoundToInt((pts[1].fX - pts[0].fX) / (2*fWidth));
            SkScalar dx = SkIntToScalar(2 * fWidth);

            for (int i = 0; i < N*10; ++i) {
                SkRect r = rect;
                for (int j = 0; j < count; ++j) {
                    canvas->drawRect(r, p);
                    r.offset(dx, 0);
                }
            }
        }
    }

private:
    typedef DashBench INHERITED;
};

static void make_unit_star(SkPath* path, int n) {
    SkScalar rad = -SK_ScalarPI / 2;
    const SkScalar drad = (n >> 1) * SK_ScalarPI * 2 / n;

    path->moveTo(0, -SK_Scalar1);
    for (int i = 1; i < n; i++) {
        rad += drad;
        SkScalar cosV, sinV = SkScalarSinCos(rad, &cosV);
        path->lineTo(cosV, sinV);
    }
    path->close();
}

static void make_poly(SkPath* path) {
    make_unit_star(path, 9);
    SkMatrix matrix;
    matrix.setScale(SkIntToScalar(100), SkIntToScalar(100));
    path->transform(matrix);
}

static void make_quad(SkPath* path) {
    SkScalar x0 = SkIntToScalar(10);
    SkScalar y0 = SkIntToScalar(10);
    path->moveTo(x0, y0);
    path->quadTo(x0,                    y0 + 400 * SK_Scalar1,
                 x0 + 600 * SK_Scalar1, y0 + 400 * SK_Scalar1);
}

static void make_cubic(SkPath* path) {
    SkScalar x0 = SkIntToScalar(10);
    SkScalar y0 = SkIntToScalar(10);
    path->moveTo(x0, y0);
    path->cubicTo(x0,                    y0 + 400 * SK_Scalar1,
                  x0 + 600 * SK_Scalar1, y0 + 400 * SK_Scalar1,
                  x0 + 600 * SK_Scalar1, y0);
}

class MakeDashBench : public SkBenchmark {
    SkString fName;
    SkPath   fPath;
    SkAutoTUnref<SkPathEffect> fPE;

public:
    MakeDashBench(void (*proc)(SkPath*), const char name[])  {
        fName.printf("makedash_%s", name);
        proc(&fPath);

        SkScalar vals[] = { SkIntToScalar(4), SkIntToScalar(4) };
        fPE.reset(new SkDashPathEffect(vals, 2, 0));
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas*) SK_OVERRIDE {
        SkPath dst;
        for (int i = 0; i < loops; ++i) {
            SkStrokeRec rec(SkStrokeRec::kHairline_InitStyle);

            fPE->filterPath(&dst, fPath, &rec, NULL);
            dst.rewind();
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

/*
 *  We try to special case square dashes (intervals are equal to strokewidth).
 */
class DashLineBench : public SkBenchmark {
    SkString fName;
    SkScalar fStrokeWidth;
    bool     fIsRound;
    SkAutoTUnref<SkPathEffect> fPE;

public:
    DashLineBench(SkScalar width, bool isRound)  {
        fName.printf("dashline_%g_%s", SkScalarToFloat(width), isRound ? "circle" : "square");
        fStrokeWidth = width;
        fIsRound = isRound;

        SkScalar vals[] = { SK_Scalar1, SK_Scalar1 };
        fPE.reset(new SkDashPathEffect(vals, 2, 0));
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkPaint paint;
        this->setupPaint(&paint);
        paint.setStrokeWidth(fStrokeWidth);
        paint.setStrokeCap(fIsRound ? SkPaint::kRound_Cap : SkPaint::kSquare_Cap);
        paint.setPathEffect(fPE);
        for (int i = 0; i < loops; ++i) {
            canvas->drawLine(10 * SK_Scalar1, 10 * SK_Scalar1,
                             640 * SK_Scalar1, 10 * SK_Scalar1, paint);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

class DrawPointsDashingBench : public SkBenchmark {
    SkString fName;
    int      fStrokeWidth;
    bool     fDoAA;

    SkAutoTUnref<SkPathEffect> fPathEffect;

public:
    DrawPointsDashingBench(int dashLength, int strokeWidth, bool doAA)
         {
        fName.printf("drawpointsdash_%d_%d%s", dashLength, strokeWidth, doAA ? "_aa" : "_bw");
        fStrokeWidth = strokeWidth;
        fDoAA = doAA;

        SkScalar vals[] = { SkIntToScalar(dashLength), SkIntToScalar(dashLength) };
        fPathEffect.reset(new SkDashPathEffect(vals, 2, SK_Scalar1, false));
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkPaint p;
        this->setupPaint(&p);
        p.setColor(SK_ColorBLACK);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(SkIntToScalar(fStrokeWidth));
        p.setPathEffect(fPathEffect);
        p.setAntiAlias(fDoAA);

        SkPoint pts[2] = {
            { SkIntToScalar(10), 0 },
            { SkIntToScalar(640), 0 }
        };

        for (int i = 0; i < loops; ++i) {
            pts[0].fY = pts[1].fY = SkIntToScalar(i % 480);
            canvas->drawPoints(SkCanvas::kLines_PointMode, 2, pts, p);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};

// Want to test how we handle dashing when 99% of the dash is clipped out
class GiantDashBench : public SkBenchmark {
    SkString fName;
    SkScalar fStrokeWidth;
    SkPoint  fPts[2];
    SkAutoTUnref<SkPathEffect> fPathEffect;

public:
    enum LineType {
        kHori_LineType,
        kVert_LineType,
        kDiag_LineType,
        kLineTypeCount
    };

    static const char* LineTypeName(LineType lt) {
        static const char* gNames[] = { "hori", "vert", "diag" };
        SK_COMPILE_ASSERT(kLineTypeCount == SK_ARRAY_COUNT(gNames), names_wrong_size);
        return gNames[lt];
    }

    GiantDashBench(LineType lt, SkScalar width)  {
        fName.printf("giantdashline_%s_%g", LineTypeName(lt), width);
        fStrokeWidth = width;

        // deliberately pick intervals that won't be caught by asPoints(), so
        // we can test the filterPath code-path.
        const SkScalar intervals[] = { 2, 1, 1, 1 };
        fPathEffect.reset(new SkDashPathEffect(intervals,
                                               SK_ARRAY_COUNT(intervals), 0));

        SkScalar cx = 640 / 2;  // center X
        SkScalar cy = 480 / 2;  // center Y
        SkMatrix matrix;

        switch (lt) {
            case kHori_LineType:
                matrix.setIdentity();
                break;
            case kVert_LineType:
                matrix.setRotate(90, cx, cy);
                break;
            case kDiag_LineType:
                matrix.setRotate(45, cx, cy);
                break;
            case kLineTypeCount:
                // Not a real enum value.
                break;
        }

        const SkScalar overshoot = 100*1000;
        const SkPoint pts[2] = {
            { -overshoot, cy }, { 640 + overshoot, cy }
        };
        matrix.mapPoints(fPts, pts, 2);
    }

protected:
    virtual const char* onGetName() SK_OVERRIDE {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkPaint p;
        this->setupPaint(&p);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(fStrokeWidth);
        p.setPathEffect(fPathEffect);

        for (int i = 0; i < loops; i++) {
            canvas->drawPoints(SkCanvas::kLines_PointMode, 2, fPts, p);
        }
    }

private:
    typedef SkBenchmark INHERITED;
};


///////////////////////////////////////////////////////////////////////////////

static const SkScalar gDots[] = { SK_Scalar1, SK_Scalar1 };

#define PARAM(array)    array, SK_ARRAY_COUNT(array)

DEF_BENCH( return new DashBench(PARAM(gDots), 0); )
DEF_BENCH( return new DashBench(PARAM(gDots), 1); )
DEF_BENCH( return new DashBench(PARAM(gDots), 1, true); )
DEF_BENCH( return new DashBench(PARAM(gDots), 4); )
DEF_BENCH( return new MakeDashBench(make_poly, "poly"); )
DEF_BENCH( return new MakeDashBench(make_quad, "quad"); )
DEF_BENCH( return new MakeDashBench(make_cubic, "cubic"); )
DEF_BENCH( return new DashLineBench(0, false); )
DEF_BENCH( return new DashLineBench(SK_Scalar1, false); )
DEF_BENCH( return new DashLineBench(2 * SK_Scalar1, false); )
DEF_BENCH( return new DashLineBench(0, true); )
DEF_BENCH( return new DashLineBench(SK_Scalar1, true); )
DEF_BENCH( return new DashLineBench(2 * SK_Scalar1, true); )

DEF_BENCH( return new DrawPointsDashingBench(1, 1, false); )
DEF_BENCH( return new DrawPointsDashingBench(1, 1, true); )
DEF_BENCH( return new DrawPointsDashingBench(3, 1, false); )
DEF_BENCH( return new DrawPointsDashingBench(3, 1, true); )
DEF_BENCH( return new DrawPointsDashingBench(5, 5, false); )
DEF_BENCH( return new DrawPointsDashingBench(5, 5, true); )

/* Disable the GiantDashBench for Android devices until we can better control
 * the memory usage. (https://code.google.com/p/skia/issues/detail?id=1430)
 */
#ifndef SK_BUILD_FOR_ANDROID
DEF_BENCH( return new GiantDashBench(GiantDashBench::kHori_LineType, 0); )
DEF_BENCH( return new GiantDashBench(GiantDashBench::kVert_LineType, 0); )
DEF_BENCH( return new GiantDashBench(GiantDashBench::kDiag_LineType, 0); )

// pass 2 to explicitly avoid any 1-is-the-same-as-hairline special casing

// hori_2 is just too slow to enable at the moment
DEF_BENCH( return new GiantDashBench(GiantDashBench::kHori_LineType, 2); )
DEF_BENCH( return new GiantDashBench(GiantDashBench::kVert_LineType, 2); )
DEF_BENCH( return new GiantDashBench(GiantDashBench::kDiag_LineType, 2); )
#endif
