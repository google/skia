/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAIndex.h"
#include "include/core/SkYUVASizeInfo.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/gpu/GrContext.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkCachedData.h"
#include "src/image/SkImage_Base.h"

static constexpr int kScale = 10;
static constexpr SkISize kImageDim = {5, 5};

static sk_sp<SkImage> make_image(GrContext* context) {
    // Generate a small jpeg with odd dimensions.
    SkBitmap bmp;
    bmp.allocPixels(SkImageInfo::Make(kImageDim, kRGBA_8888_SkColorType, kPremul_SkAlphaType));
    SkRandom random;
    // These random values won't compress well, but it doesn't matter. This test exists to
    // compare the GPU YUV code path to the SW.
    for (int y = 0; y < bmp.height(); ++y) {
        for (int x = 0; x < bmp.width(); ++x) {
            *bmp.getAddr32(x, y) = random.nextU() | 0xFF000000;
        }
    }
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
#ifdef SK_DEBUG
    static constexpr SkISize kUVDim = {kImageDim.width() / 2 + 1, kImageDim.height() / 2 + 1};
#endif
    SkASSERT(info.fSizes[0] == kImageDim);
    SkASSERT(info.fSizes[1] == kUVDim);
    SkASSERT(info.fSizes[2] == kUVDim);
    for (int i = 0; i < 4; ++i) {
        if (!info.fSizes[i].isZero()) {
            pixmaps[i].reset(SkImageInfo::MakeA8(info.fSizes[i]), planes[i], info.fWidthBytes[i]);
        }
    }
    return SkImage::MakeFromYUVAPixmaps(context, cs, pixmaps, indices, image->dimensions(),
                                        kTopLeft_GrSurfaceOrigin, false);
}

// This GM tests that the YUVA image code path in the GPU backend handles odd sized images with
// 420 chroma subsampling correctly.
DEF_SIMPLE_GM_CAN_FAIL(yuv420_odd_dim, canvas, errMsg,
                       kScale* kImageDim.width(), kScale* kImageDim.height()) {
    auto image = make_image(canvas->getGrContext());
    if (!image) {
        if (canvas->getGrContext() && canvas->getGrContext()->abandoned()) {
            return skiagm::DrawResult::kOk;
        }
        return skiagm::DrawResult::kFail;
    }
    // We draw the image offscreen and then blow it up using nearest filtering by kScale.
    // This avoids skbug.com/9693
    sk_sp<SkSurface> surface;
    if (auto origSurface = canvas->getSurface()) {
        surface = origSurface->makeSurface(image->width(), image->height());
    } else {
        auto ct = canvas->imageInfo().colorType();
        if (ct == kUnknown_SkColorType) {
            ct = image->colorType();
        }
        auto info = canvas->imageInfo().makeColorType(ct);
        info = info.makeAlphaType(kPremul_SkAlphaType);
        surface = SkSurface::MakeRaster(info);
    }
    surface->getCanvas()->drawImage(image, 0, 0);
    canvas->scale(kScale, kScale);
    canvas->drawImage(surface->makeImageSnapshot(), 0, 0);
    return skiagm::DrawResult::kOk;
}
