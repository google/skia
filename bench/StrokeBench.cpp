/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRRect.h"
#include "SkString.h"

struct RRectRec {
    SkCanvas*   fCanvas;
    SkRRect     fRRect;
    SkPaint     fPaint;
};

typedef const char* (*DrawProc)(const RRectRec*, int);

static const char* draw_rect(const RRectRec* rec, int count) {
    if (rec) {
        const SkRect& r = rec->fRRect.getBounds();
        for (int i = 0; i < count; ++i) {
            rec->fCanvas->drawRect(r, rec->fPaint);
        }
    }
    return "rect";
}

static const char* draw_rrect(const RRectRec* rec, int count) {
    if (rec) {
        for (int i = 0; i < count; ++i) {
            rec->fCanvas->drawRRect(rec->fRRect, rec->fPaint);
        }
    }
    return "rrect";
}

static const char* draw_oval(const RRectRec* rec, int count) {
    if (rec) {
        const SkRect& r = rec->fRRect.getBounds();
        for (int i = 0; i < count; ++i) {
            rec->fCanvas->drawOval(r, rec->fPaint);
        }
    }
    return "oval";
}

// Handles rect, rrect, and oval
//
// Test drawing a small stroked version to see the effect of special-casing
// our stroke code for these convex single-contour shapes.
//
class StrokeRRectBench : public SkBenchmark {
    SkString fName;
    SkPaint::Join fJoin;
    RRectRec fRec;
    DrawProc fProc;
public:
    StrokeRRectBench(SkPaint::Join j, DrawProc proc) {
        static const char* gJoinName[] = {
            "miter", "round", "bevel"
        };

        fJoin = j;
        fProc = proc;
        fName.printf("draw_stroke_%s_%s", proc(NULL, 0), gJoinName[j]);

        SkRect r = { 20, 20, 40, 40 };
        SkScalar rad = 4;
        fRec.fRRect.setRectXY(r, rad, rad);
    }

protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(const int loops, SkCanvas* canvas) {
        fRec.fCanvas = canvas;
        this->setupPaint(&fRec.fPaint);
        fRec.fPaint.setStyle(SkPaint::kStroke_Style);
        fRec.fPaint.setStrokeJoin(fJoin);
        fRec.fPaint.setStrokeWidth(5);
        fProc(&fRec, loops);
    }

private:
    typedef SkBenchmark INHERITED;
};

DEF_BENCH( return new StrokeRRectBench(SkPaint::kRound_Join, draw_rect); )
DEF_BENCH( return new StrokeRRectBench(SkPaint::kBevel_Join, draw_rect); )
DEF_BENCH( return new StrokeRRectBench(SkPaint::kMiter_Join, draw_rect); )

DEF_BENCH( return new StrokeRRectBench(SkPaint::kRound_Join, draw_rrect); )
DEF_BENCH( return new StrokeRRectBench(SkPaint::kBevel_Join, draw_rrect); )
DEF_BENCH( return new StrokeRRectBench(SkPaint::kMiter_Join, draw_rrect); )

DEF_BENCH( return new StrokeRRectBench(SkPaint::kRound_Join, draw_oval); )
DEF_BENCH( return new StrokeRRectBench(SkPaint::kBevel_Join, draw_oval); )
DEF_BENCH( return new StrokeRRectBench(SkPaint::kMiter_Join, draw_oval); )
