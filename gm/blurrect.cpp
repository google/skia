/*
* Copyright 2012 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPath.h"
#include "include/private/SkTo.h"
#include "src/core/SkBlurMask.h"

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

#include "include/effects/SkGradientShader.h"

/*
 * Spits out a dummy gradient to test blur with shader on paint
 */
static sk_sp<SkShader> make_radial() {
    SkPoint pts[2] = {
        { 0, 0 },
        { SkIntToScalar(100), SkIntToScalar(100) }
    };
    SkTileMode tm = SkTileMode::kClamp;
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
    return SkGradientShader::MakeTwoPointConical(center1, (pts[1].fX - pts[0].fX) / 7,
                                                 center0, (pts[1].fX - pts[0].fX) / 2,
                                                 colors, pos, SK_ARRAY_COUNT(colors), tm,
                                                 0, &scale);
}

typedef void (*PaintProc)(SkPaint*, SkScalar width);

class BlurRectGM : public skiagm::GM {
      sk_sp<SkMaskFilter> fMaskFilters[kLastEnum_SkBlurStyle + 1];
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
            fMaskFilters[i] = SkMaskFilter::MakeBlur((SkBlurStyle)i,
                                  SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(STROKE_WIDTH/2)));
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
                paintWithRadial.setShader(make_radial());

                constexpr Proc procs[] = {
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

DEF_SIMPLE_GM(blurrect_gallery, canvas, 1200, 1024) {
        const int fGMWidth = 1200;
        const int fPadding = 10;
        const int fMargin = 100;

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
                    if (!SkBlurMask::BlurRect(SkBlurMask::ConvertRadiusToSigma(radius),
                                              &mask, r, style)) {
                        continue;
                    }

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
                    canvas->drawBitmap(bm, 0.f, 0.f, nullptr);
                    canvas->restore();

                    cur_x += bm.width() + fPadding;
                    if (bm.height() > max_height)
                        max_height = bm.height();
                }
            }
        }
}

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BlurRectGM("blurrects", 0xFF);)
