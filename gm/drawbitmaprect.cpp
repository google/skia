/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrDirectContext.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkBlurMask.h"
#include "tools/GpuToolUtils.h"
#include "tools/ToolUtils.h"

static SkBitmap make_chessbm(int w, int h) {
    SkBitmap bm;
    bm.allocN32Pixels(w, h);

    for (int y = 0; y < bm.height(); y++) {
        uint32_t* p = bm.getAddr32(0, y);
        for (int x = 0; x < bm.width(); x++) {
            p[x] = ((x + y) & 1) ? SK_ColorWHITE : SK_ColorBLACK;
        }
    }
    bm.setImmutable();
    return bm;
}

// Creates a bitmap and a matching image.
static sk_sp<SkImage> makebm(SkCanvas* origCanvas, SkBitmap* resultBM, int w, int h) {
    SkImageInfo info = SkImageInfo::MakeN32Premul(w, h);

    auto      surface(ToolUtils::makeSurface(origCanvas, info));
    SkCanvas* canvas = surface->getCanvas();

    canvas->clear(SK_ColorTRANSPARENT);

    SkScalar wScalar = SkIntToScalar(w);
    SkScalar hScalar = SkIntToScalar(h);

    SkPoint     pt = { wScalar / 2, hScalar / 2 };

    SkScalar    radius = 4 * std::max(wScalar, hScalar);

    SkColor     colors[] = { SK_ColorRED, SK_ColorYELLOW,
                             SK_ColorGREEN, SK_ColorMAGENTA,
                             SK_ColorBLUE, SK_ColorCYAN,
                             SK_ColorRED};

    SkScalar    pos[] = {0,
                         SK_Scalar1 / 6,
                         2 * SK_Scalar1 / 6,
                         3 * SK_Scalar1 / 6,
                         4 * SK_Scalar1 / 6,
                         5 * SK_Scalar1 / 6,
                         SK_Scalar1};

    SkPaint     paint;
    SkRect rect = SkRect::MakeWH(wScalar, hScalar);
    SkMatrix mat = SkMatrix::I();
    for (int i = 0; i < 4; ++i) {
        paint.setShader(SkGradientShader::MakeRadial(
                        pt, radius,
                        colors, pos,
                        std::size(colors),
                        SkTileMode::kRepeat,
                        0, &mat));
        canvas->drawRect(rect, paint);
        rect.inset(wScalar / 8, hScalar / 8);
        mat.postScale(SK_Scalar1 / 4, SK_Scalar1 / 4);
    }

    auto image = surface->makeImageSnapshot();

    SkBitmap tempBM;

    image->asLegacyBitmap(&tempBM);

    // Let backends know we won't change this, so they don't have to deep copy it defensively.
    tempBM.setImmutable();
    *resultBM = tempBM;

    return image;
}

static void bitmapproc(SkCanvas* canvas, sk_sp<SkImage> image,
                       const SkBitmap&, const SkIRect& srcR,
                       const SkRect& dstR, const SkSamplingOptions& sampling,
                       const SkPaint* paint) {
    canvas->drawImageRect(image, SkRect::Make(srcR), dstR, sampling, paint,
                          SkCanvas::kStrict_SrcRectConstraint);
}

static void bitmapsubsetproc(SkCanvas* canvas, sk_sp<SkImage> image,
                             const SkBitmap& bm, const SkIRect& srcR,
                             const SkRect& dstR, const SkSamplingOptions& sampling,
                             const SkPaint* paint) {
    if (!bm.bounds().contains(srcR)) {
        bitmapproc(canvas, std::move(image), bm, srcR, dstR, sampling, paint);
        return;
    }

    SkBitmap subset;
    if (bm.extractSubset(&subset, srcR)) {
        sk_sp<SkImage> subsetImg = ToolUtils::MakeTextureImage(canvas, subset.asImage());
        canvas->drawImageRect(subsetImg, dstR, sampling, paint);
    }
}

static void imageproc(SkCanvas* canvas, sk_sp<SkImage> image, const SkBitmap&, const SkIRect& srcR,
                      const SkRect& dstR, const SkSamplingOptions& sampling, const SkPaint* paint) {
    sk_sp<SkImage> tmp = ToolUtils::MakeTextureImage(canvas, std::move(image));
    canvas->drawImageRect(tmp, SkRect::Make(srcR), dstR, sampling, paint,
                          SkCanvas::kStrict_SrcRectConstraint);
}

static void imagesubsetproc(SkCanvas* canvas, sk_sp<SkImage> image, const SkBitmap& bm,
                            const SkIRect& srcR, const SkRect& dstR,
                            const SkSamplingOptions& sampling, const SkPaint* paint) {
    SkASSERT_RELEASE(image);
    if (!image->bounds().contains(srcR)) {
        imageproc(canvas, std::move(image), bm, srcR, dstR, sampling, paint);
        return;
    }

    auto direct = GrAsDirectContext(canvas->recordingContext());
    if (sk_sp<SkImage> subset = image->makeSubset(direct, srcR)) {
        canvas->drawImageRect(subset, dstR, sampling, paint);
        return;
    }
#if defined(SK_GRAPHITE)
    if (sk_sp<SkImage> subset = image->makeSubset(canvas->recorder(), srcR, {})) {
        canvas->drawImageRect(subset, dstR, sampling, paint);
    }
#endif
}

typedef void DrawRectRectProc(SkCanvas*, sk_sp<SkImage>, const SkBitmap&,
                              const SkIRect& srcR, const SkRect& dstR,
                              const SkSamplingOptions&, const SkPaint*);

constexpr int gSize = 1024;
constexpr int gBmpSize = 2048;

class DrawBitmapRectGM : public skiagm::GM {
public:
    DrawBitmapRectGM(DrawRectRectProc proc, const char suffix[]) : fProc(proc) {
        fName.set("drawbitmaprect");
        if (suffix) {
            fName.append(suffix);
        }
    }

    DrawRectRectProc*   fProc;
    SkBitmap            fLargeBitmap;
    sk_sp<SkImage>      fImage;
    SkString            fName;

protected:
    SkString getName() const override { return fName; }

    SkISize getISize() override { return SkISize::Make(gSize, gSize); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!fImage || !fImage->isValid(canvas->recordingContext())) {
            fImage = ToolUtils::MakeTextureImage(canvas,
                                                 makebm(canvas, &fLargeBitmap, gBmpSize, gBmpSize));
            if (!fImage) {
                *errorMsg = "Image creation failed";
                return DrawResult::kSkip;
            }
        }

        SkRect dstRect = { 0, 0, SkIntToScalar(64), SkIntToScalar(64)};
        const int kMaxSrcRectSize = 1 << (SkNextLog2(gBmpSize) + 2);

        const int kPadX = 30;
        const int kPadY = 40;
        SkPaint alphaPaint;
        alphaPaint.setAlphaf(0.125f);
        canvas->drawImageRect(fImage, SkRect::MakeIWH(gSize, gSize), SkSamplingOptions(),
                              &alphaPaint);
        canvas->translate(SK_Scalar1 * kPadX / 2,
                          SK_Scalar1 * kPadY / 2);
        SkPaint blackPaint;
        SkScalar titleHeight = SK_Scalar1 * 24;
        blackPaint.setColor(SK_ColorBLACK);
        blackPaint.setAntiAlias(true);

        SkFont font(ToolUtils::create_portable_typeface(), titleHeight);

        SkString title;
        title.printf("Bitmap size: %d x %d", gBmpSize, gBmpSize);
        canvas->drawString(title, 0, titleHeight, font, blackPaint);

        canvas->translate(0, SK_Scalar1 * kPadY / 2  + titleHeight);
        int rowCount = 0;
        canvas->save();
        for (int w = 1; w <= kMaxSrcRectSize; w *= 4) {
            for (int h = 1; h <= kMaxSrcRectSize; h *= 4) {

                SkIRect srcRect = SkIRect::MakeXYWH((gBmpSize - w) / 2, (gBmpSize - h) / 2, w, h);
                fProc(canvas, fImage, fLargeBitmap, srcRect, dstRect, SkSamplingOptions(),
                      nullptr);

                SkString label;
                label.appendf("%d x %d", w, h);
                blackPaint.setAntiAlias(true);
                blackPaint.setStyle(SkPaint::kFill_Style);
                font.setSize(SK_Scalar1 * 10);
                SkScalar baseline = dstRect.height() + font.getSize() + SK_Scalar1 * 3;
                canvas->drawString(label, 0, baseline, font, blackPaint);
                blackPaint.setStyle(SkPaint::kStroke_Style);
                blackPaint.setStrokeWidth(SK_Scalar1);
                blackPaint.setAntiAlias(false);
                canvas->drawRect(dstRect, blackPaint);

                canvas->translate(dstRect.width() + SK_Scalar1 * kPadX, 0);
                ++rowCount;
                if ((dstRect.width() + kPadX) * rowCount > gSize) {
                    canvas->restore();
                    canvas->translate(0, dstRect.height() + SK_Scalar1 * kPadY);
                    canvas->save();
                    rowCount = 0;
                }
            }
        }

        {
            // test the following code path:
            // SkGpuDevice::drawPath() -> SkGpuDevice::drawWithMaskFilter()
            SkIRect srcRect;
            SkPaint maskPaint;
            SkBitmap bm = make_chessbm(5, 5);
            sk_sp<SkImage> img = ToolUtils::MakeTextureImage(canvas, bm.asImage());

            srcRect.setXYWH(1, 1, 3, 3);
            maskPaint.setMaskFilter(SkMaskFilter::MakeBlur(
                kNormal_SkBlurStyle,
                SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(5))));

            fProc(canvas, img, bm, srcRect, dstRect,
                  SkSamplingOptions(SkFilterMode::kLinear), &maskPaint);
        }

        return DrawResult::kOk;
    }

private:
    using INHERITED = skiagm::GM;
};

DEF_GM( return new DrawBitmapRectGM(bitmapproc      , nullptr); )
DEF_GM( return new DrawBitmapRectGM(bitmapsubsetproc, "-subset"); )
DEF_GM( return new DrawBitmapRectGM(imageproc       , "-imagerect"); )
DEF_GM( return new DrawBitmapRectGM(imagesubsetproc , "-imagerect-subset"); )
