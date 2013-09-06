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

#if SK_SUPPORT_GPU
#include "GrContext.h"

namespace skiagm {
extern GrContext* GetGr();
};
#endif

// Create a black&white checked texture with a 1-pixel red ring
// around the outside edge
static void make_red_ringed_bitmap(SkBitmap* result, int width, int height) {
    SkASSERT(0 == width % 2 && 0 == width % 2);

    result->setConfig(SkBitmap::kARGB_8888_Config, width, height);
    result->allocPixels();
    SkAutoLockPixels lock(*result);

    SkPMColor* scanline = result->getAddr32(0, 0);
    for (int x = 0; x < width; ++x) {
        scanline[x] = SK_ColorRED;
    }

    for (int y = 1; y < height/2; ++y) {
        scanline = result->getAddr32(0, y);
        scanline[0] = SK_ColorRED;
        for (int x = 1; x < width/2; ++x) {
            scanline[x] = SK_ColorBLACK;
        }
        for (int x = width/2; x < width-1; ++x) {
            scanline[x] = SK_ColorWHITE;
        }
        scanline[width-1] = SK_ColorRED;
    }

    for (int y = height/2; y < height-1; ++y) {
        scanline = result->getAddr32(0, y);
        scanline[0] = SK_ColorRED;
        for (int x = 1; x < width/2; ++x) {
            scanline[x] = SK_ColorWHITE;
        }
        for (int x = width/2; x < width-1; ++x) {
            scanline[x] = SK_ColorBLACK;
        }
        scanline[width-1] = SK_ColorRED;
    }

    scanline = result->getAddr32(0, height-1);
    for (int x = 0; x < width; ++x) {
        scanline[x] = SK_ColorRED;
    }
    result->setIsOpaque(true);
    result->setImmutable();
}

// This GM exercises the drawBitmapRectToRect "bleed" flag
class BleedGM : public skiagm::GM {
public:
    BleedGM() {}

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("bleed");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(kWidth, kHeight);
    }

    virtual void onOnceBeforeDraw() SK_OVERRIDE {
        make_red_ringed_bitmap(&fBitmapSmall, kSmallTextureSize, kSmallTextureSize);

        // To exercise the GPU's tiling path we need a texture
        // too big for the GPU to handle in one go
        make_red_ringed_bitmap(&fBitmapBig, 2*kMaxTextureSize, 2*kMaxTextureSize);
    }

    // Draw only the center of the small bitmap
    void drawCase1(SkCanvas* canvas, int transX, int transY,
                   SkCanvas::DrawBitmapRectFlags flags, bool filter) {
        SkRect src = SkRect::MakeXYWH(1, 1,
                                      kSmallTextureSize-2,
                                      kSmallTextureSize-2);
        SkRect dst = SkRect::MakeXYWH(0, 0, SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterBitmap(filter);

        canvas->save();
        canvas->translate(SkIntToScalar(transX), SkIntToScalar(transY));
        canvas->drawBitmapRectToRect(fBitmapSmall, &src, dst, &paint, flags);
        canvas->restore();
    }

    // Draw almost all of the large bitmap
    void drawCase2(SkCanvas* canvas, int transX, int transY,
                   SkCanvas::DrawBitmapRectFlags flags, bool filter) {
        SkRect src = SkRect::MakeXYWH(1, 1,
                                      SkIntToScalar(fBitmapBig.width()-2),
                                      SkIntToScalar(fBitmapBig.height()-2));
        SkRect dst = SkRect::MakeXYWH(0, 0, SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterBitmap(filter);

        canvas->save();
        canvas->translate(SkIntToScalar(transX), SkIntToScalar(transY));
        canvas->drawBitmapRectToRect(fBitmapBig, &src, dst, &paint, flags);
        canvas->restore();
    }

    // Draw ~1/4 of the large bitmap
    void drawCase3(SkCanvas* canvas, int transX, int transY,
                   SkCanvas::DrawBitmapRectFlags flags, bool filter) {
        SkRect src = SkRect::MakeXYWH(1, 1,
                                      SkIntToScalar(fBitmapBig.width()/2-1),
                                      SkIntToScalar(fBitmapBig.height()/2-1));
        SkRect dst = SkRect::MakeXYWH(0, 0, SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterBitmap(filter);

        canvas->save();
        canvas->translate(SkIntToScalar(transX), SkIntToScalar(transY));
        canvas->drawBitmapRectToRect(fBitmapBig, &src, dst, &paint, flags);
        canvas->restore();
    }

    // Draw the center of the small bitmap with a mask filter
    void drawCase4(SkCanvas* canvas, int transX, int transY,
                   SkCanvas::DrawBitmapRectFlags flags, bool filter) {
        SkRect src = SkRect::MakeXYWH(1, 1,
                                      kSmallTextureSize-2,
                                      kSmallTextureSize-2);
        SkRect dst = SkRect::MakeXYWH(0, 0, SkIntToScalar(kBlockSize), SkIntToScalar(kBlockSize));

        SkPaint paint;
        paint.setFilterBitmap(filter);
        SkMaskFilter* mf = SkBlurMaskFilter::Create(SkBlurMaskFilter::kNormal_BlurStyle,
                                         SkBlurMask::ConvertRadiusToSigma(SkIntToScalar(3)));
        paint.setMaskFilter(mf)->unref();

        canvas->save();
        canvas->translate(SkIntToScalar(transX), SkIntToScalar(transY));
        canvas->drawBitmapRectToRect(fBitmapSmall, &src, dst, &paint, flags);
        canvas->restore();
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {

        canvas->clear(SK_ColorGRAY);

        // First draw a column with no bleeding, tiling, or filtering
        this->drawCase1(canvas, kCol0X, kRow0Y, SkCanvas::kNone_DrawBitmapRectFlag, false);
        this->drawCase2(canvas, kCol0X, kRow1Y, SkCanvas::kNone_DrawBitmapRectFlag, false);
        this->drawCase3(canvas, kCol0X, kRow2Y, SkCanvas::kNone_DrawBitmapRectFlag, false);
        this->drawCase4(canvas, kCol0X, kRow3Y, SkCanvas::kNone_DrawBitmapRectFlag, false);

        // Then draw a column with no bleeding or tiling but with filtering
        this->drawCase1(canvas, kCol1X, kRow0Y, SkCanvas::kNone_DrawBitmapRectFlag, true);
        this->drawCase2(canvas, kCol1X, kRow1Y, SkCanvas::kNone_DrawBitmapRectFlag, true);
        this->drawCase3(canvas, kCol1X, kRow2Y, SkCanvas::kNone_DrawBitmapRectFlag, true);
        this->drawCase4(canvas, kCol1X, kRow3Y, SkCanvas::kNone_DrawBitmapRectFlag, true);


#if SK_SUPPORT_GPU
        GrContext* ctx = skiagm::GetGr();
        int oldMaxTextureSize = 0;
        if (NULL != ctx) {
            // shrink the max texture size so all our textures can be reasonably sized
            oldMaxTextureSize = ctx->getMaxTextureSize();
            ctx->setMaxTextureSizeOverride(kMaxTextureSize);
        }
#endif

        // Then draw a column with no bleeding but with tiling and filtering
        this->drawCase1(canvas, kCol2X, kRow0Y, SkCanvas::kNone_DrawBitmapRectFlag, true);
        this->drawCase2(canvas, kCol2X, kRow1Y, SkCanvas::kNone_DrawBitmapRectFlag, true);
        this->drawCase3(canvas, kCol2X, kRow2Y, SkCanvas::kNone_DrawBitmapRectFlag, true);
        this->drawCase4(canvas, kCol2X, kRow3Y, SkCanvas::kNone_DrawBitmapRectFlag, true);

        // Finally draw a column with all three (bleeding, tiling, and filtering)
        this->drawCase1(canvas, kCol3X, kRow0Y, SkCanvas::kBleed_DrawBitmapRectFlag, true);
        this->drawCase2(canvas, kCol3X, kRow1Y, SkCanvas::kBleed_DrawBitmapRectFlag, true);
        this->drawCase3(canvas, kCol3X, kRow2Y, SkCanvas::kBleed_DrawBitmapRectFlag, true);
        this->drawCase4(canvas, kCol3X, kRow3Y, SkCanvas::kBleed_DrawBitmapRectFlag, true);

#if SK_SUPPORT_GPU
        if (NULL != ctx) {
            ctx->setMaxTextureSizeOverride(oldMaxTextureSize);
        }
#endif
    }

private:
    static const int kBlockSize = 90;
    static const int kBlockSpacing = 10;

    static const int kCol0X = kBlockSpacing;
    static const int kCol1X = 2*kBlockSpacing + kBlockSize;
    static const int kCol2X = 3*kBlockSpacing + 2*kBlockSize;
    static const int kCol3X = 4*kBlockSpacing + 3*kBlockSize;
    static const int kWidth = 5*kBlockSpacing + 4*kBlockSize;

    static const int kRow0Y = kBlockSpacing;
    static const int kRow1Y = 2*kBlockSpacing + kBlockSize;
    static const int kRow2Y = 3*kBlockSpacing + 2*kBlockSize;
    static const int kRow3Y = 4*kBlockSpacing + 3*kBlockSize;
    static const int kHeight = 5*kBlockSpacing + 4*kBlockSize;

    static const int kSmallTextureSize = 4;
    static const int kMaxTextureSize = 32;

    SkBitmap fBitmapSmall;
    SkBitmap fBitmapBig;

    typedef GM INHERITED;
};

DEF_GM( return new BleedGM(); )
