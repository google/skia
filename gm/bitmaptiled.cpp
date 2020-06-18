/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"

class GrRenderTargetContext;

namespace skiagm {

// This test exercises Ganesh's drawing of tiled bitmaps. In particular, that the offsets and the
// extents of the tiles don't causes gaps between tiles.
class BitmapTiledGM : public GpuGM {
public:
    BitmapTiledGM(bool vertical) : fVertical(vertical) {
        this->setBGColor(SK_ColorBLACK);
    }

protected:
    SkString onShortName() override {
        return fVertical ? SkString("bitmaptiled_fractional_vertical")
                         : SkString("bitmaptiled_fractional_horizontal");
    }

    SkISize onISize() override {
        return fVertical ? SkISize::Make(365, 1124) : SkISize::Make(1124, 365);
    }

    DrawResult onGpuSetup(GrContext* context, SkString* errorMsg) override {
        if (!context) {
            return DrawResult::kSkip;
        }

        SkASSERT(context->priv().asDirectContext());

        // To trigger tiling, we need the image to be more than 50% of the cache, so we ensure the
        // cache is sized to make that true.
        const int kBitmapArea = kBitmapLongEdge * kBitmapShortEdge;
        const size_t kBitmapBytes = kBitmapArea * sizeof(SkPMColor);

        const size_t newMaxResourceBytes = kBitmapBytes + (kBitmapBytes / 2);
        context->setResourceCacheLimit(newMaxResourceBytes);

        return DrawResult::kOk;
    }

    DrawResult onDraw(GrContext*, GrRenderTargetContext*, SkCanvas* canvas,  SkString*) override {
        // Construct our bitmap as either very wide or very tall
        SkBitmap bmp;
        bmp.allocN32Pixels(fVertical ? kBitmapShortEdge : kBitmapLongEdge,
                           fVertical ? kBitmapLongEdge  : kBitmapShortEdge, true);
        bmp.eraseColor(SK_ColorRED);

        // Draw ten strips with varying fractional offset to catch any rasterization issues with tiling
        for (int i = 0; i < 10; ++i) {
            float offset = i * 0.1f;
            if (fVertical) {
                canvas->drawBitmapRect(bmp, SkRect::MakeXYWH(0.0f, (kTileSize - 50) + offset,
                                                             32.0f, 1124.0f),
                                       SkRect::MakeXYWH(37.0f * i, 0.0f, 32.0f, 1124.0f), nullptr);
            } else {
                canvas->drawBitmapRect(bmp, SkRect::MakeXYWH((kTileSize - 50) + offset, 0.0f,
                                                             1124.0f, 32.0f),
                                       SkRect::MakeXYWH(0.0f, 37.0f * i, 1124.0f, 32.0f), nullptr);
            }
        }

        return DrawResult::kOk;
    }

private:
    // This should match kBmpSmallTileSize in SkGpuDevice.cpp. Note that our canvas size is tuned
    // to this constant as well.
    static const int kTileSize = 1 << 10;

    // We're going to draw a section of the bitmap that intersects 3 tiles (3x1 or 1x3).
    // We need that to be < 50% of the total image, so our image is 7 tiles (7x1 or 1x7).
    static const int kBitmapLongEdge = 7 * kTileSize;
    static const int kBitmapShortEdge = 1 * kTileSize;

    bool fVertical;

    typedef GpuGM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new BitmapTiledGM(true);)
DEF_GM(return new BitmapTiledGM(false);)

} // skiagm

