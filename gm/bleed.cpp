/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFilterQuality.h"
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
#include "include/core/SkTypes.h"
#include "include/gpu/GrContextOptions.h"
#include "include/private/SkTDArray.h"
#include "src/core/SkBlurMask.h"
#include "tools/ToolUtils.h"

/** Creates an image with two one-pixel wide borders around a checkerboard. The checkerboard is 2x2
    checks where each check has as many pixels as is necessary to fill the interior. It returns
    the image and a src rect that bounds the checkerboard portion. */
std::tuple<sk_sp<SkImage>, SkRect> make_ringed_image(int width, int height) {

    // These are kRGBA_8888_SkColorType values.
    static constexpr uint32_t kOuterRingColor = 0xFFFF0000,
                              kInnerRingColor = 0xFF0000FF,
                              kCheckColor1    = 0xFF000000,
                              kCheckColor2    = 0xFFFFFFFF;

    SkASSERT(0 == width % 2 && 0 == height % 2);
    SkASSERT(width >= 6 && height >= 6);

    SkImageInfo info = SkImageInfo::Make(width, height, kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType);
    size_t rowBytes = SkAlign4(info.minRowBytes());
    SkBitmap bitmap;
    bitmap.allocPixels(info, rowBytes);

    uint32_t* scanline = bitmap.getAddr32(0, 0);
    for (int x = 0; x < width; ++x) {
        scanline[x] = kOuterRingColor;
    }
    scanline = bitmap.getAddr32(0, 1);
    scanline[0] = kOuterRingColor;
    for (int x = 1; x < width - 1; ++x) {
        scanline[x] = kInnerRingColor;
    }
    scanline[width - 1] = kOuterRingColor;

    for (int y = 2; y < height / 2; ++y) {
        scanline = bitmap.getAddr32(0, y);
        scanline[0] = kOuterRingColor;
        scanline[1] = kInnerRingColor;
        for (int x = 2; x < width / 2; ++x) {
            scanline[x] = kCheckColor1;
        }
        for (int x = width / 2; x < width - 2; ++x) {
            scanline[x] = kCheckColor2;
        }
        scanline[width - 2] = kInnerRingColor;
        scanline[width - 1] = kOuterRingColor;
    }

    for (int y = height / 2; y < height - 2; ++y) {
        scanline = bitmap.getAddr32(0, y);
        scanline[0] = kOuterRingColor;
        scanline[1] = kInnerRingColor;
        for (int x = 2; x < width / 2; ++x) {
            scanline[x] = kCheckColor2;
        }
        for (int x = width / 2; x < width - 2; ++x) {
            scanline[x] = kCheckColor1;
        }
        scanline[width - 2] = kInnerRingColor;
        scanline[width - 1] = kOuterRingColor;
    }

    scanline = bitmap.getAddr32(0, height - 2);
    scanline[0] = kOuterRingColor;
    for (int x = 1; x < width - 1; ++x) {
        scanline[x] = kInnerRingColor;
    }
    scanline[width - 1] = kOuterRingColor;

    scanline = bitmap.getAddr32(0, height - 1);
    for (int x = 0; x < width; ++x) {
        scanline[x] = kOuterRingColor;
    }
    bitmap.setImmutable();
    return {bitmap.asImage(), SkRect::Make({2, 2, width - 2, height - 2})};
}

/**
 * These GMs exercise the behavior of the drawImageRect and its SrcRectConstraint parameter. They
 * tests various matrices, filter qualities, and interaction with mask filters. They also exercise
 * the tiling image draws of SkGpuDevice by overriding the maximum texture size of the GrContext.
 */
class SrcRectConstraintGM : public skiagm::GM {
public:
    SrcRectConstraintGM(const char* shortName, SkCanvas::SrcRectConstraint constraint, bool batch)
        : fShortName(shortName)
        , fConstraint(constraint)
        , fBatch(batch) {
        // Make sure GPU SkSurfaces can be created for this GM.
        SkASSERT(this->onISize().width() <= kMaxTextureSize &&
                 this->onISize().height() <= kMaxTextureSize);
    }

protected:
    SkString onShortName() override { return fShortName; }
    SkISize onISize() override { return SkISize::Make(800, 1000); }

    void drawImage(SkCanvas* canvas, sk_sp<SkImage> image, SkRect srcRect, SkRect dstRect,
                   const SkSamplingOptions& sampling, SkPaint* paint) {
        if (fBatch) {
            SkCanvas::ImageSetEntry imageSetEntry[1];
            imageSetEntry[0].fImage = image;
            imageSetEntry[0].fSrcRect = srcRect;
            imageSetEntry[0].fDstRect = dstRect;
            imageSetEntry[0].fAAFlags = paint->isAntiAlias() ? SkCanvas::kAll_QuadAAFlags
                                                             : SkCanvas::kNone_QuadAAFlags;
            canvas->experimental_DrawEdgeAAImageSet(imageSetEntry, SK_ARRAY_COUNT(imageSetEntry),
                                                    /*dstClips=*/nullptr,
                                                    /*preViewMatrices=*/nullptr,
                                                    sampling, paint, fConstraint);
        } else {
            canvas->drawImageRect(image.get(), srcRect, dstRect, sampling, paint, fConstraint);
        }
    }

    // Draw the area of interest of the small image
    void drawCase1(SkCanvas* canvas, int transX, int transY, bool aa,
                   const SkSamplingOptions& sampling) {
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        drawImage(canvas, fSmallImage, fSmallSrcRect, dst, sampling, &paint);
    }

    // Draw the area of interest of the large image
    void drawCase2(SkCanvas* canvas, int transX, int transY, bool aa,
                   const SkSamplingOptions& sampling) {
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        drawImage(canvas, fBigImage, fBigSrcRect, dst, sampling, &paint);
    }

    // Draw upper-left 1/4 of the area of interest of the large image
    void drawCase3(SkCanvas* canvas, int transX, int transY, bool aa,
                   const SkSamplingOptions& sampling) {
        SkRect src = SkRect::MakeXYWH(fBigSrcRect.fLeft,
                                      fBigSrcRect.fTop,
                                      fBigSrcRect.width()/2,
                                      fBigSrcRect.height()/2);
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        drawImage(canvas, fBigImage, src, dst, sampling, &paint);
    }

    // Draw the area of interest of the small image with a normal blur
    void drawCase4(SkCanvas* canvas, int transX, int transY, bool aa,
                   const SkSamplingOptions& sampling) {
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                                   SkBlurMask::ConvertRadiusToSigma(3)));
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        drawImage(canvas, fSmallImage, fSmallSrcRect, dst, sampling, &paint);
    }

    // Draw the area of interest of the small image with a outer blur
    void drawCase5(SkCanvas* canvas, int transX, int transY, bool aa,
                   const SkSamplingOptions& sampling) {
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kOuter_SkBlurStyle,
                                                   SkBlurMask::ConvertRadiusToSigma(7)));
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        drawImage(canvas, fSmallImage, fSmallSrcRect, dst, sampling, &paint);
    }

    void onOnceBeforeDraw() override {
        std::tie(fBigImage, fBigSrcRect) = make_ringed_image(2*kMaxTextureSize, 2*kMaxTextureSize);
        std::tie(fSmallImage, fSmallSrcRect) = make_ringed_image(kSmallSize, kSmallSize);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorGRAY);
        std::vector<SkMatrix> matrices;
        // Draw with identity
        matrices.push_back(SkMatrix::I());

        // Draw with rotation and scale down in x, up in y.
        SkMatrix m;
        constexpr SkScalar kBottom = SkIntToScalar(kRow4Y + kBlockSize + kBlockSpacing);
        m.setTranslate(0, kBottom);
        m.preRotate(15.f, 0, kBottom + kBlockSpacing);
        m.preScale(0.71f, 1.22f);
        matrices.push_back(m);

        // Align the next set with the middle of the previous in y, translated to the right in x.
        SkPoint corners[] = {{0, 0}, {0, kBottom}, {kWidth, kBottom}, {kWidth, 0}};
        matrices.back().mapPoints(corners, 4);
        SkScalar y = (corners[0].fY + corners[1].fY + corners[2].fY + corners[3].fY) / 4;
        SkScalar x = std::max({corners[0].fX, corners[1].fX, corners[2].fX, corners[3].fX});
        m.setTranslate(x, y);
        m.preScale(0.2f, 0.2f);
        matrices.push_back(m);

        const SkSamplingOptions none(SkFilterMode::kNearest);
        const SkSamplingOptions  low(SkFilterMode::kLinear);
        const SkSamplingOptions high(SkCubicResampler::Mitchell());

        SkScalar maxX = 0;
        for (bool antiAlias : {false, true}) {
            canvas->save();
            canvas->translate(maxX, 0);
            for (const SkMatrix& matrix : matrices) {
                canvas->save();
                canvas->concat(matrix);

                // First draw a column with no filtering
                this->drawCase1(canvas, kCol0X, kRow0Y, antiAlias, none);
                this->drawCase2(canvas, kCol0X, kRow1Y, antiAlias, none);
                this->drawCase3(canvas, kCol0X, kRow2Y, antiAlias, none);
                this->drawCase4(canvas, kCol0X, kRow3Y, antiAlias, none);
                this->drawCase5(canvas, kCol0X, kRow4Y, antiAlias, none);

                // Then draw a column with low filtering
                this->drawCase1(canvas, kCol1X, kRow0Y, antiAlias, low);
                this->drawCase2(canvas, kCol1X, kRow1Y, antiAlias, low);
                this->drawCase3(canvas, kCol1X, kRow2Y, antiAlias, low);
                this->drawCase4(canvas, kCol1X, kRow3Y, antiAlias, low);
                this->drawCase5(canvas, kCol1X, kRow4Y, antiAlias, low);

                // Then draw a column with high filtering. Skip it if in kStrict mode and MIP
                // mapping will be used. On GPU we allow bleeding at non-base levels because
                // building a new MIP chain for the subset is expensive.
                SkScalar scales[2];
                SkAssertResult(matrix.getMinMaxScales(scales));
                if (fConstraint != SkCanvas::kStrict_SrcRectConstraint || scales[0] >= 1.f) {
                    this->drawCase1(canvas, kCol2X, kRow0Y, antiAlias, high);
                    this->drawCase2(canvas, kCol2X, kRow1Y, antiAlias, high);
                    this->drawCase3(canvas, kCol2X, kRow2Y, antiAlias, high);
                    this->drawCase4(canvas, kCol2X, kRow3Y, antiAlias, high);
                    this->drawCase5(canvas, kCol2X, kRow4Y, antiAlias, high);
                }

                SkPoint innerCorners[] = {{0, 0}, {0, kBottom}, {kWidth, kBottom}, {kWidth, 0}};
                matrix.mapPoints(innerCorners, 4);
                SkScalar x = kBlockSize + std::max({innerCorners[0].fX, innerCorners[1].fX,
                                                    innerCorners[2].fX, innerCorners[3].fX});
                maxX = std::max(maxX, x);
                canvas->restore();
            }
            canvas->restore();
        }
    }

    void modifyGrContextOptions(GrContextOptions* options) override {
        options->fMaxTextureSizeOverride = kMaxTextureSize;
    }

private:
    static constexpr int kBlockSize = 70;
    static constexpr int kBlockSpacing = 12;

    static constexpr int kCol0X = kBlockSpacing;
    static constexpr int kCol1X = 2*kBlockSpacing + kBlockSize;
    static constexpr int kCol2X = 3*kBlockSpacing + 2*kBlockSize;
    static constexpr int kWidth = 4*kBlockSpacing + 3*kBlockSize;

    static constexpr int kRow0Y = kBlockSpacing;
    static constexpr int kRow1Y = 2*kBlockSpacing + kBlockSize;
    static constexpr int kRow2Y = 3*kBlockSpacing + 2*kBlockSize;
    static constexpr int kRow3Y = 4*kBlockSpacing + 3*kBlockSize;
    static constexpr int kRow4Y = 5*kBlockSpacing + 4*kBlockSize;

    static constexpr int kSmallSize = 6;
    // This must be at least as large as the GM width and height so that a surface can be made.
    static constexpr int kMaxTextureSize = 1000;

    SkString fShortName;
    sk_sp<SkImage> fBigImage;
    sk_sp<SkImage> fSmallImage;
    SkRect fBigSrcRect;
    SkRect fSmallSrcRect;
    SkCanvas::SrcRectConstraint fConstraint;
    bool fBatch = false;
    using INHERITED = GM;
};

DEF_GM(return new SrcRectConstraintGM("strict_constraint_no_red_allowed",
                                      SkCanvas::kStrict_SrcRectConstraint,
                                      /*batch=*/false););
DEF_GM(return new SrcRectConstraintGM("strict_constraint_batch_no_red_allowed",
                                      SkCanvas::kStrict_SrcRectConstraint,
                                      /*batch=*/true););
DEF_GM(return new SrcRectConstraintGM("fast_constraint_red_is_allowed",
                                      SkCanvas::kFast_SrcRectConstraint,
                                      /*batch=*/false););

///////////////////////////////////////////////////////////////////////////////////////////////////

// Construct an image and return the inner "src" rect. Build the image such that the interior is
// blue, with a margin of blue (2px) but then an outer margin of red.
//
// Show that kFast_SrcRectConstraint sees even the red margin (due to mipmapping) when the image
// is scaled down far enough.
//
static sk_sp<SkImage> make_image(SkCanvas* canvas, SkRect* srcR) {
    // Intentially making the size a power of 2 to avoid the noise from how different GPUs will
    // produce different mipmap filtering when we have an odd sized texture.
    const int N = 10 + 2 + 8 + 2 + 10;
    SkImageInfo info = SkImageInfo::MakeN32Premul(N, N);
    auto        surface = ToolUtils::makeSurface(canvas, info);
    SkCanvas* c = surface->getCanvas();
    SkRect r = SkRect::MakeIWH(info.width(), info.height());
    SkPaint paint;

    paint.setColor(SK_ColorRED);
    c->drawRect(r, paint);
    r.inset(10, 10);
    paint.setColor(SK_ColorBLUE);
    c->drawRect(r, paint);

    *srcR = r.makeInset(2, 2);
    return surface->makeImageSnapshot();
}

DEF_SIMPLE_GM(bleed_downscale, canvas, 360, 240) {
    SkRect src;
    sk_sp<SkImage> img = make_image(canvas, &src);
    SkPaint paint;

    canvas->translate(10, 10);

    const SkCanvas::SrcRectConstraint constraints[] = {
        SkCanvas::kStrict_SrcRectConstraint, SkCanvas::kFast_SrcRectConstraint
    };
    const SkSamplingOptions samplings[] = {
        SkSamplingOptions(SkFilterMode::kNearest),
        SkSamplingOptions(SkFilterMode::kLinear),
        SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
    };
    for (auto constraint : constraints) {
        canvas->save();
        for (auto sampling : samplings) {
            auto surf = ToolUtils::makeSurface(canvas, SkImageInfo::MakeN32Premul(1, 1));
            surf->getCanvas()->drawImageRect(img, src, SkRect::MakeWH(1, 1), sampling,
                                             nullptr, constraint);
            // now blow up the 1 pixel result
            canvas->drawImageRect(surf->makeImageSnapshot(), SkRect::MakeWH(100, 100),
                                  SkSamplingOptions());
            canvas->translate(120, 0);
        }
        canvas->restore();
        canvas->translate(0, 120);
    }
}


