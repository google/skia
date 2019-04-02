/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPath.h"

typedef SkScalar (*MakePathProc)(SkPath*);

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

static SkScalar make_star(SkPath* path, int n) {
    const SkScalar c = SkIntToScalar(45);
    const SkScalar r = SkIntToScalar(20);

    SkScalar rad = -SK_ScalarPI / 2;
    const SkScalar drad = (n >> 1) * SK_ScalarPI * 2 / n;

    path->moveTo(c, c - r);
    for (int i = 1; i < n; i++) {
        rad += drad;
        path->lineTo(c + SkScalarCos(rad) * r, c + SkScalarSin(rad) * r);
    }
    path->close();
    return r * 2 * 6 / 5;
}

static SkScalar make_star_5(SkPath* path) { return make_star(path, 5); }
static SkScalar make_star_13(SkPath* path) { return make_star(path, 13); }

static SkScalar make_three_line(SkPath* path) {
    static SkScalar xOffset = 34.f;
    static SkScalar yOffset = 50.f;
    path->moveTo(-32.5f + xOffset, 0.0f + yOffset);
    path->lineTo(32.5f + xOffset, 0.0f + yOffset);

    path->moveTo(-32.5f + xOffset, 19 + yOffset);
    path->lineTo(32.5f + xOffset, 19 + yOffset);

    path->moveTo(-32.5f + xOffset, -19 + yOffset);
    path->lineTo(32.5f + xOffset, -19 + yOffset);
    path->lineTo(-32.5f + xOffset, -19 + yOffset);

    path->close();

    return SkIntToScalar(70);
}

static SkScalar make_arrow(SkPath* path) {
    static SkScalar xOffset = 34.f;
    static SkScalar yOffset = 40.f;
    path->moveTo(-26.f + xOffset, 0.0f + yOffset);
    path->lineTo(26.f + xOffset, 0.0f + yOffset);

    path->moveTo(-28.f + xOffset, -2.4748745f + yOffset);
    path->lineTo(0 + xOffset, 25.525126f + yOffset);

    path->moveTo(-28.f + xOffset, 2.4748745f + yOffset);
    path->lineTo(0 + xOffset, -25.525126f + yOffset);
    path->lineTo(-28.f + xOffset, 2.4748745f + yOffset);

    path->close();

    return SkIntToScalar(70);
}

static SkScalar make_curve(SkPath* path) {
    static SkScalar xOffset = -382.f;
    static SkScalar yOffset = -50.f;
    path->moveTo(491 + xOffset, 56 + yOffset);
    path->conicTo(435.93292f + xOffset, 56.000031f + yOffset,
                  382.61078f + xOffset, 69.752716f + yOffset,
                  0.9920463f);

    return SkIntToScalar(40);
}

static SkScalar make_battery(SkPath* path) {
    static SkScalar xOffset = 5.0f;

    path->moveTo(24.67f + xOffset, 0.33000004f);
    path->lineTo(8.3299999f + xOffset, 0.33000004f);
    path->lineTo(8.3299999f + xOffset, 5.3299999f);
    path->lineTo(0.33000004f + xOffset, 5.3299999f);
    path->lineTo(0.33000004f + xOffset, 50.669998f);
    path->lineTo(32.669998f + xOffset, 50.669998f);
    path->lineTo(32.669998f + xOffset, 5.3299999f);
    path->lineTo(24.67f + xOffset, 5.3299999f);
    path->lineTo(24.67f + xOffset, 0.33000004f);
    path->close();

    path->moveTo(25.727224f + xOffset, 12.886665f);
    path->lineTo(10.907918f + xOffset, 12.886665f);
    path->lineTo(7.5166659f + xOffset, 28.683645f);
    path->lineTo(14.810181f + xOffset, 28.683645f);
    path->lineTo(7.7024879f + xOffset, 46.135998f);
    path->lineTo(28.049999f + xOffset, 25.136419f);
    path->lineTo(16.854223f + xOffset, 25.136419f);
    path->lineTo(25.727224f + xOffset, 12.886665f);
    path->close();
    return SkIntToScalar(50);
}

static SkScalar make_battery2(SkPath* path) {
    static SkScalar xOffset = 225.625f;

    path->moveTo(32.669998f + xOffset, 9.8640003f);
    path->lineTo(0.33000004f + xOffset, 9.8640003f);
    path->lineTo(0.33000004f + xOffset, 50.669998f);
    path->lineTo(32.669998f + xOffset, 50.669998f);
    path->lineTo(32.669998f + xOffset, 9.8640003f);
    path->close();

    path->moveTo(10.907918f + xOffset, 12.886665f);
    path->lineTo(25.727224f + xOffset, 12.886665f);
    path->lineTo(16.854223f + xOffset, 25.136419f);
    path->lineTo(28.049999f + xOffset, 25.136419f);
    path->lineTo(7.7024879f + xOffset, 46.135998f);
    path->lineTo(14.810181f + xOffset, 28.683645f);
    path->lineTo(7.5166659f + xOffset, 28.683645f);
    path->lineTo(10.907918f + xOffset, 12.886665f);
    path->close();

    return SkIntToScalar(70);
}

constexpr MakePathProc gProcs[] = {
    make_triangle,
    make_rect,
    make_oval,
    make_star_5,
    make_star_13,
    make_three_line,
    make_arrow,
    make_curve,
    make_battery,
    make_battery2
};

constexpr SkScalar gWidths[] = {
    2.0f,
    3.0f,
    4.0f,
    5.0f,
    6.0f,
    7.0f,
    7.0f,
    14.0f,
    0.0f,
    0.0f,
};

constexpr SkScalar gMiters[] = {
    2.0f,
    3.0f,
    3.0f,
    3.0f,
    4.0f,
    4.0f,
    4.0f,
    4.0f,
    4.0f,
    4.0f,
};

constexpr SkScalar gXTranslate[] = {
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    -220.625f,
    0.0f,
};

#define N   SK_ARRAY_COUNT(gProcs)

// This GM tests out drawing small paths (i.e., for Ganesh, using the Distance
// Field path renderer) which are filled, stroked and filledAndStroked. In
// particular this ensures that any cache keys in use include the stroking
// parameters.
class SmallPathsGM : public skiagm::GM {
    SkPath  fPath[N];
    SkScalar fDY[N];
protected:
    void onOnceBeforeDraw() override {
        for (size_t i = 0; i < N; i++) {
            fDY[i] = gProcs[i](&fPath[i]);
        }
    }

    SkString onShortName() override {
        return SkString("smallpaths");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 480);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setAntiAlias(true);

        // first column: filled paths
        canvas->save();
        for (size_t i = 0; i < N; i++) {
            canvas->drawPath(fPath[i], paint);
            canvas->translate(gXTranslate[i], fDY[i]);
        }
        canvas->restore();
        canvas->translate(SkIntToScalar(120), SkIntToScalar(0));

        // second column: stroked paths
        canvas->save();
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeCap(SkPaint::kButt_Cap);
        for (size_t i = 0; i < N; i++) {
            paint.setStrokeWidth(gWidths[i]);
            paint.setStrokeMiter(gMiters[i]);
            canvas->drawPath(fPath[i], paint);
            canvas->translate(gXTranslate[i], fDY[i]);
        }
        canvas->restore();
        canvas->translate(SkIntToScalar(120), SkIntToScalar(0));

        // third column: stroked paths with different widths
        canvas->save();
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeCap(SkPaint::kButt_Cap);
        for (size_t i = 0; i < N; i++) {
            paint.setStrokeWidth(gWidths[i] + 2.0f);
            paint.setStrokeMiter(gMiters[i]);
            canvas->drawPath(fPath[i], paint);
            canvas->translate(gXTranslate[i], fDY[i]);
        }
        canvas->restore();
        canvas->translate(SkIntToScalar(120), SkIntToScalar(0));

        // fourth column: stroked and filled paths
        paint.setStyle(SkPaint::kStrokeAndFill_Style);
        paint.setStrokeCap(SkPaint::kButt_Cap);
        for (size_t i = 0; i < N; i++) {
            paint.setStrokeWidth(gWidths[i]);
            paint.setStrokeMiter(gMiters[i]);
            canvas->drawPath(fPath[i], paint);
            canvas->translate(gXTranslate[i], fDY[i]);
        }

    }

private:
    typedef skiagm::GM INHERITED;
};

DEF_GM(return new SmallPathsGM;)
