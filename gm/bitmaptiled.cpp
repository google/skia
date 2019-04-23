/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/gpu/GrContext.h"

// This test exercises Ganesh's drawing of tiled bitmaps. In particular, that the offsets and the
// extents of the tiles don't causes gaps between tiles.
static void draw_tile_bitmap_with_fractional_offset(GrContext* context, SkCanvas* canvas,
                                                    bool vertical) {
    // This should match kBmpSmallTileSize in SkGpuDevice.cpp. Note that our canvas size is tuned
    // to this constant as well.
    const int kTileSize = 1 << 10;

    // We're going to draw a section of the bitmap that intersects 3 tiles (3x1 or 1x3).
    // We need that to be < 50% of the total image, so our image is 7 tiles (7x1 or 1x7).
    const int kBitmapLongEdge = 7 * kTileSize;
    const int kBitmapShortEdge = 1 * kTileSize;

    // To trigger tiling, we also need the image to be more than 50% of the cache, so we ensure the
    // cache is sized to make that true.
    const int kBitmapArea = kBitmapLongEdge * kBitmapShortEdge;
    const size_t kBitmapBytes = kBitmapArea * sizeof(SkPMColor);

    int oldMaxResources;
    size_t oldMaxResourceBytes;
    context->getResourceCacheLimits(&oldMaxResources, &oldMaxResourceBytes);

    const size_t newMaxResourceBytes = kBitmapBytes + (kBitmapBytes / 2);
    context->setResourceCacheLimits(oldMaxResources, newMaxResourceBytes);

    // Construct our bitmap as either very wide or very tall
    SkBitmap bmp;
    bmp.allocN32Pixels(vertical ? kBitmapShortEdge : kBitmapLongEdge,
                       vertical ? kBitmapLongEdge : kBitmapShortEdge, true);
    bmp.eraseColor(SK_ColorWHITE);

    // Draw ten strips with varying fractional offset to catch any rasterization issues with tiling
    for (int i = 0; i < 10; ++i) {
        float offset = i * 0.1f;
        if (vertical) {
            canvas->drawBitmapRect(bmp, SkRect::MakeXYWH(0.0f, (kTileSize - 50) + offset,
                                                         32.0f, 1124.0f),
                                   SkRect::MakeXYWH(37.0f * i, 0.0f, 32.0f, 1124.0f), nullptr);
        } else {
            canvas->drawBitmapRect(bmp, SkRect::MakeXYWH((kTileSize - 50) + offset, 0.0f,
                                                         1124.0f, 32.0f),
                                   SkRect::MakeXYWH(0.0f, 37.0f * i, 1124.0f, 32.0f), nullptr);
        }
    }

    // Restore the cache
    context->setResourceCacheLimits(oldMaxResources, oldMaxResourceBytes);
}

DEF_SIMPLE_GPU_GM_BG(
        bitmaptiled_fractional_horizontal, context, rtc, canvas, 1124, 365, SK_ColorBLACK) {
    draw_tile_bitmap_with_fractional_offset(context, canvas, false);
}

DEF_SIMPLE_GPU_GM_BG(
        bitmaptiled_fractional_vertical, context, rtc, canvas, 365, 1124, SK_ColorBLACK) {
    draw_tile_bitmap_with_fractional_offset(context, canvas, true);
}
