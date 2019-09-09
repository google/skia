/*
* Copyright 2012 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrContext.h"
#include "include/private/SkTo.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkMask.h"
#include "src/gpu/GrContextPriv.h"

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
public:
    BlurRectGM(const char name[], U8CPU alpha) : fName(name), fAlpha(SkToU8(alpha)) {}

private:
    sk_sp<SkMaskFilter> fMaskFilters[kLastEnum_SkBlurStyle + 1];
    const char* fName;
    SkAlpha fAlpha;

    void onOnceBeforeDraw() override {
        for (int i = 0; i <= kLastEnum_SkBlurStyle; ++i) {
            fMaskFilters[i] = SkMaskFilter::MakeBlur((SkBlurStyle)i,
                                  SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(STROKE_WIDTH/2)));
        }
    }

    SkString onShortName() override { return SkString(fName); }

    SkISize onISize() override { return {860, 820}; }

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

namespace skiagm {

class BlurRectCompareGM : public GM {
protected:
    SkString onShortName() override { return SkString("blurrect_compare"); }

    SkISize onISize() override { return {1200, 1024}; }

    void onOnceBeforeDraw() override { this->prepareReferenceMasks(); }

    void onDraw(SkCanvas* canvas) override {
        int32_t ctxID = canvas->getGrContext() ? canvas->getGrContext()->priv().contextID() : 0;
        if (!fActualMasks[0][0][0] || ctxID != fLastContextUniqueID) {
            this->prepareActualMasks(canvas);
            this->prepareMaskDiffernces(canvas);
            fLastContextUniqueID = ctxID;
        }
        canvas->clear(SK_ColorBLACK);
        static constexpr float kMargin = 30;
        float totalW = 0;
        for (auto w : kSizes) {
            totalW += w + kMargin;
        }
        canvas->translate(kMargin, kMargin);
        for (int mode = 0; mode < 3; ++mode) {
            canvas->save();
            for (size_t sigmaIdx = 0; sigmaIdx < kNumSigmas; ++sigmaIdx) {
                auto sigma = kSigmas[sigmaIdx];
                for (size_t heightIdx = 0; heightIdx < kNumSizes; ++heightIdx) {
                    auto h = kSizes[heightIdx];
                    canvas->save();
                    for (size_t widthIdx = 0; widthIdx < kNumSizes; ++widthIdx) {
                        auto w = kSizes[widthIdx];
                        SkPaint paint;
                        paint.setColor(SK_ColorWHITE);
                        SkImage* img;
                        switch (mode) {
                            case 0:
                                img = fReferenceMasks[sigmaIdx][heightIdx][widthIdx].get();
                                break;
                            case 1:
                                img = fActualMasks[sigmaIdx][heightIdx][widthIdx].get();
                                break;
                            case 2:
                                img = fMaskDifferences[sigmaIdx][heightIdx][widthIdx].get();
                                break;
                        }
                        auto pad = PadForSigma(sigma);
                        canvas->drawImage(img, -pad, -pad, &paint);
#if 0
                        SkPaint stroke;
                        stroke.setColor(SK_ColorRED);
                        stroke.setStrokeWidth(1.f);
                        stroke.setStyle(SkPaint::kStroke_Style);
                        canvas->drawRect(SkRect::MakeWH(w, h), stroke);
#endif
                        canvas->translate(w + kMargin, 0.f);
                    }
                    canvas->restore();
                    canvas->translate(0, h + kMargin);
                }
            }
            canvas->restore();
            canvas->translate(totalW, 0);
        }
    }
private:
    void prepareReferenceMasks() {
        // Number of times to subsample (in both X and Y)
        static constexpr int kNumSubsamples = 8;
        auto create_reference_mask = [](int w, int h, float sigma) {
          int pad = PadForSigma(sigma);
          int maskW = w + 2 * pad;
          int maskH = h + 2 * pad;
          // We'll do all our calculations at subpixel resolution, so adjust params
          w *= kNumSubsamples;
          h *= kNumSubsamples;
          sigma *= kNumSubsamples;
          auto def_integral_approx = [scale = 1.f / (M_SQRT2 * sigma)](float a, float b) {
            return 0.5f * (std::erf(b * scale) - std::erf(a * scale));
          };
          // Do the x-pass. Above/below rect are rows of zero. All rows that intersect the rect are
          // the same. We evaluate kNumSubsamples subpixels for each pixel in x.
          std::unique_ptr<float[]> row(new float[maskW * kNumSubsamples]);
          for (int col = 0; col < maskW * kNumSubsamples; ++col) {
              // Compute distance to rect left in subpixel units
              float ldiff = kNumSubsamples * pad - col + 0.5f;
              float rdiff = ldiff + w;
              row[col] = def_integral_approx(ldiff, rdiff);
          }
          SkBitmap bmp;
          bmp.allocPixels(SkImageInfo::MakeA8(maskW, maskH));
          for (int x = 0; x < maskW; ++x) {
              for (int y = 0; y < maskH; ++y) {
                  float accum = 0;
                  for (int ys = 0; ys < kNumSubsamples; ++ys) {
                      // compute distance to rect top in subpixel units.
                      float tdiff = kNumSubsamples * pad - (y * kNumSubsamples + ys) + 0.5f;
                      float bdiff = tdiff + h;
                      auto w = def_integral_approx(tdiff, bdiff);
                      for (int xs = 0; xs < kNumSubsamples; ++xs) {
                          int rowIdx = x * kNumSubsamples + xs;
                          accum += w * row[rowIdx];
                      }
                  }
                  auto pixelResult = accum / kNumSubsamples / kNumSubsamples;
                  *bmp.getAddr8(x, y) = SkToU8(sk_float_round2int(255.f * pixelResult));
              }
          }
          return SkImage::MakeFromBitmap(bmp);
        };
        for (size_t sigmaIdx = 0; sigmaIdx < kNumSigmas; ++sigmaIdx) {
            auto sigma = kSigmas[sigmaIdx];
            for (size_t heightIdx = 0; heightIdx < kNumSizes; ++heightIdx) {
                auto h = kSizes[heightIdx];
                for (size_t widthIdx = 0; widthIdx < kNumSizes; ++widthIdx) {
                    auto w = kSizes[widthIdx];
                    fReferenceMasks[sigmaIdx][heightIdx][widthIdx] =
                            create_reference_mask(w, h, sigma);
                }
            }
        }
    }

    void prepareActualMasks(SkCanvas* canvas) {
        for (size_t sigmaIdx = 0; sigmaIdx < kNumSigmas; ++sigmaIdx) {
            auto sigma = kSigmas[sigmaIdx];
            for (size_t heightIdx = 0; heightIdx < kNumSizes; ++heightIdx) {
                auto h = kSizes[heightIdx];
                for (size_t widthIdx = 0; widthIdx < kNumSizes; ++widthIdx) {
                    auto w = kSizes[widthIdx];
                    auto pad = PadForSigma(sigma);
                    auto ii = SkImageInfo::MakeA8(w + 2 * pad, h + 2 * pad);
                    auto surf = canvas->makeSurface(ii);
                    if (!surf) {
                        return;
                    }
                    auto rect = SkRect::MakeXYWH(pad, pad, w, h);
                    SkPaint paint;
                    paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, sigma));
                    surf->getCanvas()->drawRect(rect, paint);
                    fActualMasks[sigmaIdx][heightIdx][widthIdx] = surf->makeImageSnapshot();
                }
            }
        }
    }

    void prepareMaskDiffernces(SkCanvas* canvas) {
        for (size_t sigmaIdx = 0; sigmaIdx < kNumSigmas; ++sigmaIdx) {
            for (size_t heightIdx = 0; heightIdx < kNumSizes; ++heightIdx) {
                for (size_t widthIdx = 0; widthIdx < kNumSizes; ++widthIdx) {
                    const auto& r =  fReferenceMasks[sigmaIdx][heightIdx][widthIdx];
                    const auto& a =     fActualMasks[sigmaIdx][heightIdx][widthIdx];
                          auto& d = fMaskDifferences[sigmaIdx][heightIdx][widthIdx];
                    SkASSERT(r->width()  == a->width());
                    SkASSERT(r->height() == a->height());
                    auto ii = SkImageInfo::Make(r->width(), r->height(),
                                                kRGBA_8888_SkColorType, kPremul_SkAlphaType);
                    auto surf = canvas->makeSurface(ii);
                    if (!surf) {
                        return;
                    }
                    // We visualize the difference by turning both the alpha masks into opaque green
                    // images (where alpha becomes the green channel) and then performing a
                    // SkBlendMode::kDifference between them.
                    SkPaint filterPaint;
                    filterPaint.setColor(SK_ColorWHITE);
                    static constexpr float kGreenifyM[] = {0, 0, 0, 0, 0,
                                                           0, 0, 0, 5, 0,
                                                           0, 0, 0, 0, 0,
                                                           0, 0, 0, 0, 1};
                    auto greenifyCF = SkColorFilters::Matrix(kGreenifyM);
                    SkPaint paint;
                    paint.setBlendMode(SkBlendMode::kSrc);
                    paint.setColorFilter(std::move(greenifyCF));
                    surf->getCanvas()->drawImage(a, 0, 0, &paint);
                    paint.setBlendMode(SkBlendMode::kDifference);
                    surf->getCanvas()->drawImage(r, 0, 0, &paint);
                    d = surf->makeImageSnapshot();
                }
            }
        }
    }

    // Per side padding around mask images for a sigma. Make this overly generous to ensure bugs
    // related to big blurs are fully visible.
    static int PadForSigma(float sigma) { return sk_float_ceil2int(3 * sigma); }

    static constexpr int kSizes[] = {1, 2, 4, 8, 16, 32};
    static constexpr float kSigmas[] = {0.5, 1, 2, 4, 8};
    static constexpr size_t kNumSizes = SK_ARRAY_COUNT(kSizes);
    static constexpr size_t kNumSigmas = SK_ARRAY_COUNT(kSigmas);

    sk_sp<SkImage> fReferenceMasks[kNumSigmas][kNumSizes][kNumSizes];
    sk_sp<SkImage> fActualMasks[kNumSigmas][kNumSizes][kNumSizes];
    sk_sp<SkImage> fMaskDifferences[kNumSigmas][kNumSizes][kNumSizes];
    int32_t fLastContextUniqueID;
};

// Delete these when C++17.
constexpr int BlurRectCompareGM::kSizes[];
constexpr float BlurRectCompareGM::kSigmas[];
constexpr size_t BlurRectCompareGM::kNumSizes;
constexpr size_t BlurRectCompareGM::kNumSigmas;

}  // namespace skiagm

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BlurRectGM("blurrects", 0xFF);)
DEF_GM(return new skiagm::BlurRectCompareGM();)
