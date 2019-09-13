/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "samplecode/Sample.h"
#include "src/core/SkAAClip.h"

static void testop(const SkIRect& r0, const SkIRect& r1, SkRegion::Op op,
                   const SkIRect& expectedR) {
    SkAAClip c0, c1, c2;
    c0.setRect(r0);
    c1.setRect(r1);
    c2.op(c0, c1, op);

    SkDEBUGCODE(SkIRect r2 = c2.getBounds());
    SkASSERT(r2 == expectedR);
}

static const struct {
    SkIRect r0;
    SkIRect r1;
    SkRegion::Op op;
    SkIRect expectedR;
} gRec[] = {
    {{ 1, 2, 9, 3 }, { -3, 2, 5, 11 }, SkRegion::kDifference_Op, { 5, 2, 9, 3 }},
    {{ 1, 10, 5, 13 }, { 1, 2, 5, 11 }, SkRegion::kDifference_Op, { 1, 11, 5, 13 }},
    {{ 1, 10, 5, 13 }, { 1, 2, 5, 11 }, SkRegion::kReverseDifference_Op, { 1, 2, 5, 10 }},
};

static void testop() {
    for (size_t i = 0; i < SK_ARRAY_COUNT(gRec); ++i) {
        testop(gRec[i].r0, gRec[i].r1, gRec[i].op, gRec[i].expectedR);
    }
}

static void drawClip(SkCanvas* canvas, const SkAAClip& clip) {
    SkMask mask;
    SkBitmap bm;

    clip.copyToMask(&mask);
    SkAutoMaskFreeImage amfi(mask.fImage);

    bm.installMaskPixels(mask);

    SkPaint paint;
    canvas->drawBitmap(bm,
                       SK_Scalar1 * mask.fBounds.fLeft,
                       SK_Scalar1 * mask.fBounds.fTop,
                       &paint);
}

class AAClipView : public Sample {
    SkString name() override { return SkString("AAClip"); }

    void onOnceBeforeDraw() override { testop(); }

    void onDrawContent(SkCanvas* canvas) override {
#if 1
        SkAAClip aaclip;
        SkPath path;
        SkRect bounds;

        bounds.setLTRB(0, 0, 20, 20);
        bounds.inset(SK_ScalarHalf, SK_ScalarHalf);

//        path.addRect(bounds);
//        path.addOval(bounds);
        path.addRRect(SkRRect::MakeRectXY(bounds, 4, 4));
        aaclip.setPath(path);
        canvas->translate(30, 30);
        drawClip(canvas, aaclip);

        SkAAClip aaclip2;
        path.offset(10, 10);
        aaclip2.setPath(path);
        canvas->translate(30, 0);
        drawClip(canvas, aaclip2);

        SkAAClip aaclip3;
        aaclip3.op(aaclip, aaclip2, SkRegion::kIntersect_Op);
        canvas->translate(30, 0);
        drawClip(canvas, aaclip3);

#endif

#if 0
        SkRect r;
        r.set(0, 0, this->width(), this->height());
        r.inset(20, 20);
        canvas->clipRect(r);

        SkPath path;
        path.addRect(r);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorRED);
        canvas->drawPath(path, paint);
#endif
    }
};
DEF_SAMPLE( return new AAClipView(); )
