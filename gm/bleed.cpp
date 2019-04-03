/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMask.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkImage.h"
#include "SkMaskFilter.h"
#include "SkTDArray.h"
#include "SkUTF.h"
#include "ToolUtils.h"
#include "gm.h"

#include "GrContext.h"
#include "GrContextOptions.h"
#include "SkGr.h"

/** Holds either a bitmap or image to be rendered and a rect that indicates what part of the bitmap
    or image should be tested by the GM. The area outside of the rect is present to check
    for bleed due to filtering/blurring. */
struct TestPixels {
    enum Type {
        kBitmap,
        kImage
    };
    Type            fType;
    SkBitmap        fBitmap;
    sk_sp<SkImage>  fImage;
    SkIRect         fRect;  // The region of the bitmap/image that should be rendered.
};

/** Creates a bitmap with two one-pixel rings around a checkerboard. The checkerboard is 2x2
    logically where each check has as many pixels as is necessary to fill the interior. The rect
    to draw is set to the checkerboard portion. */
template<typename PIXEL_TYPE>
bool make_ringed_bitmap(TestPixels* result, int width, int height,
                        SkColorType ct, SkAlphaType at,
                        PIXEL_TYPE outerRingColor, PIXEL_TYPE innerRingColor,
                        PIXEL_TYPE checkColor1, PIXEL_TYPE checkColor2) {
    SkASSERT(0 == width % 2 && 0 == height % 2);
    SkASSERT(width >= 6 && height >= 6);

    result->fType = TestPixels::kBitmap;
    SkImageInfo info = SkImageInfo::Make(width, height, ct, at);
    size_t rowBytes = SkAlign4(info.minRowBytes());
    result->fBitmap.allocPixels(info, rowBytes);

    PIXEL_TYPE* scanline = (PIXEL_TYPE*)result->fBitmap.getAddr(0, 0);
    for (int x = 0; x < width; ++x) {
        scanline[x] = outerRingColor;
    }
    scanline = (PIXEL_TYPE*)result->fBitmap.getAddr(0, 1);
    scanline[0] = outerRingColor;
    for (int x = 1; x < width - 1; ++x) {
        scanline[x] = innerRingColor;
    }
    scanline[width - 1] = outerRingColor;

    for (int y = 2; y < height / 2; ++y) {
        scanline = (PIXEL_TYPE*)result->fBitmap.getAddr(0, y);
        scanline[0] = outerRingColor;
        scanline[1] = innerRingColor;
        for (int x = 2; x < width / 2; ++x) {
            scanline[x] = checkColor1;
        }
        for (int x = width / 2; x < width - 2; ++x) {
            scanline[x] = checkColor2;
        }
        scanline[width - 2] = innerRingColor;
        scanline[width - 1] = outerRingColor;
    }

    for (int y = height / 2; y < height - 2; ++y) {
        scanline = (PIXEL_TYPE*)result->fBitmap.getAddr(0, y);
        scanline[0] = outerRingColor;
        scanline[1] = innerRingColor;
        for (int x = 2; x < width / 2; ++x) {
            scanline[x] = checkColor2;
        }
        for (int x = width / 2; x < width - 2; ++x) {
            scanline[x] = checkColor1;
        }
        scanline[width - 2] = innerRingColor;
        scanline[width - 1] = outerRingColor;
    }

    scanline = (PIXEL_TYPE*)result->fBitmap.getAddr(0, height - 2);
    scanline[0] = outerRingColor;
    for (int x = 1; x < width - 1; ++x) {
        scanline[x] = innerRingColor;
    }
    scanline[width - 1] = outerRingColor;

    scanline = (PIXEL_TYPE*)result->fBitmap.getAddr(0, height - 1);
    for (int x = 0; x < width; ++x) {
        scanline[x] = outerRingColor;
    }
    result->fBitmap.setImmutable();
    result->fRect.set(2, 2, width - 2, height - 2);
    return true;
}

/** Create a black and white checked bitmap with 2 1-pixel rings around the outside edge.
    The inner ring is red and the outer ring is blue. */
static bool make_ringed_color_bitmap(TestPixels* result, int width, int height) {
    const SkPMColor kBlue  = SkPreMultiplyColor(SK_ColorBLUE);
    const SkPMColor kRed   = SkPreMultiplyColor(SK_ColorRED);
    const SkPMColor kBlack = SkPreMultiplyColor(SK_ColorBLACK);
    const SkPMColor kWhite = SkPreMultiplyColor(SK_ColorWHITE);
    return make_ringed_bitmap<SkPMColor>(result, width, height, kN32_SkColorType,
                                         kPremul_SkAlphaType, kBlue, kRed, kBlack, kWhite);
}

/** Makes a alpha bitmap with 1 wide rect/ring of 0s, an inset of 1s, and the interior is a 2x2
    checker board of 3/4 and 1/2. The inner checkers are large enough to fill the interior with
    the 2x2 checker grid. */
static bool make_ringed_alpha_bitmap(TestPixels* result, int width, int height) {
    constexpr uint8_t kZero = 0x00;
    constexpr uint8_t kHalf = 0x80;
    constexpr uint8_t k3Q   = 0xC0;
    constexpr uint8_t kOne  = 0xFF;
    return make_ringed_bitmap<uint8_t>(result, width, height, kAlpha_8_SkColorType,
                                       kPremul_SkAlphaType, kZero, kOne, k3Q, kHalf);
}

/** Helper to reuse above functions to produce images rather than bmps */
static void bmp_to_image(TestPixels* result) {
    SkASSERT(TestPixels::kBitmap == result->fType);
    result->fImage = SkImage::MakeFromBitmap(result->fBitmap);
    SkASSERT(result->fImage);
    result->fType = TestPixels::kImage;
    result->fBitmap.reset();
}

/** Color image case. */
bool make_ringed_color_image(TestPixels* result, int width, int height) {
    if (make_ringed_color_bitmap(result, width, height)) {
        bmp_to_image(result);
        return true;
    }
    return false;
}

/** Alpha image case. */
bool make_ringed_alpha_image(TestPixels* result, int width, int height) {
    if (make_ringed_alpha_bitmap(result, width, height)) {
        bmp_to_image(result);
        return true;
    }
    return false;
}

static sk_sp<SkShader> make_shader() {
    constexpr SkPoint pts[] = { {0, 0}, {20, 20} };
    constexpr SkColor colors[] = { SK_ColorGREEN, SK_ColorYELLOW };
    return SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kMirror);
}

static sk_sp<SkShader> make_null_shader() { return nullptr; }

enum BleedTest {
    kUseBitmap_BleedTest,
    kUseImage_BleedTest,
    kUseAlphaBitmap_BleedTest,
    kUseAlphaImage_BleedTest,
    kUseAlphaBitmapShader_BleedTest,
    kUseAlphaImageShader_BleedTest,
};

const struct {
    const char* fName;
    bool (*fPixelMaker)(TestPixels* result, int width, int height);
    sk_sp<SkShader> (*fShaderMaker)();
} gBleedRec[] = {
    { "bleed",                          make_ringed_color_bitmap,                   make_null_shader },
    { "bleed_image",                    make_ringed_color_image,                    make_null_shader },
    { "bleed_alpha_bmp",                make_ringed_alpha_bitmap,                   make_null_shader },
    { "bleed_alpha_image",              make_ringed_alpha_image,                    make_null_shader },
    { "bleed_alpha_bmp_shader",         make_ringed_alpha_bitmap,                   make_shader      },
    { "bleed_alpha_image_shader",       make_ringed_alpha_image,                    make_shader      },
};

/** This GM exercises the behavior of the drawBitmapRect & drawImageRect calls. Specifically their
    handling of :
     - SrcRectConstraint(bleed vs.no - bleed)
     - handling of the sub - region feature(area - of - interest) of drawBitmap*
     - handling of 8888 vs. A8 (including presence of a shader in the A8 case).
    In particular, we should never see the padding outside of an SkBitmap's sub - region (green for
    8888, 1/4 for alpha). In some instances we can see the two outer rings outside of the area o
    interest (i.e., the inner four checks) due to AA or filtering if allowed by the
    SrcRectConstraint.
*/
class BleedGM : public skiagm::GM {
public:
    BleedGM(BleedTest bt) : fBT(bt){}

protected:

    SkString onShortName() override {
        return SkString(gBleedRec[fBT].fName);
    }

    SkISize onISize() override {
        return SkISize::Make(1200, 1080);
    }

    void drawPixels(SkCanvas* canvas, const TestPixels& pixels, const SkRect& src,
                    const SkRect& dst, const SkPaint* paint,
                    SkCanvas::SrcRectConstraint constraint) {
        if (TestPixels::kBitmap == pixels.fType) {
            canvas->drawBitmapRect(pixels.fBitmap, src, dst, paint, constraint);
        } else {
            canvas->drawImageRect(pixels.fImage.get(), src, dst, paint, constraint);
        }
    }

    // Draw the area of interest of the small image
    void drawCase1(SkCanvas* canvas, int transX, int transY, bool aa,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {

        SkRect src = SkRect::Make(fSmallTestPixels.fRect);
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);
        paint.setShader(fShader);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        this->drawPixels(canvas, fSmallTestPixels, src, dst, &paint, constraint);
    }

    // Draw the area of interest of the large image
    void drawCase2(SkCanvas* canvas, int transX, int transY, bool aa,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {
        SkRect src = SkRect::Make(fBigTestPixels.fRect);
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);
        paint.setShader(fShader);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        this->drawPixels(canvas, fBigTestPixels, src, dst, &paint, constraint);
    }

    // Draw upper-left 1/4 of the area of interest of the large image
    void drawCase3(SkCanvas* canvas, int transX, int transY, bool aa,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {
        SkRect src = SkRect::MakeXYWH(SkIntToScalar(fBigTestPixels.fRect.fLeft),
                                      SkIntToScalar(fBigTestPixels.fRect.fTop),
                                      fBigTestPixels.fRect.width()/2.f,
                                      fBigTestPixels.fRect.height()/2.f);
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);
        paint.setShader(fShader);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        this->drawPixels(canvas, fBigTestPixels, src, dst, &paint, constraint);
    }

    // Draw the area of interest of the small image with a normal blur
    void drawCase4(SkCanvas* canvas, int transX, int transY, bool aa,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {
        SkRect src = SkRect::Make(fSmallTestPixels.fRect);
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                                   SkBlurMask::ConvertRadiusToSigma(3)));
        paint.setShader(fShader);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        this->drawPixels(canvas, fSmallTestPixels, src, dst, &paint, constraint);
    }

    // Draw the area of interest of the small image with a outer blur
    void drawCase5(SkCanvas* canvas, int transX, int transY, bool aa,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {
        SkRect src = SkRect::Make(fSmallTestPixels.fRect);
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);
        paint.setMaskFilter(SkMaskFilter::MakeBlur(kOuter_SkBlurStyle,
                                                   SkBlurMask::ConvertRadiusToSigma(7)));
        paint.setShader(fShader);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        this->drawPixels(canvas, fSmallTestPixels, src, dst, &paint, constraint);
    }

    void onOnceBeforeDraw() override {
        SkAssertResult(gBleedRec[fBT].fPixelMaker(&fSmallTestPixels, kSmallSize, kSmallSize));
        SkAssertResult(gBleedRec[fBT].fPixelMaker(&fBigTestPixels, 2 * kMaxTileSize,
                                                 2 * kMaxTileSize));
    }

    void onDraw(SkCanvas* canvas) override {
        fShader = gBleedRec[fBT].fShaderMaker();

        canvas->clear(SK_ColorGRAY);
        SkTDArray<SkMatrix> matrices;
        // Draw with identity
        *matrices.append() = SkMatrix::I();

        // Draw with rotation and scale down in x, up in y.
        SkMatrix m;
        constexpr SkScalar kBottom = SkIntToScalar(kRow4Y + kBlockSize + kBlockSpacing);
        m.setTranslate(0, kBottom);
        m.preRotate(15.f, 0, kBottom + kBlockSpacing);
        m.preScale(0.71f, 1.22f);
        *matrices.append() = m;

        // Align the next set with the middle of the previous in y, translated to the right in x.
        SkPoint corners[] = {{0, 0}, { 0, kBottom }, { kWidth, kBottom }, {kWidth, 0} };
        matrices[matrices.count()-1].mapPoints(corners, 4);
        SkScalar y = (corners[0].fY + corners[1].fY + corners[2].fY + corners[3].fY) / 4;
        SkScalar x = SkTMax(SkTMax(corners[0].fX, corners[1].fX),
                            SkTMax(corners[2].fX, corners[3].fX));
        m.setTranslate(x, y);
        m.preScale(0.2f, 0.2f);
        *matrices.append() = m;

        SkScalar maxX = 0;
        for (int antiAlias = 0; antiAlias < 2; ++antiAlias) {
            canvas->save();
            canvas->translate(maxX, 0);
            for (int m = 0; m < matrices.count(); ++m) {
                canvas->save();
                canvas->concat(matrices[m]);
                bool aa = SkToBool(antiAlias);

                // First draw a column with no bleeding and no filtering
                this->drawCase1(canvas, kCol0X, kRow0Y, aa, SkCanvas::kStrict_SrcRectConstraint, kNone_SkFilterQuality);
                this->drawCase2(canvas, kCol0X, kRow1Y, aa, SkCanvas::kStrict_SrcRectConstraint, kNone_SkFilterQuality);
                this->drawCase3(canvas, kCol0X, kRow2Y, aa, SkCanvas::kStrict_SrcRectConstraint, kNone_SkFilterQuality);
                this->drawCase4(canvas, kCol0X, kRow3Y, aa, SkCanvas::kStrict_SrcRectConstraint, kNone_SkFilterQuality);
                this->drawCase5(canvas, kCol0X, kRow4Y, aa, SkCanvas::kStrict_SrcRectConstraint, kNone_SkFilterQuality);

                // Then draw a column with no bleeding and low filtering
                this->drawCase1(canvas, kCol1X, kRow0Y, aa, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase2(canvas, kCol1X, kRow1Y, aa, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase3(canvas, kCol1X, kRow2Y, aa, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase4(canvas, kCol1X, kRow3Y, aa, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase5(canvas, kCol1X, kRow4Y, aa, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);

                // Then draw a column with no bleeding and high filtering
                this->drawCase1(canvas, kCol2X, kRow0Y, aa, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase2(canvas, kCol2X, kRow1Y, aa, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase3(canvas, kCol2X, kRow2Y, aa, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase4(canvas, kCol2X, kRow3Y, aa, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase5(canvas, kCol2X, kRow4Y, aa, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);

                // Then draw a column with bleeding and no filtering (bleed should have no effect w/out blur)
                this->drawCase1(canvas, kCol3X, kRow0Y, aa, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);
                this->drawCase2(canvas, kCol3X, kRow1Y, aa, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);
                this->drawCase3(canvas, kCol3X, kRow2Y, aa, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);
                this->drawCase4(canvas, kCol3X, kRow3Y, aa, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);
                this->drawCase5(canvas, kCol3X, kRow4Y, aa, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);

                // Then draw a column with bleeding and low filtering
                this->drawCase1(canvas, kCol4X, kRow0Y, aa, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase2(canvas, kCol4X, kRow1Y, aa, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase3(canvas, kCol4X, kRow2Y, aa, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase4(canvas, kCol4X, kRow3Y, aa, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase5(canvas, kCol4X, kRow4Y, aa, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);

                // Finally draw a column with bleeding and high filtering
                this->drawCase1(canvas, kCol5X, kRow0Y, aa, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase2(canvas, kCol5X, kRow1Y, aa, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase3(canvas, kCol5X, kRow2Y, aa, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase4(canvas, kCol5X, kRow3Y, aa, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase5(canvas, kCol5X, kRow4Y, aa, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);

                SkPoint corners[] = { { 0, 0 },{ 0, kBottom },{ kWidth, kBottom },{ kWidth, 0 } };
                matrices[m].mapPoints(corners, 4);
                SkScalar x = kBlockSize + SkTMax(SkTMax(corners[0].fX, corners[1].fX),
                                                 SkTMax(corners[2].fX, corners[3].fX));
                maxX = SkTMax(maxX, x);
                canvas->restore();
            }
            canvas->restore();
        }
    }

    void modifyGrContextOptions(GrContextOptions* options) override {
        options->fMaxTileSizeOverride = kMaxTileSize;
    }

private:
    static constexpr int kBlockSize = 70;
    static constexpr int kBlockSpacing = 12;

    static constexpr int kCol0X = kBlockSpacing;
    static constexpr int kCol1X = 2*kBlockSpacing + kBlockSize;
    static constexpr int kCol2X = 3*kBlockSpacing + 2*kBlockSize;
    static constexpr int kCol3X = 4*kBlockSpacing + 3*kBlockSize;
    static constexpr int kCol4X = 5*kBlockSpacing + 4*kBlockSize;
    static constexpr int kCol5X = 6*kBlockSpacing + 5*kBlockSize;
    static constexpr int kWidth = 7*kBlockSpacing + 6*kBlockSize;

    static constexpr int kRow0Y = kBlockSpacing;
    static constexpr int kRow1Y = 2*kBlockSpacing + kBlockSize;
    static constexpr int kRow2Y = 3*kBlockSpacing + 2*kBlockSize;
    static constexpr int kRow3Y = 4*kBlockSpacing + 3*kBlockSize;
    static constexpr int kRow4Y = 5*kBlockSpacing + 4*kBlockSize;

    static constexpr int kSmallSize = 6;
    static constexpr int kMaxTileSize = 32;

    TestPixels      fBigTestPixels;
    TestPixels      fSmallTestPixels;

    sk_sp<SkShader> fShader;

    const BleedTest fBT;

    typedef GM INHERITED;
};


DEF_GM( return new BleedGM(kUseBitmap_BleedTest); )
DEF_GM( return new BleedGM(kUseImage_BleedTest); )
DEF_GM( return new BleedGM(kUseAlphaBitmap_BleedTest); )
DEF_GM( return new BleedGM(kUseAlphaImage_BleedTest); )
DEF_GM( return new BleedGM(kUseAlphaBitmapShader_BleedTest); )
DEF_GM( return new BleedGM(kUseAlphaImageShader_BleedTest); )

///////////////////////////////////////////////////////////////////////////////////////////////////
#include "SkSurface.h"

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
    const SkFilterQuality qualities[] = {
        kNone_SkFilterQuality, kLow_SkFilterQuality, kMedium_SkFilterQuality
    };
    for (auto constraint : constraints) {
        canvas->save();
        for (auto quality : qualities) {
            paint.setFilterQuality(quality);
            auto surf = ToolUtils::makeSurface(canvas, SkImageInfo::MakeN32Premul(1, 1));
            surf->getCanvas()->drawImageRect(img, src, SkRect::MakeWH(1, 1), &paint, constraint);
            // now blow up the 1 pixel result
            canvas->drawImageRect(surf->makeImageSnapshot(), SkRect::MakeWH(100, 100), nullptr);
            canvas->translate(120, 0);
        }
        canvas->restore();
        canvas->translate(0, 120);
    }
}


