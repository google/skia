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

enum BleedTest {
    kUseBitmap_BleedTest,
    kUseImage_BleedTest,
};

const struct {
    const char* fName;
    void (*fDraw)(SkCanvas*, const SkBitmap&, const SkImage*, const SkRect&, const SkRect&,
                  const SkPaint*, SkCanvas::SrcRectConstraint);
} gBleedRec[] = {
    { "bleed",          draw_bitmap_rect },
    { "bleed_image",    draw_image_rect },
};

// Create a black&white checked texture with 2 1-pixel rings
// around the outside edge. The inner ring is red and the outer ring is blue.
static void make_ringed_bitmap(SkBitmap* result, int width, int height) {
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

// This GM exercises the drawBitmapRect constraints
class BleedGM : public skiagm::GM {
public:
    BleedGM(BleedTest bt) : fBT(bt) {}

protected:

    SkString onShortName() override {
        return SkString(gBleedRec[fBT].fName);
    }

    SkISize onISize() override {
        return SkISize::Make(kWidth, 780);
    }

    void onOnceBeforeDraw() override {
        make_ringed_bitmap(&fBitmapSmall, kSmallTextureSize, kSmallTextureSize);
        fImageSmall.reset(SkImage::NewFromBitmap(fBitmapSmall));

        // To exercise the GPU's tiling path we need a texture
        // too big for the GPU to handle in one go
        make_ringed_bitmap(&fBitmapBig, 2*kMaxTextureSize, 2*kMaxTextureSize);
        fImageBig.reset(SkImage::NewFromBitmap(fBitmapBig));
    }

    // Draw only the center of the small bitmap
    void drawCase1(SkCanvas* canvas, int transX, int transY,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {
        SkRect src = SkRect::MakeXYWH(2, 2,
                                      SkIntToScalar(kSmallTextureSize-4),
                                      SkIntToScalar(kSmallTextureSize-4));
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);

        gBleedRec[fBT].fDraw(canvas, fBitmapSmall, fImageSmall, src, dst, &paint, constraint);
    }

    // Draw almost all of the large bitmap
    void drawCase2(SkCanvas* canvas, int transX, int transY,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {
        SkRect src = SkRect::MakeXYWH(2, 2,
                                      SkIntToScalar(fBitmapBig.width()-4),
                                      SkIntToScalar(fBitmapBig.height()-4));
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);

        gBleedRec[fBT].fDraw(canvas, fBitmapBig, fImageBig, src, dst, &paint, constraint);
    }

    // Draw ~1/4 of the large bitmap
    void drawCase3(SkCanvas* canvas, int transX, int transY,
                   SkCanvas::SrcRectConstraint constraint, SkFilterQuality filter) {
        SkRect src = SkRect::MakeXYWH(2, 2,
                                      SkIntToScalar(fBitmapBig.width()/2-2),
                                      SkIntToScalar(fBitmapBig.height()/2-2));
        SkRect dst = SkRect::MakeXYWH(SkIntToScalar(transX), SkIntToScalar(transY),
                                      SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterQuality(filter);

        gBleedRec[fBT].fDraw(canvas, fBitmapBig, fImageBig, src, dst, &paint, constraint);
    }

    // Draw the center of the small bitmap with a mask filter
    void drawCase4(SkCanvas* canvas, int transX, int transY,
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

        gBleedRec[fBT].fDraw(canvas, fBitmapSmall, fImageSmall, src, dst, &paint, constraint);
    }

    void onDraw(SkCanvas* canvas) override {

        canvas->clear(SK_ColorGRAY);

        for (int m = 0; m < 2; ++m) {
            canvas->save();
            if (m) {
                static const SkScalar kBottom = SkIntToScalar(kRow3Y + kBlockSize + kBlockSpacing);
                canvas->translate(0, kBottom);
                SkMatrix rotate;
                rotate.setRotate(15.f, 0, kBottom + kBlockSpacing);
                canvas->concat(rotate);
                canvas->scale(0.71f, 1.22f);
            }

            // First draw a column with no bleeding and no filtering
            this->drawCase1(canvas, kCol0X, kRow0Y, SkCanvas::kStrict_SrcRectConstraint, kNone_SkFilterQuality);
            this->drawCase2(canvas, kCol0X, kRow1Y, SkCanvas::kStrict_SrcRectConstraint, kNone_SkFilterQuality);
            this->drawCase3(canvas, kCol0X, kRow2Y, SkCanvas::kStrict_SrcRectConstraint, kNone_SkFilterQuality);
            this->drawCase4(canvas, kCol0X, kRow3Y, SkCanvas::kStrict_SrcRectConstraint, kNone_SkFilterQuality);

            // Then draw a column with no bleeding and low filtering
            this->drawCase1(canvas, kCol1X, kRow0Y, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);
            this->drawCase2(canvas, kCol1X, kRow1Y, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);
            this->drawCase3(canvas, kCol1X, kRow2Y, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);
            this->drawCase4(canvas, kCol1X, kRow3Y, SkCanvas::kStrict_SrcRectConstraint, kLow_SkFilterQuality);

            // Then draw a column with no bleeding and high filtering
            this->drawCase1(canvas, kCol2X, kRow0Y, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);
            this->drawCase2(canvas, kCol2X, kRow1Y, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);
            this->drawCase3(canvas, kCol2X, kRow2Y, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);
            this->drawCase4(canvas, kCol2X, kRow3Y, SkCanvas::kStrict_SrcRectConstraint, kHigh_SkFilterQuality);

            // Then draw a column with bleeding and no filtering (bleed should have no effect w/out blur)
            this->drawCase1(canvas, kCol3X, kRow0Y, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);
            this->drawCase2(canvas, kCol3X, kRow1Y, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);
            this->drawCase3(canvas, kCol3X, kRow2Y, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);
            this->drawCase4(canvas, kCol3X, kRow3Y, SkCanvas::kFast_SrcRectConstraint, kNone_SkFilterQuality);

            // Then draw a column with bleeding and low filtering
            this->drawCase1(canvas, kCol4X, kRow0Y, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);
            this->drawCase2(canvas, kCol4X, kRow1Y, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);
            this->drawCase3(canvas, kCol4X, kRow2Y, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);
            this->drawCase4(canvas, kCol4X, kRow3Y, SkCanvas::kFast_SrcRectConstraint, kLow_SkFilterQuality);

            // Finally draw a column with bleeding and high filtering
            this->drawCase1(canvas, kCol5X, kRow0Y, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);
            this->drawCase2(canvas, kCol5X, kRow1Y, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);
            this->drawCase3(canvas, kCol5X, kRow2Y, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);
            this->drawCase4(canvas, kCol5X, kRow3Y, SkCanvas::kFast_SrcRectConstraint, kHigh_SkFilterQuality);

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
    const BleedTest fBT;

    typedef GM INHERITED;
};

DEF_GM( return new BleedGM(kUseBitmap_BleedTest); )
DEF_GM( return new BleedGM(kUseImage_BleedTest); )
