/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrContext.h"
#include "GrCaps.h"
#include "GrGpu.h"
#include "GrResourceKey.h"
#include "GrRenderTarget.h"
#include "GrSurfacePriv.h"
#include "GrTexture.h"
#include "GrTexturePriv.h"
#include "GrTypes.h"
#include "SkMath.h"
#include "SkMipMap.h"
#include "SkTypes.h"

void GrTexture::markMipMapsDirty() {
    if (GrMipMapsStatus::kValid == fMipMapsStatus) {
        fMipMapsStatus = GrMipMapsStatus::kDirty;
    }
}

void GrTexture::markMipMapsClean() {
    const bool sizeChanged = GrMipMapsStatus::kNotAllocated == fMipMapsStatus;
    fMipMapsStatus = GrMipMapsStatus::kValid;
    if (sizeChanged) {
        // This must not be called until after changing fMipMapsStatus.
        this->didChangeGpuMemorySize();
        // TODO(http://skbug.com/4548) - The desc and scratch key should be
        // updated to reflect the newly-allocated mipmaps.
    }
}

size_t GrTexture::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(), 1,
                                  this->texturePriv().mipMapped(), false);
}

/////////////////////////////////////////////////////////////////////////////
GrTexture::GrTexture(GrGpu* gpu, const GrSurfaceDesc& desc, GrSLType samplerType,
                     GrSamplerState::Filter highestFilterMode,
                     GrMipMapsStatus mipMapsStatus)
        : INHERITED(gpu, desc)
        , fSamplerType(samplerType)
        , fHighestFilterMode(highestFilterMode)
        , fMipMapsStatus(mipMapsStatus)
        // Mip color mode is explicitly set after creation via GrTexturePriv
        , fMipColorMode(SkDestinationSurfaceColorMode::kLegacy) {
    if (GrMipMapsStatus::kNotAllocated == fMipMapsStatus) {
        fMaxMipMapLevel = 0;
    } else {
        fMaxMipMapLevel = SkMipMap::ComputeLevelCount(this->width(), this->height());
    }
}

bool GrTexture::StealBackendTexture(sk_sp<GrTexture>&& texture,
                                    GrBackendTexture* backendTexture,
                                    SkImage::BackendTextureReleaseProc* releaseProc) {
    if (!texture->surfacePriv().hasUniqueRef() || texture->surfacePriv().hasPendingIO()) {
        return false;
    }

    if (!texture->onStealBackendTexture(backendTexture, releaseProc)) {
        return false;
    }

    // Release any not-stolen data being held by this class.
    texture->onRelease();
    // Abandon the GrTexture so it can't be re-used.
    texture->abandon();

    return true;
}

void GrTexture::computeScratchKey(GrScratchKey* key) const {
    const GrRenderTarget* rt = this->asRenderTarget();
    int sampleCount = 1;
    if (rt) {
        sampleCount = rt->numStencilSamples();
    }
    GrTexturePriv::ComputeScratchKey(this->config(), this->width(), this->height(),
                                     SkToBool(rt), sampleCount,
                                     this->texturePriv().mipMapped(), key);
}

void GrTexturePriv::ComputeScratchKey(GrPixelConfig config, int width, int height,
                                      bool isRenderTarget, int sampleCnt,
                                      GrMipMapped mipMapped, GrScratchKey* key) {
    static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();
    uint32_t flags = isRenderTarget;
    SkASSERT(width > 0);
    SkASSERT(height > 0);
    SkASSERT(sampleCnt > 0);
    SkASSERT(1 == sampleCnt || isRenderTarget);

    // make sure desc.fConfig fits in 5 bits
    SkASSERT(sk_float_log2(kLast_GrPixelConfig) <= 5);
    SkASSERT(static_cast<int>(config) < (1 << 5));
    SkASSERT(sampleCnt < (1 << 8));
    SkASSERT(flags < (1 << 10));
    SkASSERT(static_cast<int>(mipMapped) <= 1);

    GrScratchKey::Builder builder(key, kType, 3);
    builder[0] = width;
    builder[1] = height;
    builder[2] = config | (static_cast<uint8_t>(mipMapped) << 5) | (sampleCnt << 6) | (flags << 14);
}

void GrTexturePriv::ComputeScratchKey(const GrSurfaceDesc& desc, GrScratchKey* key) {
    // Note: the fOrigin field is not used in the scratch key
    return ComputeScratchKey(desc.fConfig, desc.fWidth, desc.fHeight,
                             SkToBool(desc.fFlags & kRenderTarget_GrSurfaceFlag), desc.fSampleCnt,
                             GrMipMapped::kNo, key);
}
