/*
* Copyright 2012 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkPath.h"

#define STROKE_WIDTH    SkIntToScalar(10)

typedef void (*Proc)(SkCanvas*, const SkRect&, const SkPaint&);

static void fill_rect(SkCanvas* canvas, const SkRect& r, const SkPaint& p) {
    canvas->drawRect(r, p);
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

static const char* gBlurStyle2Name[] = {
    "normal",
    "solid",
    "outer",
    "inner"
};

class BlurRectGM : public skiagm::GM {
      SkAutoTUnref<SkMaskFilter> fMaskFilter;
      SkString  fName;
      PaintProc fPProc;
      SkAlpha   fAlpha;
public:
    BlurRectGM(const char name[], PaintProc pproc, U8CPU alpha,
               SkBlurMaskFilter::BlurStyle bs)
               : fMaskFilter(SkBlurMaskFilter::Create(bs,
                             SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(STROKE_WIDTH/2)),
                             SkBlurMaskFilter::kHighQuality_BlurFlag))
               , fName(name)
               , fPProc(pproc)
               , fAlpha(SkToU8(alpha)) {
        fName.appendf("_%s", gBlurStyle2Name[bs]);
    }

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
private:
    typedef GM INHERITED;
};

class BlurRectCompareGM : public skiagm::GM {
    SkString  fName;
    unsigned int fRectWidth, fRectHeight;
    SkScalar fRadius;
    SkBlurMask::Style fStyle;
public:
    BlurRectCompareGM(const char name[], unsigned int rectWidth,
                      unsigned int rectHeight, float radius,
                      SkBlurMask::Style style)
        : fName(name)
        , fRectWidth(rectWidth)
        , fRectHeight(rectHeight)
        , fRadius(radius)
        , fStyle(style) {
    }
    int width() const {
        return fRectWidth;
    }
    int height() const {
        return fRectHeight;
    }
    SkScalar radius() const {
        return fRadius;
    }
    SkBlurMask::Style style() const {
        return fStyle;
    }

protected:
    virtual SkString onShortName() {
        return fName;
    }

    virtual SkISize onISize() {
        return SkISize::Make(640, 480);
    }

    virtual bool makeMask(SkMask *m, const SkRect&) = 0;

    virtual void onDraw(SkCanvas* canvas) {
        SkRect r;
        r.setWH(SkIntToScalar(fRectWidth), SkIntToScalar(fRectHeight));

        SkISize canvas_size = canvas->getDeviceSize();
        int center_x = (canvas_size.fWidth - (int)(r.width()))/2;
        int center_y = (canvas_size.fHeight - (int)(r.height()))/2;

        SkMask mask;

        if (!this->makeMask(&mask, r)) {
            SkPaint paint;
            r.offset( SkIntToScalar(center_x), SkIntToScalar(center_y) );
            canvas->drawRect(r,paint);
            return;
        }
        SkAutoMaskFreeImage amfi(mask.fImage);

        SkBitmap bm;
        bm.setConfig(SkBitmap::kA8_Config, mask.fBounds.width(), mask.fBounds.height());
        bm.setPixels(mask.fImage);

        center_x = (canvas_size.fWidth - mask.fBounds.width())/2;
        center_y = (canvas_size.fHeight - mask.fBounds.height())/2;

        canvas->drawBitmap(bm, SkIntToScalar(center_x), SkIntToScalar(center_y), NULL);
    }

    virtual uint32_t onGetFlags() const { return kSkipPipe_Flag; }

private:
    typedef GM INHERITED;
};

class BlurRectFastGM: public BlurRectCompareGM {
public:
    BlurRectFastGM(const char name[], unsigned int rectWidth,
                   unsigned int rectHeight, float blurRadius,
                   SkBlurMask::Style style) :
        INHERITED(name, rectWidth, rectHeight, blurRadius, style) {
        }

protected:
    virtual bool makeMask(SkMask *m, const SkRect& r) SK_OVERRIDE {
        return SkBlurMask::BlurRect(SkBlurMask::ConvertRadiusToSigma(this->radius()),
                                    m, r, this->style());
    }
private:
    typedef BlurRectCompareGM INHERITED;
};

class BlurRectSlowGM: public BlurRectCompareGM {
public:
    BlurRectSlowGM(const char name[], unsigned int rectWidth, unsigned int rectHeight,
                   float blurRadius, SkBlurMask::Style style)
        : INHERITED(name, rectWidth, rectHeight, blurRadius, style) {
        }

protected:
    virtual bool makeMask(SkMask *m, const SkRect& r) SK_OVERRIDE {
        SkMask src;
        r.roundOut(&src.fBounds);
        src.fBounds.offset(-src.fBounds.fLeft, -src.fBounds.fTop);  // move to origin
        src.fFormat = SkMask::kA8_Format;
        src.fRowBytes = src.fBounds.width();
        src.fImage = SkMask::AllocImage(src.computeTotalImageSize());
        SkAutoMaskFreeImage amfi(src.fImage);

        memset(src.fImage, 0xff, src.computeTotalImageSize());

        return SkBlurMask::BoxBlur(m, src,
                                   SkBlurMask::ConvertRadiusToSigma(this->radius()),
                                   this->style(), this->getQuality());
    }

    virtual SkBlurMask::Quality getQuality() {
        return SkBlurMask::kHigh_Quality;
    }
private:
    typedef BlurRectCompareGM INHERITED;
};

class BlurRectSlowLowGM: public BlurRectSlowGM {
public:
    BlurRectSlowLowGM(const char name[], unsigned int rectWidth, unsigned int rectHeight,
                      float blurRadius, SkBlurMask::Style style)
        : INHERITED(name, rectWidth, rectHeight, blurRadius, style) {
        }

protected:
    virtual SkBlurMask::Quality getQuality() SK_OVERRIDE {
        return SkBlurMask::kLow_Quality;
    }
private:
    typedef BlurRectSlowGM INHERITED;
};

class BlurRectGroundTruthGM: public BlurRectCompareGM {
public:
    BlurRectGroundTruthGM(const char name[], unsigned int rectWidth, unsigned int rectHeight,
                          float blurRadius, SkBlurMask::Style style)
        : INHERITED(name, rectWidth, rectHeight, blurRadius, style) {
        }

protected:
    virtual bool makeMask(SkMask *m, const SkRect& r) SK_OVERRIDE {
        SkMask src;
        r.roundOut(&src.fBounds);
        src.fBounds.offset(-src.fBounds.fLeft, -src.fBounds.fTop);  // move to origin
        src.fFormat = SkMask::kA8_Format;
        src.fRowBytes = src.fBounds.width();
        src.fImage = SkMask::AllocImage(src.computeTotalImageSize());
        SkAutoMaskFreeImage amfi(src.fImage);

        memset(src.fImage, 0xff, src.computeTotalImageSize());

        return SkBlurMask::BlurGroundTruth(SkBlurMask::ConvertRadiusToSigma(this->radius()),
                                           m, src, this->style());
    }

    virtual SkBlurMask::Quality getQuality() {
        return SkBlurMask::kHigh_Quality;
    }
private:
    typedef BlurRectCompareGM INHERITED;
};


//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BlurRectGM("blurrect", NULL, 0xFF, SkBlurMaskFilter::kNormal_BlurStyle);)
DEF_GM(return new BlurRectGM("blurrect", NULL, 0xFF, SkBlurMaskFilter::kSolid_BlurStyle);)
DEF_GM(return new BlurRectGM("blurrect", NULL, 0xFF, SkBlurMaskFilter::kOuter_BlurStyle);)
DEF_GM(return new BlurRectGM("blurrect", NULL, 0xFF, SkBlurMaskFilter::kInner_BlurStyle);)

static const SkScalar kBig = 20;
static const SkScalar kSmall = 2;

// regular size rects, blurs should be small enough not to completely overlap.

DEF_GM(return new BlurRectFastGM( "blurrect_25_100_2_normal_fast", 25, 100, kSmall, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectFastGM("blurrect_25_100_20_normal_fast", 25, 100, kBig, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectSlowGM( "blurrect_25_100_2_normal_slow", 25, 100, kSmall, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectSlowGM("blurrect_25_100_20_normal_slow", 25, 100, kBig, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectFastGM( "blurrect_25_100_2_inner_fast", 25, 100, kSmall, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectFastGM("blurrect_25_100_20_inner_fast", 25, 100, kBig, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectSlowGM( "blurrect_25_100_2_inner_slow", 25, 100, kSmall, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectSlowGM("blurrect_25_100_20_inner_slow", 25, 100, kBig, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectFastGM( "blurrect_25_100_2_outer_fast", 25, 100, kSmall, SkBlurMask::kOuter_Style);)
DEF_GM(return new BlurRectFastGM("blurrect_25_100_20_outer_fast", 25, 100, kBig, SkBlurMask::kOuter_Style);)
DEF_GM(return new BlurRectSlowGM( "blurrect_25_100_2_outer_slow", 25, 100, kSmall, SkBlurMask::kOuter_Style);)
DEF_GM(return new BlurRectSlowGM("blurrect_25_100_20_outer_slow", 25, 100, kBig, SkBlurMask::kOuter_Style);)

// skinny tall rects, blurs overlap in X but not y

DEF_GM(return new BlurRectFastGM( "blurrect_5_100_2_normal_fast", 5, 100, kSmall, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectFastGM("blurrect_5_100_20_normal_fast", 5, 100, kBig, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectSlowGM( "blurrect_5_100_2_normal_slow", 5, 100, kSmall, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectSlowGM("blurrect_5_100_20_normal_slow", 5, 100, kBig, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectFastGM( "blurrect_5_100_2_inner_fast", 5, 100, kSmall, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectFastGM("blurrect_5_100_20_inner_fast", 5, 100, kBig, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectSlowGM( "blurrect_5_100_2_inner_slow", 5, 100, kSmall, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectSlowGM("blurrect_5_100_20_inner_slow", 5, 100, kBig, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectFastGM( "blurrect_5_100_2_outer_fast", 5, 100, kSmall, SkBlurMask::kOuter_Style);)
DEF_GM(return new BlurRectFastGM("blurrect_5_100_20_outer_fast", 5, 100, kBig, SkBlurMask::kOuter_Style);)
DEF_GM(return new BlurRectSlowGM( "blurrect_5_100_2_outer_slow", 5, 100, kSmall, SkBlurMask::kOuter_Style);)
DEF_GM(return new BlurRectSlowGM("blurrect_5_100_20_outer_slow", 5, 100, kBig, SkBlurMask::kOuter_Style);)

// tiny rects, blurs overlap in X and Y

DEF_GM(return new BlurRectFastGM( "blurrect_5_5_2_normal_fast", 5, 5, kSmall, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectFastGM("blurrect_5_5_20_normal_fast", 5, 5, kBig, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectSlowGM( "blurrect_5_5_2_normal_slow", 5, 5, kSmall, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectSlowGM("blurrect_5_5_20_normal_slow", 5, 5, kBig, SkBlurMask::kNormal_Style);)
DEF_GM(return new BlurRectFastGM( "blurrect_5_5_2_inner_fast", 5, 5, kSmall, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectFastGM("blurrect_5_5_20_inner_fast", 5, 5, kBig, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectSlowGM( "blurrect_5_5_2_inner_slow", 5, 5, kSmall, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectSlowGM("blurrect_5_5_20_inner_slow", 5, 5, kBig, SkBlurMask::kInner_Style);)
DEF_GM(return new BlurRectFastGM( "blurrect_5_5_2_outer_fast", 5, 5, kSmall, SkBlurMask::kOuter_Style);)
DEF_GM(return new BlurRectFastGM("blurrect_5_5_20_outer_fast", 5, 5, kBig, SkBlurMask::kOuter_Style);)
DEF_GM(return new BlurRectSlowGM( "blurrect_5_5_2_outer_slow", 5, 5, kSmall, SkBlurMask::kOuter_Style);)
DEF_GM(return new BlurRectSlowGM("blurrect_5_5_20_outer_slow", 5, 5, kBig, SkBlurMask::kOuter_Style);)


#if 0
// dont' need to GM the gaussian convolution; it's slow and intended
// as a ground truth comparison only.  Leaving these here in case we
// ever want to turn these back on for debugging reasons.
DEF_GM(return new BlurRectGroundTruthGM( "blurrect_25_100_1_simple", 25, 100, 1);)
DEF_GM(return new BlurRectGroundTruthGM( "blurrect_25_100_2_simple", 25, 100, 2);)
DEF_GM(return new BlurRectGroundTruthGM( "blurrect_25_100_3_simple", 25, 100, 3);)
DEF_GM(return new BlurRectGroundTruthGM( "blurrect_25_100_4_simple", 25, 100, 4);)
DEF_GM(return new BlurRectGroundTruthGM( "blurrect_25_100_5_simple", 25, 100, 5);)
DEF_GM(return new BlurRectGroundTruthGM( "blurrect_25_100_6_simple", 25, 100, 6);)
DEF_GM(return new BlurRectGroundTruthGM( "blurrect_25_100_7_simple", 25, 100, 7);)
DEF_GM(return new BlurRectGroundTruthGM( "blurrect_25_100_8_simple", 25, 100, 8);)
DEF_GM(return new BlurRectGroundTruthGM( "blurrect_25_100_9_simple", 25, 100, 9);)
DEF_GM(return new BlurRectGroundTruthGM("blurrect_25_100_10_simple", 25, 100, 10);)
DEF_GM(return new BlurRectGroundTruthGM("blurrect_25_100_11_simple", 25, 100, 11);)
DEF_GM(return new BlurRectGroundTruthGM("blurrect_25_100_12_simple", 25, 100, 12);)
DEF_GM(return new BlurRectGroundTruthGM("blurrect_25_100_13_simple", 25, 100, 13);)
DEF_GM(return new BlurRectGroundTruthGM("blurrect_25_100_14_simple", 25, 100, 14);)
DEF_GM(return new BlurRectGroundTruthGM("blurrect_25_100_15_simple", 25, 100, 15);)
DEF_GM(return new BlurRectGroundTruthGM("blurrect_25_100_16_simple", 25, 100, 16);)
DEF_GM(return new BlurRectGroundTruthGM("blurrect_25_100_17_simple", 25, 100, 17);)
DEF_GM(return new BlurRectGroundTruthGM("blurrect_25_100_18_simple", 25, 100, 18);)
DEF_GM(return new BlurRectGroundTruthGM("blurrect_25_100_19_simple", 25, 100, 19);)
#endif
