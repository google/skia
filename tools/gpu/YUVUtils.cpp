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

YUVAPixmaps::YUVAPixmaps(const SkYUVASpec& spec,
                         SkColorType colorTypes[SkYUVASpec::kMaxPlanes],
                         size_t rowbytes[SkYUVASpec::kMaxPlanes])
        : fSpec(spec) {
    if (spec.fDimensions.isEmpty()) {
        return;
    }
    SkISize planeDims[SkYUVASpec::kMaxPlanes];
    SkImageInfo ii[SkYUVASpec::kMaxPlanes];
    int numPlanes = spec.expectedPlaneDims(planeDims);
    size_t totalBytes = 0;
    for (int i = 0; i < numPlanes; ++i) {
        ii[i] = SkImageInfo::Make(planeDims[i], colorTypes[i], kPremul_SkAlphaType);
        if (!ii[i].validRowBytes(rowbytes[i])) {
            return;
        }
        totalBytes += ii[i].height()*rowbytes[i];
    }

    fStorage.reset(new char[totalBytes]);
    char* addr = fStorage.get();
    for (int i = 0; i < numPlanes; ++i) {
        fPlanes[i].reset(ii[i], addr, rowbytes[i]);
        addr += fPlanes[i].height()*rowbytes[i];
    }
    fIsValid = true;
}

bool YUVAPixmaps::toLegacy(SkYUVASizeInfo* yuvaSizeInfo, SkYUVAIndex yuvaIndices[4]) {
    if (!this->isValid()) {
        return false;
    }
    bool ok = true;
    auto getIthChannel = [&ok](SkColorType ct, int idx) -> SkColorChannel {
      int cnt = 0;
      switch (SkColorTypeChannelFlags(ct)) {
          case kAlpha_SkColorChannelFlag:
              ok |= idx == 0;
              return SkColorChannel::kA;
          case kGray_SkColorChannelFlag:
          case kRed_SkColorChannelFlag:
              cnt += 1;
              [[fallthrough]];
          case kRG_SkColorChannelFlags:
              cnt += 1;
              [[fallthrough]];
          case kRGB_SkColorChannelFlags:
              cnt += 1;
              [[fallthrough]];
          case kRGBA_SkColorChannelFlags:
              cnt += 1;
              ok |= idx < cnt;
              return static_cast<SkColorChannel>(idx);
          default:
              ok = false;
              return SkColorChannel::kR;
      }
    };
    SkColorType cts[] = {fPlanes[0].colorType(),
                         fPlanes[1].colorType(),
                         fPlanes[2].colorType(),
                         fPlanes[3].colorType()};
    switch (fSpec.fPlanes) {
        case SkYUVASpec::Planes::kY_U_V_444:
        case SkYUVASpec::Planes::kY_U_V_422:
        case SkYUVASpec::Planes::kY_U_V_420:
        case SkYUVASpec::Planes::kY_U_V_440:
        case SkYUVASpec::Planes::kY_U_V_411:
        case SkYUVASpec::Planes::kY_U_V_410:
            yuvaIndices[SkYUVAIndex::kY_Index].fIndex   = 0;
            yuvaIndices[SkYUVAIndex::kY_Index].fChannel = getIthChannel(cts[0], 0);
            yuvaIndices[SkYUVAIndex::kU_Index].fIndex   = 1;
            yuvaIndices[SkYUVAIndex::kU_Index].fChannel = getIthChannel(cts[1], 0);
            yuvaIndices[SkYUVAIndex::kV_Index].fIndex   = 2;
            yuvaIndices[SkYUVAIndex::kV_Index].fChannel = getIthChannel(cts[2], 0);
            yuvaIndices[SkYUVAIndex::kA_Index].fIndex   = -1;
            break;
    }
    if (!ok) {
        return false;
    }
    int n = fSpec.numPlanes();
    for (int i = 0; i < n; ++i) {
        yuvaSizeInfo->fSizes[i] = fPlanes[i].dimensions();
        yuvaSizeInfo->fWidthBytes[i] = fPlanes[i].rowBytes();
    }
    yuvaSizeInfo->fOrigin = fSpec.fOrigin;
    return true;
}

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

    SkColorType colorTypes[4];
    size_t rowBytes[4];
    SkYUVASpec spec;
    if (!codec->queryYUVASpec(&spec, colorTypes, rowBytes)) {
        return false;
    }
    fPixmaps = YUVAPixmaps(spec, colorTypes, rowBytes);
    if (!fPixmaps.isValid()) {
        return false;
    }

    if (!codec->getYUVAPlanes(fPixmaps.planes())) {
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
    fYUVImage = SkImage::MakeFromYUVAPixmaps(rContext->priv().backdoor(),
                                             fPixmaps.spec().fYUVColorSpace,
                                             fPixmaps.planes(),
                                             fComponents,
                                             fSizeInfo.fSizes[0],
                                             kTopLeft_GrSurfaceOrigin,
                                             static_cast<bool>(fMipmapped),
                                             false);
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
