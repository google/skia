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

/*
 * Spits out a dummy gradient to test blur with shader on paint
 */
static SkShader* MakeRadial() {
    SkPoint pts[2] = {
        { 0, 0 },
        { SkIntToScalar(100), SkIntToScalar(100) }
    };
    SkShader::TileMode tm = SkShader::kClamp_TileMode;
    const SkColor colors[] = { SK_ColorRED, SK_ColorGREEN, };
    const SkScalar pos[] = { SK_Scalar1/4, SK_Scalar1*3/4 };
    SkMatrix scale;
    scale.setScale(0.5f, 0.5f);
    scale.postTranslate(25.f, 25.f);
    SkPoint center0, center1;
    center0.set(SkScalarAve(pts[0].fX, pts[1].fX),
                SkScalarAve(pts[0].fY, pts[1].fY));
    center1.set(SkScalarInterp(pts[0].fX, pts[1].fX, SkIntToScalar(3)/5),
                SkScalarInterp(pts[0].fY, pts[1].fY, SkIntToScalar(1)/4));
    return SkGradientShader::CreateTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 7,
                                                  center0, (pts[1].fX - pts[0].fX) / 2,
                                                  colors, pos, SK_ARRAY_COUNT(colors), tm,
                                                  0, &scale);
}

typedef void (*PaintProc)(SkPaint*, SkScalar width);

class BlurRectGM : public skiagm::GM {
      SkAutoTUnref<SkMaskFilter> fMaskFilters[kLastEnum_SkBlurStyle + 1];
      SkString  fName;
      SkAlpha   fAlpha;
public:
    BlurRectGM(const char name[], U8CPU alpha)
        : fName(name)
        , fAlpha(SkToU8(alpha)) {
    }

protected:
    void onOnceBeforeDraw() override {
        for (int i = 0; i <= kLastEnum_SkBlurStyle; ++i) {
            fMaskFilters[i].reset(SkBlurMaskFilter::Create((SkBlurStyle)i,
                                  SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(STROKE_WIDTH/2)),
                                  SkBlurMaskFilter::kHighQuality_BlurFlag));
        }
    }

    SkString onShortName() override {
        return fName;
    }

    SkISize onISize() override {
        return SkISize::Make(860, 820);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(STROKE_WIDTH*3/2, STROKE_WIDTH*3/2);

        SkRect  r = { 0, 0, 100, 50 };
        SkScalar scales[] = { SK_Scalar1, 0.6f };

        for (size_t s = 0; s < SK_ARRAY_COUNT(scales); ++s) {
            canvas->save();
            for (size_t f = 0; f < SK_ARRAY_COUNT(fMaskFilters); ++f) {
                SkPaint paint;
                paint.setMaskFilter(fMaskFilters[f]);
                paint.setAlpha(fAlpha);

                SkPaint paintWithRadial = paint;
                paintWithRadial.setShader(MakeRadial())->unref();

                static const Proc procs[] = {
                    fill_rect, draw_donut, draw_donut_skewed
                };

                canvas->save();
                canvas->scale(scales[s], scales[s]);
                this->drawProcs(canvas, r, paint, false, procs, SK_ARRAY_COUNT(procs));
                canvas->translate(r.width() * 4/3, 0);
                this->drawProcs(canvas, r, paintWithRadial, false, procs, SK_ARRAY_COUNT(procs));
                canvas->translate(r.width() * 4/3, 0);
                this->drawProcs(canvas, r, paint, true, procs, SK_ARRAY_COUNT(procs));
                canvas->translate(r.width() * 4/3, 0);
                this->drawProcs(canvas, r, paintWithRadial, true, procs, SK_ARRAY_COUNT(procs));
                canvas->restore();

                canvas->translate(0, SK_ARRAY_COUNT(procs) * r.height() * 4/3 * scales[s]);
            }
            canvas->restore();
            canvas->translate(4 * r.width() * 4/3 * scales[s], 0);
        }
    }

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


class BlurRectDirectGM : public skiagm::GM {
    SkString  fName;
    int fGMWidth, fGMHeight;
    int fPadding, fMargin;
public:
    BlurRectDirectGM(const char name[])
        : fName(name),
          fGMWidth(1200),
          fGMHeight(1024),
          fPadding(10),
          fMargin(100)
    {
    }

protected:
    virtual SkString onShortName() {
        return fName;
    }

    virtual SkISize onISize() {
        return SkISize::Make(fGMWidth, fGMHeight);
    }

    virtual void onDraw(SkCanvas* canvas) {
        const int widths[] = {25, 5, 5, 100, 150, 25};
        const int heights[] = {100, 100, 5, 25, 150, 25};
        const SkBlurStyle styles[] = {kNormal_SkBlurStyle, kInner_SkBlurStyle, kOuter_SkBlurStyle};
        const float radii[] = {20, 5, 10};

        canvas->translate(50,20);

        int cur_x = 0;
        int cur_y = 0;

        int max_height = 0;

        for (size_t i = 0 ; i < SK_ARRAY_COUNT(widths) ; i++) {
            int width = widths[i];
            int height = heights[i];
            SkRect r;
            r.setWH(SkIntToScalar(width), SkIntToScalar(height));
            SkAutoCanvasRestore autoRestore(canvas, true);

            for (size_t j = 0 ; j < SK_ARRAY_COUNT(radii) ; j++) {
                float radius = radii[j];
                for (size_t k = 0 ; k < SK_ARRAY_COUNT(styles) ; k++) {
                    SkBlurStyle style = styles[k];

                    SkMask mask;
                    SkBlurMask::BlurRect(SkBlurMask::ConvertRadiusToSigma(radius), &mask, r, style);

                    SkAutoMaskFreeImage amfi(mask.fImage);

                    SkBitmap bm;
                    bm.installMaskPixels(mask);

                    if (cur_x + bm.width() >= fGMWidth - fMargin) {
                        cur_x = 0;
                        cur_y += max_height + fPadding;
                        max_height = 0;
                    }

                    canvas->save();
                    canvas->translate((SkScalar)cur_x, (SkScalar)cur_y);
                    canvas->translate(-(bm.width() - r.width())/2, -(bm.height()-r.height())/2);
                    canvas->drawBitmap(bm, 0.f, 0.f, NULL);
                    canvas->restore();

                    cur_x += bm.width() + fPadding;
                    if (bm.height() > max_height)
                        max_height = bm.height();
                }
            }
        }
    }

private:
    typedef GM INHERITED;
};

class BlurRectCompareGM : public skiagm::GM {
    SkString  fName;
    unsigned int fRectWidth, fRectHeight;
    SkScalar fRadius;
    SkBlurStyle fStyle;
public:
    BlurRectCompareGM(const char name[], unsigned int rectWidth,
                      unsigned int rectHeight, float radius,
                      SkBlurStyle style)
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
    SkBlurStyle style() const {
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
        bm.installMaskPixels(mask);

        center_x = (canvas_size.fWidth - mask.fBounds.width())/2;
        center_y = (canvas_size.fHeight - mask.fBounds.height())/2;

        canvas->drawBitmap(bm, SkIntToScalar(center_x), SkIntToScalar(center_y), NULL);
    }

private:
    typedef GM INHERITED;
};

class BlurRectFastGM: public BlurRectCompareGM {
public:
    BlurRectFastGM(const char name[], unsigned int rectWidth,
                   unsigned int rectHeight, float blurRadius,
                   SkBlurStyle style) :
        INHERITED(name, rectWidth, rectHeight, blurRadius, style) {
        }

protected:
    bool makeMask(SkMask *m, const SkRect& r) override {
        return SkBlurMask::BlurRect(SkBlurMask::ConvertRadiusToSigma(this->radius()),
                                    m, r, this->style());
    }
private:
    typedef BlurRectCompareGM INHERITED;
};

class BlurRectSlowGM: public BlurRectCompareGM {
public:
    BlurRectSlowGM(const char name[], unsigned int rectWidth, unsigned int rectHeight,
                   float blurRadius, SkBlurStyle style)
        : INHERITED(name, rectWidth, rectHeight, blurRadius, style) {
        }

protected:
    bool makeMask(SkMask *m, const SkRect& r) override {
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

    virtual SkBlurQuality getQuality() {
        return kHigh_SkBlurQuality;
    }
private:
    typedef BlurRectCompareGM INHERITED;
};

class BlurRectSlowLowGM: public BlurRectSlowGM {
public:
    BlurRectSlowLowGM(const char name[], unsigned int rectWidth, unsigned int rectHeight,
                      float blurRadius, SkBlurStyle style)
        : INHERITED(name, rectWidth, rectHeight, blurRadius, style) {
        }

protected:
    SkBlurQuality getQuality() override {
        return kLow_SkBlurQuality;
    }
private:
    typedef BlurRectSlowGM INHERITED;
};

class BlurRectGroundTruthGM: public BlurRectCompareGM {
public:
    BlurRectGroundTruthGM(const char name[], unsigned int rectWidth, unsigned int rectHeight,
                          float blurRadius, SkBlurStyle style)
        : INHERITED(name, rectWidth, rectHeight, blurRadius, style) {
        }

protected:
    bool makeMask(SkMask *m, const SkRect& r) override {
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

    virtual SkBlurQuality getQuality() {
        return kHigh_SkBlurQuality;
    }
private:
    typedef BlurRectCompareGM INHERITED;
};


//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BlurRectGM("blurrects", 0xFF);)
DEF_GM(return new BlurRectDirectGM("blurrect_gallery");)
