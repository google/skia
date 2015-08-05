/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"

typedef SkScalar (*MakePathProc)(SkPath*);

static SkScalar make_frame(SkPath* path) {
    SkRect r = { SkIntToScalar(10), SkIntToScalar(10),
                 SkIntToScalar(630), SkIntToScalar(470) };
    path->addRoundRect(r, SkIntToScalar(15), SkIntToScalar(15));

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(SkIntToScalar(5));
    paint.getFillPath(*path, path);
    return SkIntToScalar(15);
}

static SkScalar make_triangle(SkPath* path) {
    static const int gCoord[] = {
        10, 20, 15, 5, 30, 30
    };
    path->moveTo(SkIntToScalar(gCoord[0]), SkIntToScalar(gCoord[1]));
    path->lineTo(SkIntToScalar(gCoord[2]), SkIntToScalar(gCoord[3]));
    path->lineTo(SkIntToScalar(gCoord[4]), SkIntToScalar(gCoord[5]));
    path->close();
    path->offset(SkIntToScalar(10), SkIntToScalar(0));
    return SkIntToScalar(30);
}

static SkScalar make_rect(SkPath* path) {
    SkRect r = { SkIntToScalar(10), SkIntToScalar(10),
                 SkIntToScalar(30), SkIntToScalar(30) };
    path->addRect(r);
    path->offset(SkIntToScalar(10), SkIntToScalar(0));
    return SkIntToScalar(30);
}

static SkScalar make_oval(SkPath* path) {
    SkRect r = { SkIntToScalar(10), SkIntToScalar(10),
                 SkIntToScalar(30), SkIntToScalar(30) };
    path->addOval(r);
    path->offset(SkIntToScalar(10), SkIntToScalar(0));
    return SkIntToScalar(30);
}

static SkScalar make_sawtooth(SkPath* path) {
    SkScalar x = SkIntToScalar(20);
    SkScalar y = SkIntToScalar(20);
    const SkScalar x0 = x;
    const SkScalar dx = SK_Scalar1 * 5;
    const SkScalar dy = SK_Scalar1 * 10;

    path->moveTo(x, y);
    for (int i = 0; i < 32; i++) {
        x += dx;
        path->lineTo(x, y - dy);
        x += dx;
        path->lineTo(x, y + dy);
    }
    path->lineTo(x, y + (2 * dy));
    path->lineTo(x0, y + (2 * dy));
    path->close();
    return SkIntToScalar(30);
}

static SkScalar make_star(SkPath* path, int n) {
    const SkScalar c = SkIntToScalar(45);
    const SkScalar r = SkIntToScalar(20);

    SkScalar rad = -SK_ScalarPI / 2;
    const SkScalar drad = (n >> 1) * SK_ScalarPI * 2 / n;

    path->moveTo(c, c - r);
    for (int i = 1; i < n; i++) {
        rad += drad;
        SkScalar cosV, sinV = SkScalarSinCos(rad, &cosV);
        path->lineTo(c + SkScalarMul(cosV, r), c + SkScalarMul(sinV, r));
    }
    path->close();
    return r * 2 * 6 / 5;
}

static SkScalar make_star_5(SkPath* path) { return make_star(path, 5); }
static SkScalar make_star_13(SkPath* path) { return make_star(path, 13); }

// We don't expect any output from this path.
static SkScalar make_line(SkPath* path) {
    path->moveTo(SkIntToScalar(30), SkIntToScalar(30));
    path->lineTo(SkIntToScalar(120), SkIntToScalar(40));
    path->close();
    path->moveTo(SkIntToScalar(150), SkIntToScalar(30));
    path->lineTo(SkIntToScalar(150), SkIntToScalar(30));
    path->lineTo(SkIntToScalar(300), SkIntToScalar(40));
    path->close();
    return SkIntToScalar(40);
}

static const MakePathProc gProcs[] = {
    make_frame,
    make_triangle,
    make_rect,
    make_oval,
    make_sawtooth,
    make_star_5,
    make_star_13,
    make_line,
};

#define N   SK_ARRAY_COUNT(gProcs)

class PathFillGM : public skiagm::GM {
    SkPath  fPath[N];
    SkScalar fDY[N];
protected:
    void onOnceBeforeDraw() override {
        for (size_t i = 0; i < N; i++) {
            fDY[i] = gProcs[i](&fPath[i]);
        }
    }


    SkString onShortName() override {
        return SkString("pathfill");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);

        for (size_t i = 0; i < N; i++) {
            canvas->drawPath(fPath[i], paint);
            canvas->translate(SkIntToScalar(0), fDY[i]);
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

// test inverse-fill w/ a clip that completely excludes the geometry
class PathInverseFillGM : public skiagm::GM {
    SkPath  fPath[N];
    SkScalar fDY[N];
protected:
    void onOnceBeforeDraw() override {
        for (size_t i = 0; i < N; i++) {
            fDY[i] = gProcs[i](&fPath[i]);
        }
    }

    SkString onShortName() override {
        return SkString("pathinvfill");
    }

    SkISize onISize() override {
        return SkISize::Make(450, 220);
    }

    static void show(SkCanvas* canvas, const SkPath& path, const SkPaint& paint,
                     const SkRect* clip, SkScalar top, const SkScalar bottom) {
        canvas->save();
        if (clip) {
            SkRect r = *clip;
            r.fTop = top;
            r.fBottom = bottom;
            canvas->clipRect(r);
        }
        canvas->drawPath(path, paint);
        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        SkPath path;

        path.addCircle(SkIntToScalar(50), SkIntToScalar(50), SkIntToScalar(40));
        path.toggleInverseFillType();

        SkRect clipR = { 0, 0, SkIntToScalar(100), SkIntToScalar(200) };

        canvas->translate(SkIntToScalar(10), SkIntToScalar(10));

        for (int doclip = 0; doclip <= 1; ++doclip) {
            for (int aa = 0; aa <= 1; ++aa) {
                SkPaint paint;
                paint.setAntiAlias(SkToBool(aa));

                canvas->save();
                canvas->clipRect(clipR);

                const SkRect* clipPtr = doclip ? &clipR : NULL;

                show(canvas, path, paint, clipPtr, clipR.fTop, clipR.centerY());
                show(canvas, path, paint, clipPtr, clipR.centerY(), clipR.fBottom);

                canvas->restore();
                canvas->translate(SkIntToScalar(110), 0);
            }
        }
    }

private:
    typedef skiagm::GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new PathFillGM; }
static skiagm::GMRegistry reg(MyFactory);

static skiagm::GM* F1(void*) { return new PathInverseFillGM; }
static skiagm::GMRegistry gR1(F1);
