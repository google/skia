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
#include "include/encode/SkJpegEncoder.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/utils/SkRandom.h"
#include "src/core/SkCachedData.h"
#include "src/image/SkImage_Base.h"
#include "tools/gpu/YUVUtils.h"

static constexpr int kScale = 10;
static constexpr SkISize kImageDim = {5, 5};

static sk_sp<SkImage> make_image(GrRecordingContext* rContext) {
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
    SkDynamicMemoryWStream stream;
    SkJpegEncoder::Options options;
    options.fDownsample = SkJpegEncoder::Downsample::k420;
    options.fQuality = 100;
    if (!SkJpegEncoder::Encode(&stream, bmp.pixmap(), options)) {
        return nullptr;
    }
    auto imageHelper = sk_gpu_test::LazyYUVImage::Make(stream.detachAsData());
    if (!imageHelper) {
        return nullptr;
    }
    return imageHelper->refImage(rContext, sk_gpu_test::LazyYUVImage::Type::kFromPixmaps);
}

// This GM tests that the YUVA image code path in the GPU backend handles odd sized images with
// 420 chroma subsampling correctly.
DEF_SIMPLE_GM_CAN_FAIL(yuv420_odd_dim, canvas, errMsg,
                       kScale* kImageDim.width(), kScale* kImageDim.height()) {
    auto rContext = canvas->recordingContext();
    if (!rContext) {
        // This GM exists to exercise GPU planar images.
        return skiagm::DrawResult::kSkip;
    }
    auto image = make_image(rContext);
    if (!image) {
        return rContext->abandoned() ? skiagm::DrawResult::kOk : skiagm::DrawResult::kFail;
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
