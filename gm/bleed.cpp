/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBlurMask.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkImage.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrContextOptions.h"
#endif

static void draw_bitmap_rect(SkCanvas* canvas, const SkBitmap& bitmap, const SkImage*,
                             const SkRect& src, const SkRect& dst,
                             const SkPaint* paint, SkCanvas::SrcRectConstraint constraint) {
    canvas->drawBitmapRect(bitmap, src, dst, paint, constraint);
}

static void draw_image_rect(SkCanvas* canvas, const SkBitmap&, const SkImage* image,
                            const SkRect& src, const SkRect& dst,
                            const SkPaint* paint, SkCanvas::SrcRectConstraint constraint) {
    canvas->drawImageRect(image, src, dst, paint, constraint);
}

// Create a black&white checked texture with 2 1-pixel rings
// around the outside edge. The inner ring is red and the outer ring is blue.
static void make_ringed_color_bitmap(SkBitmap* result, int width, int height) {
    SkASSERT(0 == width % 2 && 0 == height % 2);

    static const SkPMColor kRed = SkPreMultiplyColor(SK_ColorRED);
    static const SkPMColor kBlue = SkPreMultiplyColor(SK_ColorBLUE);
    static const SkPMColor kBlack = SkPreMultiplyColor(SK_ColorBLACK);
    static const SkPMColor kWhite = SkPreMultiplyColor(SK_ColorWHITE);

    result->allocN32Pixels(width, height, true);

    SkPMColor* scanline = result->getAddr32(0, 0);
    for (int x = 0; x < width; ++x) {
        scanline[x] = kBlue;
    }
    scanline = result->getAddr32(0, 1);
    scanline[0] = kBlue;
    for (int x = 1; x < width - 1; ++x) {
        scanline[x] = kRed;
    }
    scanline[width-1] = kBlue;

    for (int y = 2; y < height/2; ++y) {
        scanline = result->getAddr32(0, y);
        scanline[0] = kBlue;
        scanline[1] = kRed;
        for (int x = 2; x < width/2; ++x) {
            scanline[x] = kBlack;
        }
        for (int x = width/2; x < width-2; ++x) {
            scanline[x] = kWhite;
        }
        scanline[width-2] = kRed;
        scanline[width-1] = kBlue;
    }

    for (int y = height/2; y < height-2; ++y) {
        scanline = result->getAddr32(0, y);
        scanline[0] = kBlue;
        scanline[1] = kRed;
        for (int x = 2; x < width/2; ++x) {
            scanline[x] = kWhite;
        }
        for (int x = width/2; x < width-2; ++x) {
            scanline[x] = kBlack;
        }
        scanline[width-2] = kRed;
        scanline[width-1] = kBlue;
    }

    scanline = result->getAddr32(0, height-2);
    scanline[0] = kBlue;
    for (int x = 1; x < width - 1; ++x) {
        scanline[x] = kRed;
    }
    scanline[width-1] = kBlue;

    scanline = result->getAddr32(0, height-1);
    for (int x = 0; x < width; ++x) {
        scanline[x] = kBlue;
    }
    result->setImmutable();
}

/** Makes a alpha bitmap with 1 wide rect/ring of 1s, an inset of 0s, and the interior is a 2x2
    checker board of 3/4 and 1/2. The inner checkers are large enough to fill the interior with
    the 2x2 checker grid. */
static void make_ringed_alpha_bitmap(SkBitmap* result, int width, int height) {
    SkASSERT(0 == width % 2 && 0 == height % 2);

    static const SkPMColor kZero = 0x00;
    static const SkPMColor kHalf = 0x80;
    static const SkPMColor k3Q   = 0xC0;
    static const SkPMColor kOne  = 0xFF;
    SkImageInfo info = SkImageInfo::MakeA8(width, height);
    // The 4 byte alignment seems to be necessary to allow this bmp to be converted
    // to an image.
    result->allocPixels(info, SkAlign4(width));

    uint8_t* scanline = result->getAddr8(0, 0);
    for (int x = 0; x < width; ++x) {
        scanline[x] = kOne;
    }
    scanline = result->getAddr8(0, 1);
    scanline[0] = kOne;
    for (int x = 1; x < width - 1; ++x) {
        scanline[x] = kZero;
    }
    scanline[width - 1] = kOne;

    for (int y = 2; y < height / 2; ++y) {
        scanline = result->getAddr8(0, y);
        scanline[0] = kOne;
        scanline[1] = kZero;
        for (int x = 2; x < width / 2; ++x) {
            scanline[x] = k3Q;
        }
        for (int x = width / 2; x < width - 2; ++x) {
            scanline[x] = kHalf;
        }
        scanline[width - 2] = kZero;
        scanline[width - 1] = kOne;
    }

    for (int y = height / 2; y < height - 2; ++y) {
        scanline = result->getAddr8(0, y);
        scanline[0] = kOne;
        scanline[1] = kZero;
        for (int x = 2; x < width / 2; ++x) {
            scanline[x] = kHalf;
        }
        for (int x = width / 2; x < width - 2; ++x) {
            scanline[x] = k3Q;
        }
        scanline[width - 2] = kZero;
        scanline[width - 1] = kOne;
    }

    scanline = result->getAddr8(0, height - 2);
    scanline[0] = kOne;
    for (int x = 1; x < width - 1; ++x) {
        scanline[x] = kZero;
    }
    scanline[width - 1] = kOne;

    scanline = result->getAddr8(0, height - 1);
    for (int x = 0; x < width; ++x) {
        scanline[x] = kOne;
    }
    result->setImmutable();
}

static SkShader* make_shader() {
    static const SkPoint pts[] = { {0, 0}, {20, 20} };
    static const SkColor colors[] = { SK_ColorGREEN, SK_ColorYELLOW };
    return SkGradientShader::CreateLinear(pts, colors, nullptr, 2, SkShader::kMirror_TileMode);
}

static SkShader* make_null_shader() { return nullptr; }

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
    void(*fBmpMaker)(SkBitmap* result, int width, int height);
    SkShader*(*fShaderMaker)();
    void(*fDraw)(SkCanvas*, const SkBitmap&, const SkImage*, const SkRect&, const SkRect&,
                 const SkPaint*, SkCanvas::SrcRectConstraint);
} gBleedRec[] = {
    { "bleed",                     make_ringed_color_bitmap, make_null_shader,  draw_bitmap_rect },
    { "bleed_image",               make_ringed_color_bitmap, make_null_shader,  draw_image_rect  },
    { "bleed_alpha_bmp",           make_ringed_alpha_bitmap, make_null_shader,  draw_bitmap_rect },
    { "bleed_alpha_image",         make_ringed_alpha_bitmap, make_null_shader,  draw_image_rect  },
    { "bleed_alpha_bmp_shader",    make_ringed_alpha_bitmap, make_shader,       draw_bitmap_rect },
    { "bleed_alpha_image_shader",  make_ringed_alpha_bitmap, make_shader,       draw_image_rect  },
};

// This GM exercises the drawBitmapRect constraints
class BleedGM : public skiagm::GM {
public:
    BleedGM(BleedTest bt) : fBT(bt) {}

protected:

    SkString onShortName() override {
        return SkString(gBleedRec[fBT].fName);
    }

    SkISize onISize() override {
        return SkISize::Make(1050, 780);
    }

    void onOnceBeforeDraw() override {
        gBleedRec[fBT].fBmpMaker(&fBitmapSmall, kSmallTextureSize, kSmallTextureSize);
        fImageSmall.reset(SkImage::NewFromBitmap(fBitmapSmall));

        // To exercise the GPU's tiling path we need a texture
        // too big for the GPU to handle in one go
        gBleedRec[fBT].fBmpMaker(&fBitmapBig, 2*kMaxTextureSize, 2*kMaxTextureSize);
        fImageBig.reset(SkImage::NewFromBitmap(fBitmapBig));

        fShader.reset(gBleedRec[fBT].fShaderMaker());
    }

    // Draw only the center of the small bitmap
    void drawCase1(SkCanvas* canvas, int transX, int transY, bool aa,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {
        SkRect src = SkRect::MakeXYWH(2, 2,
                                      SkIntToScalar(kSmallTextureSize-4),
                                      SkIntToScalar(kSmallTextureSize-4));
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);
        paint.setShader(fShader);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        gBleedRec[fBT].fDraw(canvas, fBitmapSmall, fImageSmall, src, dst, &paint, constraint);
    }

    // Draw almost all of the large bitmap
    void drawCase2(SkCanvas* canvas, int transX, int transY, bool aa,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {
        SkRect src = SkRect::MakeXYWH(2, 2,
                                      SkIntToScalar(fBitmapBig.width()-4),
                                      SkIntToScalar(fBitmapBig.height()-4));
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);
        paint.setShader(fShader);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        gBleedRec[fBT].fDraw(canvas, fBitmapBig, fImageBig, src, dst, &paint, constraint);
    }

    // Draw ~1/4 of the large bitmap
    void drawCase3(SkCanvas* canvas, int transX, int transY, bool aa,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {
        SkRect src = SkRect::MakeXYWH(2, 2,
                                      SkIntToScalar(fBitmapBig.width()/2-2),
                                      SkIntToScalar(fBitmapBig.height()/2-2));
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);
        paint.setShader(fShader);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        gBleedRec[fBT].fDraw(canvas, fBitmapBig, fImageBig, src, dst, &paint, constraint);
    }

    // Draw the center of the small bitmap with a mask filter
    void drawCase4(SkCanvas* canvas, int transX, int transY, bool aa,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {
        SkRect src = SkRect::MakeXYWH(2, 2,
                                      SkIntToScalar(kSmallTextureSize-4),
                                      SkIntToScalar(kSmallTextureSize-4));
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);
        SkMaskFilter* mf = SkBlurMaskFilter::Create(kNormal_SkBlurStyle,
                                         SkBlurMask::ConvertRadiusToSigma(3));
        paint.setMaskFilter(mf)->unref();
        paint.setShader(fShader);
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(aa);

        gBleedRec[fBT].fDraw(canvas, fBitmapSmall, fImageSmall, src, dst, &paint, constraint);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->clear(SK_ColorGRAY);
        SkTDArray<SkMatrix> matrices;
        // Draw with identity
        *matrices.append() = SkMatrix::I();

        // Draw with rotation and scale down in x, up in y.
        SkMatrix m;
        static const SkScalar kBottom = SkIntToScalar(kRow3Y + kBlockSize + kBlockSpacing);
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

                // Then draw a column with no bleeding and low filtering
                this->drawCase1(canvas, kCol1X, kRow0Y, aa, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase2(canvas, kCol1X, kRow1Y, aa, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase3(canvas, kCol1X, kRow2Y, aa, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase4(canvas, kCol1X, kRow3Y, aa, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);

                // Then draw a column with no bleeding and high filtering
                this->drawCase1(canvas, kCol2X, kRow0Y, aa, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase2(canvas, kCol2X, kRow1Y, aa, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase3(canvas, kCol2X, kRow2Y, aa, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase4(canvas, kCol2X, kRow3Y, aa, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);

                // Then draw a column with bleeding and no filtering (bleed should have no effect w/out blur)
                this->drawCase1(canvas, kCol3X, kRow0Y, aa, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);
                this->drawCase2(canvas, kCol3X, kRow1Y, aa, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);
                this->drawCase3(canvas, kCol3X, kRow2Y, aa, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);
                this->drawCase4(canvas, kCol3X, kRow3Y, aa, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);

                // Then draw a column with bleeding and low filtering
                this->drawCase1(canvas, kCol4X, kRow0Y, aa, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase2(canvas, kCol4X, kRow1Y, aa, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase3(canvas, kCol4X, kRow2Y, aa, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);
                this->drawCase4(canvas, kCol4X, kRow3Y, aa, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);

                // Finally draw a column with bleeding and high filtering
                this->drawCase1(canvas, kCol5X, kRow0Y, aa, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase2(canvas, kCol5X, kRow1Y, aa, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase3(canvas, kCol5X, kRow2Y, aa, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);
                this->drawCase4(canvas, kCol5X, kRow3Y, aa, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);

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

#if SK_SUPPORT_GPU
    void modifyGrContextOptions(GrContextOptions* options) override {
        options->fMaxTextureSizeOverride = kMaxTextureSize;
    }
#endif

private:
    static const int kBlockSize = 70;
    static const int kBlockSpacing = 5;

    static const int kCol0X = kBlockSpacing;
    static const int kCol1X = 2*kBlockSpacing + kBlockSize;
    static const int kCol2X = 3*kBlockSpacing + 2*kBlockSize;
    static const int kCol3X = 4*kBlockSpacing + 3*kBlockSize;
    static const int kCol4X = 5*kBlockSpacing + 4*kBlockSize;
    static const int kCol5X = 6*kBlockSpacing + 5*kBlockSize;
    static const int kWidth = 7*kBlockSpacing + 6*kBlockSize;

    static const int kRow0Y = kBlockSpacing;
    static const int kRow1Y = 2*kBlockSpacing + kBlockSize;
    static const int kRow2Y = 3*kBlockSpacing + 2*kBlockSize;
    static const int kRow3Y = 4*kBlockSpacing + 3*kBlockSize;

    static const int kSmallTextureSize = 6;
    static const int kMaxTextureSize = 32;

    SkBitmap fBitmapSmall;
    SkBitmap fBitmapBig;
    SkAutoTUnref<SkImage> fImageSmall;
    SkAutoTUnref<SkImage> fImageBig;

    SkAutoTUnref<SkShader> fShader;

    const BleedTest fBT;

    typedef GM INHERITED;
};

DEF_GM( return new BleedGM(kUseBitmap_BleedTest); )
DEF_GM( return new BleedGM(kUseImage_BleedTest); )
DEF_GM( return new BleedGM(kUseAlphaBitmap_BleedTest); )
DEF_GM( return new BleedGM(kUseAlphaImage_BleedTest); )
#if 0  // Currently crashes GPU backend
DEF_GM(return new BleedGM(kUseAlphaBitmapShader_BleedTest); )
DEF_GM(return new BleedGM(kUseAlphaImageShader_BleedTest); )
#endif
