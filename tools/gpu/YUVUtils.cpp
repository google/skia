/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/YUVUtils.h"

#include "include/core/SkData.h"
#include "include/gpu/GrContext.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/gpu/GrContextPriv.h"

namespace sk_gpu_test {

std::unique_ptr<LazyYUVImage> LazyYUVImage::Make(sk_sp<SkData> data) {
    std::unique_ptr<LazyYUVImage> image(new LazyYUVImage());
    if (image->reset(std::move(data))) {
        return image;
    } else {
        return nullptr;
    }
}

sk_sp<SkImage> LazyYUVImage::refImage(GrContext* context) {
    if (this->ensureYUVImage(context)) {
        return fYUVImage;
    } else {
        return nullptr;
    }
}

const SkImage* LazyYUVImage::getImage(GrContext* context) {
    if (this->ensureYUVImage(context)) {
        return fYUVImage.get();
    } else {
        return nullptr;
    }
}

bool LazyYUVImage::reset(sk_sp<SkData> data) {
    auto codec = SkCodecImageGenerator::MakeFromEncodedCodec(data);
    if (!codec) {
        return false;
    }

    if (!codec->queryYUVA8(&fSizeInfo, fComponents, &fColorSpace)) {
        return false;
    }

    fPlaneData.reset(fSizeInfo.computeTotalBytes());
    void* planes[SkYUVASizeInfo::kMaxCount];
    fSizeInfo.computePlanes(fPlaneData.get(), planes);
    if (!codec->getYUVA8Planes(fSizeInfo, fComponents, planes)) {
        return false;
    }

    for (int i = 0; i < SkYUVASizeInfo::kMaxCount; ++i) {
        if (fSizeInfo.fSizes[i].isEmpty()) {
            fPlanes[i].reset();
        } else {
            SkASSERT(planes[i]);
            auto planeInfo = SkImageInfo::Make(fSizeInfo.fSizes[i].fWidth,
                                               fSizeInfo.fSizes[i].fHeight,
                                               kGray_8_SkColorType, kOpaque_SkAlphaType, nullptr);
            fPlanes[i].reset(planeInfo, planes[i], fSizeInfo.fWidthBytes[i]);
        }
    }
    // The SkPixmap data is fully configured now for MakeFromYUVAPixmaps once we get a GrContext
    return true;
}

bool LazyYUVImage::ensureYUVImage(GrContext* context) {
    if (!context) {
        return false; // Cannot make a YUV image from planes
    }
    if (context->priv().contextID() == fOwningContextID) {
        return fYUVImage != nullptr; // Have already made a YUV image (or tried and failed)
    }
    // Must make a new YUV image
    fYUVImage = SkImage::MakeFromYUVAPixmaps(context, fColorSpace, fPlanes, fComponents,
            fSizeInfo.fSizes[0], kTopLeft_GrSurfaceOrigin, false, false);
    fOwningContextID = context->priv().contextID();
    return fYUVImage != nullptr;
}

} // namespace sk_gpu_test
