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

///////////////////////////////////////////////////////////////////////////////////////////////////
void YUVABackendReleaseContext::Unwind(GrContext* context, YUVABackendReleaseContext* beContext,
                                       bool fullFlush) {

    // Some backends (e.g., Vulkan) require that all work associated w/ texture
    // creation be completed before deleting the textures.
    if (fullFlush) {
        // If the release context client performed some operations other than backend texture
        // creation then we may require a full flush to ensure that all the work is completed.
        context->flush();
        context->submit(true);
    } else {
        context->submit();

        while (!beContext->creationCompleted()) {
            context->checkAsyncWorkCompletion();
        }
    }

    delete beContext;
}

YUVABackendReleaseContext::YUVABackendReleaseContext(GrContext* context) : fContext(context) {
    SkASSERT(context->priv().getGpu());
    SkASSERT(context->asDirectContext());
}

YUVABackendReleaseContext::~YUVABackendReleaseContext() {
    for (int i = 0; i < 4; ++i) {
        if (fBETextures[i].isValid()) {
            SkASSERT(fCreationComplete[i]);
            fContext->deleteBackendTexture(fBETextures[i]);
        }
    }
}

template<int I> static void CreationComplete(void* releaseContext) {
    auto beContext = reinterpret_cast<YUVABackendReleaseContext*>(releaseContext);
    beContext->setCreationComplete(I);
}

GrGpuFinishedProc YUVABackendReleaseContext::CreationCompleteProc(int index) {
    SkASSERT(index >= 0 && index < 4);

    switch (index) {
        case 0: return CreationComplete<0>;
        case 1: return CreationComplete<1>;
        case 2: return CreationComplete<2>;
        case 3: return CreationComplete<3>;
    }

    SK_ABORT("Invalid YUVA Index.");
    return nullptr;
}

} // namespace sk_gpu_test
