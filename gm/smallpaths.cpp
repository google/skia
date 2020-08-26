/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/pathops/SkPathOps.h"

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

    return SkIntToScalar(60);
}

static SkScalar make_ring(SkPath* path) {
    static SkScalar xOffset = 120;
    static SkScalar yOffset = -270.f;

    path->setFillType(SkPathFillType::kWinding);
    path->moveTo(xOffset + 144.859f, yOffset + 285.172f);
    path->lineTo(xOffset + 144.859f, yOffset + 285.172f);
    path->lineTo(xOffset + 144.859f, yOffset + 285.172f);
    path->lineTo(xOffset + 143.132f, yOffset + 284.617f);
    path->lineTo(xOffset + 144.859f, yOffset + 285.172f);
    path->close();
    path->moveTo(xOffset + 135.922f, yOffset + 286.844f);
    path->lineTo(xOffset + 135.922f, yOffset + 286.844f);
    path->lineTo(xOffset + 135.922f, yOffset + 286.844f);
    path->lineTo(xOffset + 135.367f, yOffset + 288.571f);
    path->lineTo(xOffset + 135.922f, yOffset + 286.844f);
    path->close();
    path->moveTo(xOffset + 135.922f, yOffset + 286.844f);
    path->cubicTo(xOffset + 137.07f, yOffset + 287.219f, xOffset + 138.242f, yOffset + 287.086f,
                  xOffset + 139.242f, yOffset + 286.578f);
    path->cubicTo(xOffset + 140.234f, yOffset + 286.078f, xOffset + 141.031f, yOffset + 285.203f,
                  xOffset + 141.406f, yOffset + 284.055f);
    path->lineTo(xOffset + 144.859f, yOffset + 285.172f);
    path->cubicTo(xOffset + 143.492f, yOffset + 289.375f, xOffset + 138.992f, yOffset + 291.656f,
                  xOffset + 134.797f, yOffset + 290.297f);
    path->lineTo(xOffset + 135.922f, yOffset + 286.844f);
    path->close();
    path->moveTo(xOffset + 129.68f, yOffset + 280.242f);
    path->lineTo(xOffset + 129.68f, yOffset + 280.242f);
    path->lineTo(xOffset + 129.68f, yOffset + 280.242f);
    path->lineTo(xOffset + 131.407f, yOffset + 280.804f);
    path->lineTo(xOffset + 129.68f, yOffset + 280.242f);
    path->close();
    path->moveTo(xOffset + 133.133f, yOffset + 281.367f);
    path->cubicTo(xOffset + 132.758f, yOffset + 282.508f, xOffset + 132.883f, yOffset + 283.687f,
                  xOffset + 133.391f, yOffset + 284.679f);
    path->cubicTo(xOffset + 133.907f, yOffset + 285.679f, xOffset + 134.774f, yOffset + 286.468f,
                  xOffset + 135.922f, yOffset + 286.843f);
    path->lineTo(xOffset + 134.797f, yOffset + 290.296f);
    path->cubicTo(xOffset + 130.602f, yOffset + 288.929f, xOffset + 128.313f, yOffset + 284.437f,
                  xOffset + 129.68f, yOffset + 280.241f);
    path->lineTo(xOffset + 133.133f, yOffset + 281.367f);
    path->close();
    path->moveTo(xOffset + 139.742f, yOffset + 275.117f);
    path->lineTo(xOffset + 139.742f, yOffset + 275.117f);
    path->lineTo(xOffset + 139.18f, yOffset + 276.844f);
    path->lineTo(xOffset + 139.742f, yOffset + 275.117f);
    path->close();
    path->moveTo(xOffset + 138.609f, yOffset + 278.57f);
    path->cubicTo(xOffset + 137.461f, yOffset + 278.203f, xOffset + 136.297f, yOffset + 278.328f,
                  xOffset + 135.297f, yOffset + 278.836f);
    path->cubicTo(xOffset + 134.297f, yOffset + 279.344f, xOffset + 133.508f, yOffset + 280.219f,
                  xOffset + 133.133f, yOffset + 281.367f);
    path->lineTo(xOffset + 129.68f, yOffset + 280.242f);
    path->cubicTo(xOffset + 131.047f, yOffset + 276.039f, xOffset + 135.539f, yOffset + 273.758f,
                  xOffset + 139.742f, yOffset + 275.117f);
    path->lineTo(xOffset + 138.609f, yOffset + 278.57f);
    path->close();
    path->moveTo(xOffset + 141.406f, yOffset + 284.055f);
    path->cubicTo(xOffset + 141.773f, yOffset + 282.907f, xOffset + 141.648f, yOffset + 281.735f,
                  xOffset + 141.148f, yOffset + 280.735f);
    path->cubicTo(xOffset + 140.625f, yOffset + 279.735f, xOffset + 139.757f, yOffset + 278.946f,
                  xOffset + 138.609f, yOffset + 278.571f);
    path->lineTo(xOffset + 139.742f, yOffset + 275.118f);
    path->cubicTo(xOffset + 143.937f, yOffset + 276.493f, xOffset + 146.219f, yOffset + 280.977f,
                  xOffset + 144.859f, yOffset + 285.173f);
    path->lineTo(xOffset + 141.406f, yOffset + 284.055f);
    path->close();

    // uncomment to reveal PathOps bug, see https://bugs.chromium.org/p/skia/issues/detail?id=9732
    // (void) Simplify(*path, path);

    return SkIntToScalar(15);
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
    make_battery2,
    make_ring
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
    0.0f
};
static_assert(SK_ARRAY_COUNT(gWidths) == SK_ARRAY_COUNT(gProcs));

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
    4.0f,
};
static_assert(SK_ARRAY_COUNT(gMiters) == SK_ARRAY_COUNT(gProcs));

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
    0.0f,
};
static_assert(SK_ARRAY_COUNT(gXTranslate) == SK_ARRAY_COUNT(gProcs));

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
        return SkISize::Make(640, 512);
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
