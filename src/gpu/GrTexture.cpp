/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMath.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTexture.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrResourceKey.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrSurfacePriv.h"
#include "src/gpu/GrTexturePriv.h"

void GrTexture::markMipMapsDirty() {
    if (GrMipMapsStatus::kValid == fMipMapsStatus) {
        fMipMapsStatus = GrMipMapsStatus::kDirty;
    }
}

void GrTexture::markMipMapsClean() {
    SkASSERT(GrMipMapsStatus::kNotAllocated != fMipMapsStatus);
    fMipMapsStatus = GrMipMapsStatus::kValid;
}

size_t GrTexture::onGpuMemorySize() const {
    const GrCaps& caps = *this->getGpu()->caps();
    return GrSurface::ComputeSize(caps, this->backendFormat(), this->dimensions(), 1,
                                  this->texturePriv().mipMapped());
}

/////////////////////////////////////////////////////////////////////////////
GrTexture::GrTexture(GrGpu* gpu,
                     const SkISize& dimensions,
                     GrPixelConfig config,
                     GrProtected isProtected,
                     GrTextureType textureType,
                     GrMipMapsStatus mipMapsStatus)
        : INHERITED(gpu, dimensions, config, isProtected)
        , fTextureType(textureType)
        , fMipMapsStatus(mipMapsStatus) {
    if (GrMipMapsStatus::kNotAllocated == fMipMapsStatus) {
        fMaxMipMapLevel = 0;
    } else {
        fMaxMipMapLevel = SkMipMap::ComputeLevelCount(this->width(), this->height());
    }
}

bool GrTexture::StealBackendTexture(sk_sp<GrTexture> texture,
                                    GrBackendTexture* backendTexture,
                                    SkImage::BackendTextureReleaseProc* releaseProc) {
    if (!texture->unique()) {
        return false;
    }

    if (!texture->onStealBackendTexture(backendTexture, releaseProc)) {
        return false;
    }
#ifdef SK_DEBUG
    GrResourceCache* cache = texture->getContext()->priv().getResourceCache();
    int preCount = cache->getResourceCount();
#endif
    // Ensure that the texture will be released by the cache when we drop the last ref.
    // A texture that has no refs and no keys should be immediately removed.
    if (texture->getUniqueKey().isValid()) {
        texture->resourcePriv().removeUniqueKey();
    }
    if (texture->resourcePriv().getScratchKey().isValid()) {
        texture->resourcePriv().removeScratchKey();
    }
#ifdef SK_DEBUG
    texture.reset();
    int postCount = cache->getResourceCount();
    SkASSERT(postCount < preCount);
#endif
    return true;
}

void GrTexture::computeScratchKey(GrScratchKey* key) const {
    if (!this->getGpu()->caps()->isFormatCompressed(this->backendFormat())) {
        int sampleCount = 1;
        GrRenderable renderable = GrRenderable::kNo;
        if (const auto* rt = this->asRenderTarget()) {
            sampleCount = rt->numSamples();
            renderable = GrRenderable::kYes;
        }
        auto isProtected = this->isProtected() ? GrProtected::kYes : GrProtected::kNo;
        GrTexturePriv::ComputeScratchKey(this->config(), this->dimensions(), renderable,
                                         sampleCount, this->texturePriv().mipMapped(), isProtected,
                                         key);
    }
}

void GrTexturePriv::ComputeScratchKey(GrPixelConfig config,
                                      SkISize dimensions,
                                      GrRenderable renderable,
                                      int sampleCnt,
                                      GrMipMapped mipMapped,
                                      GrProtected isProtected,
                                      GrScratchKey* key) {
    static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(sampleCnt > 0);
    SkASSERT(1 == sampleCnt || renderable == GrRenderable::kYes);

    // make sure desc.fConfig fits in 5 bits
    SkASSERT(sk_float_log2(kLast_GrPixelConfig) <= 5);
    SkASSERT(static_cast<uint32_t>(config) < (1 << 5));
    SkASSERT(static_cast<uint32_t>(mipMapped) <= 1);
    SkASSERT(static_cast<uint32_t>(isProtected) <= 1);
    SkASSERT(static_cast<uint32_t>(renderable) <= 1);
    SkASSERT(static_cast<uint32_t>(sampleCnt) < (1 << (32 - 8)));

    GrScratchKey::Builder builder(key, kType, 3);
    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = (static_cast<uint32_t>(config)      << 0)
               | (static_cast<uint32_t>(mipMapped)   << 5)
               | (static_cast<uint32_t>(isProtected) << 6)
               | (static_cast<uint32_t>(renderable)  << 7)
               | (static_cast<uint32_t>(sampleCnt)   << 8);
}
