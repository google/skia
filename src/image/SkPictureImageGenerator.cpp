/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/image/SkPictureImageGenerator.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageGenerator.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPicture.h"
#include "include/core/SkSize.h"
#include "src/base/SkTLazy.h"
#include "src/image/SkImageGeneratorPriv.h"

#include <memory>
#include <utility>

namespace SkImageGenerators {
std::unique_ptr<SkImageGenerator> MakeFromPicture(
        const SkISize& size,
        sk_sp<SkPicture> picture,
        const SkMatrix* matrix,
        const SkPaint* paint,
        SkImages::BitDepth bitDepth,
        sk_sp<SkColorSpace> colorSpace) {
    return MakeFromPicture(size, picture, matrix, paint, bitDepth, colorSpace, {});
}

std::unique_ptr<SkImageGenerator> MakeFromPicture(const SkISize& size,
                                                  sk_sp<SkPicture> picture,
                                                  const SkMatrix* matrix,
                                                  const SkPaint* paint,
                                                  SkImages::BitDepth bitDepth,
                                                  sk_sp<SkColorSpace> colorSpace,
                                                  SkSurfaceProps props) {
    if (!picture || !colorSpace || size.isEmpty()) {
        return nullptr;
    }

    SkColorType colorType = kN32_SkColorType;
    if (SkImages::BitDepth::kF16 == bitDepth) {
        colorType = kRGBA_F16_SkColorType;
    }

    SkImageInfo info =
            SkImageInfo::Make(size, colorType, kPremul_SkAlphaType, std::move(colorSpace));
    return std::unique_ptr<SkImageGenerator>(
        new SkPictureImageGenerator(info, std::move(picture), matrix, paint, props));
}
} // SkImageGenerators

///////////////////////////////////////////////////////////////////////////////////////////////////

SkPictureImageGenerator::SkPictureImageGenerator(const SkImageInfo& info, sk_sp<SkPicture> picture,
                                                 const SkMatrix* matrix, const SkPaint* paint,
                                                 const SkSurfaceProps& props)
        : SkImageGenerator(info)
        , fPicture(std::move(picture))
        , fProps(props) {

    if (matrix) {
        fMatrix = *matrix;
    } else {
        fMatrix.reset();
    }

    if (paint) {
        fPaint.set(*paint);
    }
}

bool SkPictureImageGenerator::onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                          const Options& opts) {
    std::unique_ptr<SkCanvas> canvas = SkCanvas::MakeRasterDirect(info, pixels, rowBytes, &fProps);
    if (!canvas) {
        return false;
    }
    canvas->clear(0);
    canvas->drawPicture(fPicture, &fMatrix, fPaint.getMaybeNull());
    return true;
}
