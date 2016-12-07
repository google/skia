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
    constexpr int gCoord[] = {
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

static SkScalar make_sawtooth(SkPath* path, int teeth) {
    SkScalar x = SkIntToScalar(20);
    SkScalar y = SkIntToScalar(20);
    const SkScalar x0 = x;
    const SkScalar dx = SkIntToScalar(5);
    const SkScalar dy = SkIntToScalar(10);

    path->moveTo(x, y);
    for (int i = 0; i < teeth; i++) {
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

static SkScalar make_sawtooth_3(SkPath* path) { return make_sawtooth(path, 3); }
static SkScalar make_sawtooth_32(SkPath* path) { return make_sawtooth(path, 32); }

static SkScalar make_house(SkPath* path) {
    path->moveTo(21, 23);
    path->lineTo(21, 11.534f);
    path->lineTo(22.327f, 12.741f);
    path->lineTo(23.673f, 11.261f);
    path->lineTo(12, 0.648f);
    path->lineTo(8, 4.285f);
    path->lineTo(8, 2);
    path->lineTo(4, 2);
    path->lineTo(4, 7.921f);
    path->lineTo(0.327f, 11.26f);
    path->lineTo(1.673f, 12.74f);
    path->lineTo(3, 11.534f);
    path->lineTo(3, 23);
    path->lineTo(11, 23);
    path->lineTo(11, 18);
    path->lineTo(13, 18);
    path->lineTo(13, 23);
    path->lineTo(21, 23);
    path->close();
    path->lineTo(9, 16);
    path->lineTo(9, 21);
    path->lineTo(5, 21);
    path->lineTo(5, 9.715f);
    path->lineTo(12, 3.351f);
    path->lineTo(19, 9.715f);
    path->lineTo(19, 21);
    path->lineTo(15, 21);
    path->lineTo(15, 16);
    path->lineTo(9, 16);
    path->close();
    path->offset(20, 0);
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

constexpr MakePathProc gProcs[] = {
    make_frame,
    make_triangle,
    make_rect,
    make_oval,
    make_sawtooth_32,
    make_star_5,
    make_star_13,
    make_line,
    make_house,
    make_sawtooth_3,
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

                const SkRect* clipPtr = doclip ? &clipR : nullptr;

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

DEF_SIMPLE_GM(rotatedcubicpath, canvas, 200, 200) {
    SkPaint p;
    p.setAntiAlias(true);
    p.setStyle(SkPaint::kFill_Style);

    canvas->translate(50, 50);
    SkPath path;
    path.moveTo(48,-23);
    path.cubicTo(48,-29.5, 6,-30, 6,-30);
    path.cubicTo(6,-30, 2,0, 2,0);
    path.cubicTo(2,0, 44,-21.5, 48,-23);
    path.close();

    p.setColor(SK_ColorBLUE);
    canvas->drawPath(path, p);

    // Rotated path, which is not antialiased on GPU
    p.setColor(SK_ColorRED);
    canvas->rotate(90);
    canvas->drawPath(path, p);
}

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new PathFillGM; )
DEF_GM( return new PathInverseFillGM; )
