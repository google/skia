/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkCornerPathEffect.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"
#include "SkTypeface.h"
#include "SkStream.h"
#include "SkColorPriv.h"

static SkRandom gRand;

static void generate_pts(SkPoint pts[], int count, int w, int h) {
    for (int i = 0; i < count; i++) {
        pts[i].set(gRand.nextUScalar1() * 3 * w - SkIntToScalar(w),
                   gRand.nextUScalar1() * 3 * h - SkIntToScalar(h));
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

static void line_proc(SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 2;
    SkPoint pts[N];
    for (int i = 0; i < 400; i++) {
        generate_pts(pts, N, WIDTH, HEIGHT);

        canvas->drawLine(pts[0], pts[1], paint);
        if (!check_bitmap_margin(bm, MARGIN)) {
            SkDebugf("---- hairline failure (%g %g) (%g %g)\n",
                     pts[0].fX, pts[0].fY, pts[1].fX, pts[1].fY);
            break;
        }
    }
}

static void poly_proc(SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 8;
    SkPoint pts[N];
    for (int i = 0; i < 50; i++) {
        generate_pts(pts, N, WIDTH, HEIGHT);

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

static void quad_proc(SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 30;
    SkPoint pts[N];
    for (int i = 0; i < 10; i++) {
        generate_pts(pts, N, WIDTH, HEIGHT);

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

static void cube_proc(SkCanvas* canvas, const SkPaint& paint,
                      const SkBitmap& bm) {
    const int N = 30;
    SkPoint pts[N];
    for (int i = 0; i < 10; i++) {
        generate_pts(pts, N, WIDTH, HEIGHT);

        SkPath path;
        path.moveTo(pts[0]);
        for (int j = 1; j < N - 2; j++) {
            add_cubic(&path, pts[j], ave(pts[j], pts[j+1]));
        }
        add_cubic(&path, pts[N - 2], pts[N - 1]);

        canvas->drawPath(path, paint);
    }
}

typedef void (*HairProc)(SkCanvas*, const SkPaint&, const SkBitmap&);

static const struct {
    const char* fName;
    HairProc    fProc;
} gProcs[] = {
    { "line",   line_proc },
    { "poly",   poly_proc },
    { "quad",   quad_proc },
    { "cube",   cube_proc },
};

static int cycle_hairproc_index(int index) {
    return (index + 1) % SK_ARRAY_COUNT(gProcs);
}

class HairlineView : public SampleView {
    SkMSec fNow;
    int fProcIndex;
    bool fDoAA;
public:
    HairlineView() {
        fProcIndex = 0;
        fDoAA = true;
        fNow = 0;
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SkString str;
            str.printf("Hair-%s", gProcs[fProcIndex].fName);
            SampleCode::TitleR(evt, str.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void show_bitmaps(SkCanvas* canvas, const SkBitmap& b0, const SkBitmap& b1,
                      const SkIRect& inset) {
        canvas->drawBitmap(b0, 0, 0, nullptr);
        canvas->drawBitmap(b1, SkIntToScalar(b0.width()), 0, nullptr);
    }

    void onDrawContent(SkCanvas* canvas) override {
        gRand.setSeed(fNow);

        SkBitmap bm, bm2;
        bm.allocN32Pixels(WIDTH + MARGIN*2, HEIGHT + MARGIN*2);
        // this will erase our margin, which we want to always stay 0
        bm.eraseColor(SK_ColorTRANSPARENT);

        bm2.installPixels(SkImageInfo::MakeN32Premul(WIDTH, HEIGHT),
                          bm.getAddr32(MARGIN, MARGIN), bm.rowBytes());

        SkCanvas c2(bm2);
        SkPaint paint;
        paint.setAntiAlias(fDoAA);
        paint.setStyle(SkPaint::kStroke_Style);

        bm2.eraseColor(SK_ColorTRANSPARENT);
        gProcs[fProcIndex].fProc(&c2, paint, bm);
        canvas->drawBitmap(bm2, SkIntToScalar(10), SkIntToScalar(10), nullptr);
    }

    bool onAnimate(const SkAnimTimer&) override {
        if (fDoAA) {
            fProcIndex = cycle_hairproc_index(fProcIndex);
            // todo: signal that we want to rebuild our TITLE
        }
        fDoAA = !fDoAA;
        return true;
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        fDoAA = !fDoAA;
        this->inval(nullptr);
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }


private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new HairlineView; }
static SkViewRegister reg(MyFactory);
