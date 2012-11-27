/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkPath.h"

#define STROKE_WIDTH    SkIntToScalar(10)

typedef void (*Proc)(SkCanvas*, const SkRect&, const SkPaint&);

static void fill_rect(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    canvas->drawRect(r, p);
}

static void stroke_rect(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    SkPaint paint(p);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(STROKE_WIDTH);
    canvas->drawRect(r, paint);
}

static void draw_donut(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    SkRect  rect;
    SkPath  path;

    rect = r;
    rect.outset(STROKE_WIDTH/2, STROKE_WIDTH/2);
    path.addRect(rect);
    rect = r;
    rect.inset(STROKE_WIDTH/2, STROKE_WIDTH/2);

    path.addRect(rect);
    path.setFillType(SkPath::kEvenOdd_FillType);

    canvas->drawPath(path, p);
}

static void draw_donut_skewed(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    SkRect  rect;
    SkPath  path;

    rect = r;
    rect.outset(STROKE_WIDTH/2, STROKE_WIDTH/2);
    path.addRect(rect);
    rect = r;
    rect.inset(STROKE_WIDTH/2, STROKE_WIDTH/2);

    rect.offset(7, -7);

    path.addRect(rect);
    path.setFillType(SkPath::kEvenOdd_FillType);

    canvas->drawPath(path, p);
}

#include "SkGradientShader.h"

typedef void (*PaintProc)(SkPaint*, SkScalar width);

static void setgrad(SkPaint* paint, SkScalar width) {
    SkPoint pts[] = { { 0, 0 }, { width, 0 } };
    SkColor colors[] = { SK_ColorRED, SK_ColorGREEN };
    SkShader* s = SkGradientShader::CreateLinear(pts, colors, NULL, 2,
                                                 SkShader::kClamp_TileMode);
    paint->setShader(s)->unref();
}

class BlurRectGM : public skiagm::GM {
    SkAutoTUnref<SkMaskFilter> fMaskFilter;
    SkString  fName;
    PaintProc fPProc;
    SkAlpha   fAlpha;
public:
    BlurRectGM(const char name[], PaintProc pproc, U8CPU alpha) :
        fMaskFilter(SkBlurMaskFilter::Create(STROKE_WIDTH/2,
                                       SkBlurMaskFilter::kNormal_BlurStyle,
                                       SkBlurMaskFilter::kHighQuality_BlurFlag))
        , fName(name)
        , fPProc(pproc)
        , fAlpha(SkToU8(alpha))
    {}

protected:
    virtual SkString onShortName() {
        return fName;
    }

    virtual SkISize onISize() {
        return SkISize::Make(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) {
        canvas->translate(STROKE_WIDTH*3/2, STROKE_WIDTH*3/2);

        SkRect  r = { 0, 0, 250, 120 };

        SkPaint paint;
        paint.setMaskFilter(fMaskFilter);
        if (fPProc) {
            fPProc(&paint, r.width());
        }
        paint.setAlpha(fAlpha);

        static const Proc procs[] = {
            fill_rect, draw_donut, draw_donut_skewed
        };

        this->drawProcs(canvas, r, paint, false, procs, SK_ARRAY_COUNT(procs));
        canvas->translate(r.width() * 4/3, 0);
        this->drawProcs(canvas, r, paint, true, procs, SK_ARRAY_COUNT(procs));
    }

    virtual uint32_t onGetFlags() const { return kSkipPipe_Flag; }

private:
    void drawProcs(SkCanvas* canvas, const SkRect& r, const SkPaint& paint,
                   bool doClip, const Proc procs[], size_t procsCount) {
        SkAutoCanvasRestore acr(canvas, true);
        for (size_t i = 0; i < procsCount; ++i) {
            if (doClip) {
                SkRect clipRect(r);
                clipRect.inset(STROKE_WIDTH/2, STROKE_WIDTH/2);
                canvas->save();
                canvas->clipRect(r);
            }
            procs[i](canvas, r, paint);
            if (doClip) {
                canvas->restore();
            }
            canvas->translate(0, r.height() * 4/3);
        }
    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BlurRectGM("blurrect", NULL, 0xFF);)
DEF_GM(return new BlurRectGM("blurrect_grad_80", setgrad, 0x80);)

