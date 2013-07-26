/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"

namespace skiagm {
extern GrContext* GetGr();
};

void GrContext::setMaxTextureSizeOverride(int maxTextureSizeOverride) {
    fMaxTextureSizeOverride = maxTextureSizeOverride;
}
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

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
#if SK_SUPPORT_GPU
        GrContext* ctx = skiagm::GetGr();
        int oldMaxTextureSize = 0;
        if (NULL != ctx) {
            // shrink the max texture size so all our textures can be reasonably sized
            oldMaxTextureSize = ctx->getMaxTextureSize();
            ctx->setMaxTextureSizeOverride(kMaxTextureSize);
        }
#endif

        canvas->clear(SK_ColorGRAY);

        SkPaint paint;

        // Bleeding only comes into play when filtering
        paint.setFilterBitmap(true);

        // carve out the center of the small bitmap
        SkRect src = SkRect::MakeXYWH(1, 1,
                                      kSmallTextureSize-2,
                                      kSmallTextureSize-2);
        SkRect dst = SkRect::MakeXYWH(10, 10, 100, 100);

        // first draw without bleeding
        canvas->drawBitmapRectToRect(fBitmapSmall, &src, dst, &paint);

        // then draw with bleeding
        dst = SkRect::MakeXYWH(120, 10, 100, 100);
        canvas->drawBitmapRectToRect(fBitmapSmall, &src, dst, &paint);

        // Next test out the GPU's tiling of large textures

        // first draw almost the whole thing
        src = SkRect::MakeXYWH(1, 1,
                               SkIntToScalar(fBitmapBig.width()-2),
                               SkIntToScalar(fBitmapBig.height()-2));
        dst = SkRect::MakeXYWH(10, 120, 100, 100);

        // first without bleeding
        canvas->drawBitmapRectToRect(fBitmapBig, &src, dst, &paint);

        // then with bleeding
        dst = SkRect::MakeXYWH(120, 120, 100, 100);
        canvas->drawBitmapRectToRect(fBitmapBig, &src, dst, &paint);

        // next draw ~1/4 of the bitmap
        src = SkRect::MakeXYWH(1, 1,
                               SkIntToScalar(fBitmapBig.width()/2-1),
                               SkIntToScalar(fBitmapBig.height()/2-1));
        dst = SkRect::MakeXYWH(10, 230, 100, 100);

        // first without bleeding
        canvas->drawBitmapRectToRect(fBitmapBig, &src, dst, &paint);

        // then with bleeding
        dst = SkRect::MakeXYWH(120, 230, 100, 100);
        canvas->drawBitmapRectToRect(fBitmapBig, &src, dst, &paint);

#if SK_SUPPORT_GPU
        if (NULL != ctx) {
            ctx->setMaxTextureSizeOverride(oldMaxTextureSize);
        }
#endif
    }

private:
    static const int kWidth = 230;
    static const int kHeight = 340;

    static const int kSmallTextureSize = 4;
    static const int kMaxTextureSize = 32;

    SkBitmap fBitmapSmall;
    SkBitmap fBitmapBig;

    typedef GM INHERITED;
};

DEF_GM( return new BleedGM(); )
