/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGraphics.h"
#include "SkRandom.h"
#include "SkBlurDrawLooper.h"
#include "SkGradientShader.h"

typedef SkScalar (*MakePathProc)(SkPath*);

static SkScalar make_frame(SkPath* path) {
    SkRect r = { 10, 10, 630, 470 };
    path->addRoundRect(r, 15, 15);

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(5);
    paint.getFillPath(*path, path);
    return 15;
}

static SkScalar make_triangle(SkPath* path) {
    static const int gCoord[] = {
        10, 20, 15, 5, 30, 30
    };
    path->moveTo(SkIntToScalar(gCoord[0]), SkIntToScalar(gCoord[1]));
    path->lineTo(SkIntToScalar(gCoord[2]), SkIntToScalar(gCoord[3]));
    path->lineTo(SkIntToScalar(gCoord[4]), SkIntToScalar(gCoord[5]));
    path->close();
    path->offset(10, 0);
    return SkIntToScalar(30);
}

static SkScalar make_rect(SkPath* path) {
    SkRect r = { 10, 10, 30, 30 };
    path->addRect(r);
    path->offset(10, 0);
    return SkIntToScalar(30);
}

static SkScalar make_oval(SkPath* path) {
    SkRect r = { 10, 10, 30, 30 };
    path->addOval(r);
    path->offset(10, 0);
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
    path->lineTo(x, y + 2 * dy);
    path->lineTo(x0, y + 2 * dy);
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
        path->lineTo(c + cosV * r, c + sinV * r);
    }
    path->close();
    return r * 2 * 6 / 5;
}

static SkScalar make_star_5(SkPath* path) { return make_star(path, 5); }
static SkScalar make_star_13(SkPath* path) { return make_star(path, 13); }

static const MakePathProc gProcs[] = {
    make_frame,
    make_triangle,
    make_rect,
    make_oval,
    make_sawtooth,
    make_star_5,
    make_star_13
};

#define N   SK_ARRAY_COUNT(gProcs)

class PathFillView : public SampleView {
    SkPath  fPath[N];
    SkScalar fDY[N];

public:
    PathFillView() {
        for (size_t i = 0; i < N; i++) {
            fDY[i] = gProcs[i](&fPath[i]);
        }
        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "PathFill");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPaint paint;
        paint.setAntiAlias(true);

        for (size_t i = 0; i < N; i++) {
            canvas->drawPath(fPath[i], paint);
            canvas->translate(0, fDY[i]);
        }
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new PathFillView; }
static SkViewRegister reg(MyFactory);
