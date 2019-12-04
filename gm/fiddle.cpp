/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <tools/Resources.h>
#include "gm/gm.h"
#include "include/encode/SkJpegEncoder.h"
#include "skia.h"
#include "src/core/SkCachedData.h"
#include "src/image/SkImage_Base.h"

static void draw(SkCanvas* canvas);
DEF_SIMPLE_GM(fiddle, canvas, 256, 256) { draw(canvas); }

sk_sp<SkImage> make_image(GrContext* context) {
    SkBitmap bmp;
    bmp.allocPixels(SkImageInfo::Make(3, 3, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    auto pixels = static_cast<uint32_t*>(bmp.getPixels());
    pixels[ 0] = 0xFF000000; pixels[ 1] = 0xFFFF0000; pixels[ 2] = 0xFF00FF00; pixels[ 3] = 0xFFFFFF00; pixels[ 4] = 0xFF0000FF;
    pixels[ 5] = 0xFFFF00FF; pixels[ 6] = 0xFF00FFFF; pixels[ 7] = 0xFFFFFFFF; pixels[ 8] = 0xFF0F0F0F; pixels[ 9] = 0xFFF0F0F0;
    pixels[10] = 0xFFFF00FF; pixels[11] = 0xFF00FFFF; pixels[12] = 0xFFFFFFFF; pixels[13] = 0xFF0F0F0F; pixels[14] = 0xFFF0F0F0;
    pixels[15] = 0xFFFF00FF; pixels[16] = 0xFF00FFFF; pixels[17] = 0xFFFFFFFF; pixels[18] = 0xFF0F0F0F; pixels[17] = 0xFFF0F0F0;
    pixels[20] = 0xFFFF00FF; pixels[21] = 0xFF00FFFF; pixels[26] = 0xFFFFFFFF; pixels[27] = 0xFF0F0F0F; pixels[28] = 0xFFF0F0F0;
    bmp.notifyPixelsChanged();
    SkImage::MakeFromBitmap(bmp);
    SkDynamicMemoryWStream stream;
    SkJpegEncoder::Options options;
    options.fDownsample = SkJpegEncoder::Downsample::k420;
    options.fQuality = 100;
    if (!SkJpegEncoder::Encode(&stream, bmp.pixmap(), options)) {
        return nullptr;
    }
    auto image = SkImage::MakeFromEncoded(stream.detachAsData());
    //auto image = GetResourceAsImage("images/inception.jpg");
    return image;
    if (!context) {
        return image;
    }
    SkYUVASizeInfo info;
    SkYUVAIndex indices[4];
    SkYUVColorSpace cs;
    const void* planes[4];
    if (!as_IB(image)->getPlanes(&info, indices, &cs, planes)) {
        return nullptr;
    }
    SkPixmap pixmaps[4];
    //SkASSERT(info.fSizes[0] == (SkISize{3, 3}));
    //SkASSERT(info.fSizes[1] == (SkISize{2, 2}));
    //SkASSERT(info.fSizes[2] == (SkISize{2, 2}));
    for (int i = 0; i < 4; ++i) {
        if (!info.fSizes[i].isZero()) {
            pixmaps[i].reset(SkImageInfo::MakeA8(info.fSizes[i]), planes[i], info.fWidthBytes[i]);
        }
    }
    SkDebugf("i: %d %d\n", image->width(), image->height());
    SkDebugf("y: %d %d\n", pixmaps[0].width(), pixmaps[0].height());
    SkDebugf("u: %d %d\n", pixmaps[1].width(), pixmaps[1].height());
    SkDebugf("v: %d %d\n", pixmaps[2].width(), pixmaps[2].height());
    return SkImage::MakeFromYUVAPixmaps(context, cs, pixmaps, indices, image->dimensions(), kTopLeft_GrSurfaceOrigin, false);
}

// Paste your fiddle.skia.org code over this stub.
void draw(SkCanvas* canvas) {
    canvas->scale(40, 40);
    auto image = make_image(canvas->getGrContext());
    canvas->drawImage(image, 0, 0);
}
