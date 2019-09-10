/*
* Copyright 2012 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include <cmath>
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
#include "tools/timer/TimeUtils.h"

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

// Compares actual blur rects with reference masks created by the GM. Animates sigma in viewer.
class BlurRectCompareGM : public GM {
protected:
    SkString onShortName() override { return SkString("blurrect_compare"); }

    SkISize onISize() override { return {900, 1220}; }

    void onOnceBeforeDraw() override { this->prepareReferenceMasks(); }

    void onDraw(SkCanvas* canvas) override {
        int32_t ctxID = canvas->getGrContext() ? canvas->getGrContext()->priv().contextID() : 0;
        if (fRecalcMasksForAnimation || !fActualMasks[0][0][0] || ctxID != fLastContextUniqueID) {
            if (fRecalcMasksForAnimation) {
                // Sigma is changing so references must also be recalculated.
                this->prepareReferenceMasks();
            }
            this->prepareActualMasks(canvas);
            this->prepareMaskDifferences(canvas);
            fLastContextUniqueID = ctxID;
            fRecalcMasksForAnimation = false;
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
                auto sigma = kSigmas[sigmaIdx] + fSigmaAnimationBoost;
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
                                // The error images are opaque, use kPlus so they are additive if
                                // the overlap between test cases.
                                paint.setBlendMode(SkBlendMode::kPlus);
                                break;
                        }
                        auto pad = PadForSigma(sigma);
                        canvas->drawImage(img, -pad, -pad, &paint);
#if 0  // Uncomment to hairline stroke around blurred rect in red on top of the blur result.
       // The rect is defined at integer coords. We inset by 1/2 pixel so our stroke lies on top
       // of the edge pixels.
                        SkPaint stroke;
                        stroke.setColor(SK_ColorRED);
                        stroke.setStrokeWidth(0.f);
                        stroke.setStyle(SkPaint::kStroke_Style);
                        canvas->drawRect(SkRect::MakeWH(w, h).makeInset(0.5, 0.5), stroke);
#endif
                        canvas->translate(w + kMargin, 0.f);
                    }
                    canvas->restore();
                    canvas->translate(0, h + kMargin);
                }
            }
            canvas->restore();
            canvas->translate(totalW + 2 * kMargin, 0);
        }
    }
    bool onAnimate(double nanos) override {
        fSigmaAnimationBoost = TimeUtils::SineWave(nanos, 5, 2.5f, 0.f, 2.f);
        fRecalcMasksForAnimation = true;
        return true;
    }

private:
    void prepareReferenceMasks() {
        auto create_reference_mask = [](int w, int h, float sigma, int numSubpixels) {
            int pad = PadForSigma(sigma);
            int maskW = w + 2 * pad;
            int maskH = h + 2 * pad;
            // We'll do all our calculations at subpixel resolution, so adjust params
            w *= numSubpixels;
            h *= numSubpixels;
            sigma *= numSubpixels;
            auto scale = SK_ScalarRoot2Over2 / sigma;
            auto def_integral_approx = [scale](float a, float b) {
                return 0.5f * (std::erf(b * scale) - std::erf(a * scale));
            };
            // Do the x-pass. Above/below rect are rows of zero. All rows that intersect the rect
            // are the same. The row is calculated and stored at subpixel resolution.
            SkASSERT(!(numSubpixels & 0b1));
            std::unique_ptr<float[]> row(new float[maskW * numSubpixels]);
            for (int col = 0; col < maskW * numSubpixels; ++col) {
                // Compute distance to rect left in subpixel units
                float ldiff = numSubpixels * pad - (col + 0.5f);
                float rdiff = ldiff + w;
                row[col] = def_integral_approx(ldiff, rdiff);
            }
            // y-pass
            SkBitmap bmp;
            bmp.allocPixels(SkImageInfo::MakeA8(maskW, maskH));
            std::unique_ptr<float[]> accums(new float[maskW]);
            const float accumScale = 1.f / (numSubpixels * numSubpixels);
            for (int y = 0; y < maskH; ++y) {
                // Initialize subpixel accumulation buffer for this row.
                std::fill_n(accums.get(), maskW, 0);
                for (int ys = 0; ys < numSubpixels; ++ys) {
                    // At each subpixel we want to integrate over the kernel centered at the
                    // subpixel multiplied by the x-pass. The x-pass is zero above and below the
                    // rect and constant valued from rect top to rect bottom. So we can get the
                    // integral of just the kernel from rect top to rect bottom and multiply by
                    // the single x-pass value from our precomputed row.
                    float tdiff = numSubpixels * pad - (y * numSubpixels + ys + 0.5f);
                    float bdiff = tdiff + h;
                    auto w = def_integral_approx(tdiff, bdiff);
                    for (int x = 0; x < maskW; ++x) {
                        for (int xs = 0; xs < numSubpixels; ++xs) {
                            int rowIdx = x * numSubpixels + xs;
                            accums[x] += w * row[rowIdx];
                        }
                    }
                }
                for (int x = 0; x < maskW; ++x) {
                    auto result = accums[x] * accumScale;
                    *bmp.getAddr8(x, y) = SkToU8(sk_float_round2int(255.f * result));
                }
            }
            return SkImage::MakeFromBitmap(bmp);
        };

        // Number of times to subsample (in both X and Y). If fRecalcMasksForAnimation is true
        // then we're animating, don't subsample as much to keep fps higher.
        const int numSubpixels = fRecalcMasksForAnimation ? 2 : 8;

        for (size_t sigmaIdx = 0; sigmaIdx < kNumSigmas; ++sigmaIdx) {
            auto sigma = kSigmas[sigmaIdx] + fSigmaAnimationBoost;
            for (size_t heightIdx = 0; heightIdx < kNumSizes; ++heightIdx) {
                auto h = kSizes[heightIdx];
                for (size_t widthIdx = 0; widthIdx < kNumSizes; ++widthIdx) {
                    auto w = kSizes[widthIdx];
                    fReferenceMasks[sigmaIdx][heightIdx][widthIdx] =
                            create_reference_mask(w, h, sigma, numSubpixels);
                }
            }
        }
    }

    void prepareActualMasks(SkCanvas* canvas) {
        for (size_t sigmaIdx = 0; sigmaIdx < kNumSigmas; ++sigmaIdx) {
            auto sigma = kSigmas[sigmaIdx] + fSigmaAnimationBoost;
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

    void prepareMaskDifferences(SkCanvas* canvas) {
        for (size_t sigmaIdx = 0; sigmaIdx < kNumSigmas; ++sigmaIdx) {
            for (size_t heightIdx = 0; heightIdx < kNumSizes; ++heightIdx) {
                for (size_t widthIdx = 0; widthIdx < kNumSizes; ++widthIdx) {
                    const auto& r =  fReferenceMasks[sigmaIdx][heightIdx][widthIdx];
                    const auto& a =     fActualMasks[sigmaIdx][heightIdx][widthIdx];
                          auto& d = fMaskDifferences[sigmaIdx][heightIdx][widthIdx];
                    // The actual image might not be present if we're on an abandoned GrContext.
                    if (!a) {
                        d.reset();
                        continue;
                    }
                    SkASSERT(r->width()  == a->width());
                    SkASSERT(r->height() == a->height());
                    auto ii = SkImageInfo::Make(r->width(), r->height(),
                                                kRGBA_8888_SkColorType, kPremul_SkAlphaType);
                    auto surf = canvas->makeSurface(ii);
                    if (!surf) {
                        return;
                    }
                    // We visualize the difference by turning both the alpha masks into opaque green
                    // images (where alpha becomes the green channel) and then perform a
                    // SkBlendMode::kDifference between them.
                    SkPaint filterPaint;
                    filterPaint.setColor(SK_ColorWHITE);
                    // Actually 8 * alpha becomes green to really highlight differences.
                    static constexpr float kGreenifyM[] = {0, 0, 0, 0, 0,
                                                           0, 0, 0, 8, 0,
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
    static int PadForSigma(float sigma) { return sk_float_ceil2int(4 * sigma); }

    static constexpr int kSizes[] = {1, 2, 4, 8, 16, 32};
    static constexpr float kSigmas[] = {0.5f, 1.2f, 2.3f, 3.9f, 7.4f};
    static constexpr size_t kNumSizes = SK_ARRAY_COUNT(kSizes);
    static constexpr size_t kNumSigmas = SK_ARRAY_COUNT(kSigmas);

    sk_sp<SkImage> fReferenceMasks[kNumSigmas][kNumSizes][kNumSizes];
    sk_sp<SkImage> fActualMasks[kNumSigmas][kNumSizes][kNumSizes];
    sk_sp<SkImage> fMaskDifferences[kNumSigmas][kNumSizes][kNumSizes];
    int32_t fLastContextUniqueID;
    // These are used only when animating.
    float fSigmaAnimationBoost = 0;
    bool fRecalcMasksForAnimation = false;
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
