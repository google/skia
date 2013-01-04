/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMaskFilter.h"
#include "SkBlurMask.h"
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
               SkBlurMaskFilter::BlurStyle bs) :
        fMaskFilter(SkBlurMaskFilter::Create(STROKE_WIDTH/2, bs,
                                       SkBlurMaskFilter::kHighQuality_BlurFlag))
        , fName(name)
        , fPProc(pproc)
        , fAlpha(SkToU8(alpha))
    {
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

    typedef GM INHERITED;
};

class BlurRectCompareGM : public skiagm::GM {
    SkString  fName;
    unsigned int fRectWidth, fRectHeight;
    float fRadius;
public:
    BlurRectCompareGM(const char name[], unsigned int rectWidth, unsigned int rectHeight, float radius) :
        fName(name)
        , fRectWidth( rectWidth )
        , fRectHeight( rectHeight )
        , fRadius( radius )
    {}
    
  int width() const { return fRectWidth; }
  int height() const { return fRectHeight; }
  int radius() const { return fRadius; }

protected:
    virtual SkString onShortName() {
        return fName;
    }

    virtual SkISize onISize() {
        return SkISize::Make(640, 480);
    }
    
    virtual void makeMask( SkMask *m, SkRect r ) = 0;

    virtual void onDraw(SkCanvas* canvas) {
      SkRect r;
      r.setWH( fRectWidth, fRectHeight );

      SkMask mask;

      makeMask( &mask, r );

      SkBitmap bm;
      bm.setConfig(SkBitmap::kA8_Config, mask.fBounds.width(), mask.fBounds.height());
      bm.setPixels(mask.fImage);
      canvas->drawBitmap(bm, 50, 50, NULL);
    }

    virtual uint32_t onGetFlags() const { return kSkipPipe_Flag; }

private:
    typedef GM INHERITED;
};

class BlurRectFastGM: public BlurRectCompareGM {
public:
  BlurRectFastGM(const char name[], unsigned int rect_width, unsigned int rect_height, float blur_radius) :
    BlurRectCompareGM( name, rect_width, rect_height, blur_radius ) {}
protected:
  virtual void makeMask( SkMask *m, SkRect r ) {
    SkBlurMask::BlurRect( m, r, radius(), SkBlurMask::kNormal_Style, SkBlurMask::kHigh_Quality );
  }
};

class BlurRectSlowGM: public BlurRectCompareGM {
public:
  BlurRectSlowGM(const char name[], unsigned int rect_width, unsigned int rect_height, float blur_radius) :
    BlurRectCompareGM( name, rect_width, rect_height, blur_radius ) {}
protected:
  virtual void makeMask( SkMask *m, SkRect r ) {
    SkMask src;
    src.fFormat = SkMask::kA8_Format;
    src.fRowBytes = r.width();
    src.fBounds = SkIRect::MakeWH(r.width(), r.height());
    src.fImage = SkMask::AllocImage( src.computeTotalImageSize() );

    memset( src.fImage, 0xff, src.computeTotalImageSize() );

    SkBlurMask::BlurSeparable( m, src, radius()/2, SkBlurMask::kNormal_Style, SkBlurMask::kHigh_Quality );    
  }
};


//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BlurRectGM("blurrect", NULL, 0xFF, SkBlurMaskFilter::kNormal_BlurStyle);)
DEF_GM(return new BlurRectGM("blurrect", NULL, 0xFF, SkBlurMaskFilter::kSolid_BlurStyle);)
DEF_GM(return new BlurRectGM("blurrect", NULL, 0xFF, SkBlurMaskFilter::kOuter_BlurStyle);)
DEF_GM(return new BlurRectGM("blurrect", NULL, 0xFF, SkBlurMaskFilter::kInner_BlurStyle);)

DEF_GM(return new BlurRectFastGM("blurrect_fast_100_100_10", 100, 100, 10);)
DEF_GM(return new BlurRectFastGM("blurrect_fast_100_100_2", 100, 100, 2);)
DEF_GM(return new BlurRectFastGM("blurrect_fast_10_10_100", 10, 10, 100);)
DEF_GM(return new BlurRectFastGM("blurrect_fast_10_100_10", 10, 100, 10);)

DEF_GM(return new BlurRectSlowGM("blurrect_slow_100_100_10", 100, 100, 10);)
DEF_GM(return new BlurRectSlowGM("blurrect_slow_100_100_2", 100, 100, 2);)
DEF_GM(return new BlurRectSlowGM("blurrect_slow_10_10_100", 10, 10, 100);)
DEF_GM(return new BlurRectSlowGM("blurrect_slow_10_100_10", 10, 100, 10);)