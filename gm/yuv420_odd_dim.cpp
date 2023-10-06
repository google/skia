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
#include "include/core/SkShader.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTileMode.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "src/base/SkRandom.h"
#include "src/core/SkCachedData.h"
#include "src/image/SkImage_Base.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
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
        surface = SkSurfaces::Raster(info);
    }
    surface->getCanvas()->drawImage(image, 0, 0);
    canvas->scale(kScale, kScale);
    canvas->drawImage(surface->makeImageSnapshot(), 0, 0);
    return skiagm::DrawResult::kOk;
}

// crbug.com/1210557 Subsampled planes weren't repeated at the correct frequency.
DEF_SIMPLE_GM_CAN_FAIL(yuv420_odd_dim_repeat, canvas, errMsg,
                       1000,
                       500) {
    auto rContext = canvas->recordingContext();
    if (!rContext) {
        // This GM exists to exercise GPU planar images.
        return skiagm::DrawResult::kSkip;
    }
    auto image = ToolUtils::GetResourceAsImage("images/mandrill_256.png");
    if (!image) {
        return rContext->abandoned() ? skiagm::DrawResult::kOk : skiagm::DrawResult::kFail;
    }
    // Make sure the image is odd dimensioned.
    int w = image->width()  & 0b1 ? image->width()  : image->width()  - 1;
    int h = image->height() & 0b1 ? image->height() : image->height() - 1;
    image = image->makeSubset(nullptr, SkIRect::MakeWH(w, h));

    auto [planes, yuvaInfo] = sk_gpu_test::MakeYUVAPlanesAsA8(image.get(),
                                                              kJPEG_SkYUVColorSpace,
                                                              SkYUVAInfo::Subsampling::k420,
                                                              nullptr);
    SkPixmap pixmaps[4];
    for (int i = 0; i < yuvaInfo.numPlanes(); ++i) {
        planes[i]->peekPixels(&pixmaps[i]);
    }
    auto yuvaPixmaps = SkYUVAPixmaps::FromExternalPixmaps(yuvaInfo, pixmaps);
    image = SkImages::TextureFromYUVAPixmaps(canvas->recordingContext(),
                                             yuvaPixmaps,
                                             skgpu::Mipmapped::kYes,
                                             /* limit to max tex size */ false,
                                             /* color space */ nullptr);
    if (!image) {
        *errMsg = "Could not make YUVA image";
        return rContext->abandoned() ? skiagm::DrawResult::kSkip : skiagm::DrawResult::kFail;
    }
    int i = 0;
    for (SkMipmapMode mm : {SkMipmapMode::kNone, SkMipmapMode::kLinear}) {
        int j = 0;
        for (SkFilterMode filter : {SkFilterMode::kNearest, SkFilterMode::kLinear}) {
            canvas->save();
            canvas->clipRect(SkRect::MakeXYWH(500.f*j, 250.f*i, 500.f, 250.f));
            canvas->rotate(30.f);
            canvas->scale(0.4f, 0.4f);  // so mipmaps sampling doesn't just use base level.
            // Large translation so that if U/V planes aren't repeated correctly WRT to Y plane we
            // accumulate a lot of error.
            canvas->translate(-240000.f, -240000.f);
            auto shader = image->makeShader(SkTileMode::kRepeat,
                                            SkTileMode::kRepeat,
                                            SkSamplingOptions(filter, mm));
            SkPaint paint;
            paint.setShader(std::move(shader));
            canvas->drawPaint(paint);
            canvas->restore();
            ++j;
        }
        ++i;
    }
    return skiagm::DrawResult::kOk;
}
