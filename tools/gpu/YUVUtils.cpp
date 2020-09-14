/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/YUVUtils.h"

#include "include/core/SkData.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/codec/SkCodecImageGenerator.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRecordingContextPriv.h"

namespace sk_gpu_test {

std::unique_ptr<LazyYUVImage> LazyYUVImage::Make(sk_sp<SkData> data, GrMipmapped mipmapped) {
    std::unique_ptr<LazyYUVImage> image(new LazyYUVImage());
    if (image->reset(std::move(data), mipmapped)) {
        return image;
    } else {
        return nullptr;
    }
}

sk_sp<SkImage> LazyYUVImage::refImage(GrRecordingContext* rContext) {
    if (this->ensureYUVImage(rContext)) {
        return fYUVImage;
    } else {
        return nullptr;
    }
}

const SkImage* LazyYUVImage::getImage(GrRecordingContext* rContext) {
    if (this->ensureYUVImage(rContext)) {
        return fYUVImage.get();
    } else {
        return nullptr;
    }
}

bool LazyYUVImage::reset(sk_sp<SkData> data, GrMipmapped mipmapped) {
    fMipmapped = mipmapped;
    auto codec = SkCodecImageGenerator::MakeFromEncodedCodec(data);
    if (!codec) {
        return false;
    }

    SkYUVAPixmapInfo yuvaPixmapInfo;
    if (!codec->queryYUVAInfo(SkYUVAPixmapInfo::SupportedDataTypes::All(), &yuvaPixmapInfo)) {
        return false;
    }
    fPixmaps = SkYUVAPixmaps::Allocate(yuvaPixmapInfo);
    if (!fPixmaps.isValid()) {
        return false;
    }

    if (!codec->getYUVAPlanes(fPixmaps)) {
        return false;
    }

    if (!fPixmaps.toLegacy(&fSizeInfo, fComponents)) {
        return false;
    }
    // The SkPixmap data is fully configured now for MakeFromYUVAPixmaps once we get a GrContext
    return true;
}

bool LazyYUVImage::ensureYUVImage(GrRecordingContext* rContext) {
    if (!rContext) {
        return false; // Cannot make a YUV image from planes
    }
    if (fYUVImage && fYUVImage->isValid(rContext)) {
        return true; // Have already made a YUV image valid for this context.
    }
    // Try to make a new YUV image for this context.
    fYUVImage = SkImage::MakeFromYUVAPixmaps(rContext, fPixmaps, fMipmapped, false, nullptr);
    return fYUVImage != nullptr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void YUVABackendReleaseContext::Unwind(GrDirectContext* dContext,
                                       YUVABackendReleaseContext* beContext,
                                       bool fullFlush) {

    // Some backends (e.g., Vulkan) require that all work associated w/ texture
    // creation be completed before deleting the textures.
    if (fullFlush) {
        // If the release context client performed some operations other than backend texture
        // creation then we may require a full flush to ensure that all the work is completed.
        dContext->flush();
        dContext->submit(true);
    } else {
        dContext->submit();

        while (!beContext->creationCompleted()) {
            dContext->checkAsyncWorkCompletion();
        }
    }

    delete beContext;
}

YUVABackendReleaseContext::YUVABackendReleaseContext(GrDirectContext* dContext)
        : fDContext(dContext) {
}

YUVABackendReleaseContext::~YUVABackendReleaseContext() {
    for (int i = 0; i < 4; ++i) {
        if (fBETextures[i].isValid()) {
            SkASSERT(fCreationComplete[i]);
            fDContext->deleteBackendTexture(fBETextures[i]);
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
