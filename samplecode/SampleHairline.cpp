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
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkTime.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkCornerPathEffect.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkTo.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"


static void generate_pts(SkRandom& rnd, SkPoint pts[], int count, int w, int h) {
    for (int i = 0; i < count; i++) {
        pts[i].set(rnd.nextUScalar1() * 3 * w - SkIntToScalar(w),
                   rnd.nextUScalar1() * 3 * h - SkIntToScalar(h));
    }
}

static bool check_zeros(const SkPMColor pixels[], int count, int skip) {
    for (int i = 0; i < count; i++) {
        if (*pixels) {
            return false;
        }
        pixels += skip;
    }
    return true;
}

static bool check_bitmap_margin(const SkBitmap& bm, int margin) {
    size_t rb = bm.rowBytes();
    for (int i = 0; i < margin; i++) {
        if (!check_zeros(bm.getAddr32(0, i), bm.width(), 1)) {
            return false;
        }
        int bottom = bm.height() - i - 1;
        if (!check_zeros(bm.getAddr32(0, bottom), bm.width(), 1)) {
            return false;
        }
        // left column
        if (!check_zeros(bm.getAddr32(i, 0), bm.height(), SkToInt(rb >> 2))) {
            return false;
        }
        int right = bm.width() - margin + i;
        if (!check_zeros(bm.getAddr32(right, 0), bm.height(),
                         SkToInt(rb >> 2))) {
            return false;
        }
    }
    return true;
}

#define WIDTH   620
#define HEIGHT  460
#define MARGIN  10

static void line_proc(SkRandom& rnd, SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 2;
    SkPoint pts[N];
    for (int i = 0; i < 400; i++) {
        generate_pts(rnd, pts, N, WIDTH, HEIGHT);

        canvas->drawLine(pts[0], pts[1], paint);
        if (!check_bitmap_margin(bm, MARGIN)) {
            SkDebugf("---- hairline failure (%g %g) (%g %g)\n",
                     pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY);
            break;
        }
    }
}

static void poly_proc(SkRandom& rnd, SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 8;
    SkPoint pts[N];
    for (int i = 0; i < 50; i++) {
        generate_pts(rnd, pts, N, WIDTH, HEIGHT);

        SkPath path;
        path.moveTo(pts[0]);
        for (int j = 1; j < N; j++) {
            path.lineTo(pts[j]);
        }
        canvas->drawPath(path, paint);
    }
}

static SkPoint ave(const SkPoint& a, const SkPoint& b) {
    SkPoint c = a + b;
    c.fX = SkScalarHalf(c.fX);
    c.fY = SkScalarHalf(c.fY);
    return c;
}

static void quad_proc(SkRandom& rnd, SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 30;
    SkPoint pts[N];
    for (int i = 0; i < 10; i++) {
        generate_pts(rnd, pts, N, WIDTH, HEIGHT);

        SkPath path;
        path.moveTo(pts[0]);
        for (int j = 1; j < N - 2; j++) {
            path.quadTo(pts[j], ave(pts[j], pts[j+1]));
        }
        path.quadTo(pts[N - 2], pts[N - 1]);

        canvas->drawPath(path, paint);
    }
}

static void add_cubic(SkPath* path, const SkPoint& mid, const SkPoint& end) {
    SkPoint start;
    path->getLastPt(&start);
    path->cubicTo(ave(start, mid), ave(mid, end), end);
}

static void cube_proc(SkRandom& rnd, SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 30;
    SkPoint pts[N];
    for (int i = 0; i < 10; i++) {
        generate_pts(rnd, pts, N, WIDTH, HEIGHT);

        SkPath path;
        path.moveTo(pts[0]);
        for (int j = 1; j < N - 2; j++) {
            add_cubic(&path, pts[j], ave(pts[j], pts[j+1]));
        }
        add_cubic(&path, pts[N - 2], pts[N - 1]);

        canvas->drawPath(path, paint);
    }
}

using HairProc = void (*)(SkRandom&, SkCanvas*, const SkPaint&, const SkBitmap&);

static void draw_proc(SkCanvas* canvas, bool doAA, HairProc proc) {
    SkRandom rnd;
    rnd.setSeed(0);

    SkBitmap bm, bm2;
    bm.allocN32Pixels(WIDTH + MARGIN*2, HEIGHT + MARGIN*2);
    // this will erase our margin, which we want to always stay 0
    bm.eraseColor(SK_ColorTRANSPARENT);

    bm2.installPixels(SkImageInfo::MakeN32Premul(WIDTH, HEIGHT),
                      bm.getAddr32(MARGIN, MARGIN), bm.rowBytes());

    SkCanvas c2(bm2);
    SkPaint paint;
    paint.setAntiAlias(doAA);
    paint.setStyle(SkPaint::kStroke_Style);

    bm2.eraseColor(SK_ColorTRANSPARENT);
    proc(rnd, &c2, paint, bm);
    canvas->drawBitmap(bm2, SkIntToScalar(10), SkIntToScalar(10), nullptr);
}

class HairlineView : public Sample {
    HairProc fProc;
    const char* fName;
    bool fDoAA;
    SkString name() override { return SkString(fName); }
    void onDrawContent(SkCanvas* canvas) override { draw_proc(canvas, fDoAA, fProc); }

public:
    HairlineView(HairProc p, const char* n, bool doAA) : fProc(p), fName(n), fDoAA(doAA) {}
};

DEF_SAMPLE( return new HairlineView(line_proc, "Hair-line-aa", true ); )
DEF_SAMPLE( return new HairlineView(poly_proc, "Hair-poly-aa", true ); )
DEF_SAMPLE( return new HairlineView(quad_proc, "Hair-quad-aa", true ); )
DEF_SAMPLE( return new HairlineView(cube_proc, "Hair-cube-aa", true ); )
DEF_SAMPLE( return new HairlineView(line_proc, "Hair-line",    false ); )
DEF_SAMPLE( return new HairlineView(poly_proc, "Hair-poly",    false ); )
DEF_SAMPLE( return new HairlineView(quad_proc, "Hair-quad",    false ); )
DEF_SAMPLE( return new HairlineView(cube_proc, "Hair-cube",    false ); )
