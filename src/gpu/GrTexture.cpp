/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMath.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrRenderTarget.h"
#include "include/gpu/GrTexture.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrResourceKey.h"
#include "src/core/SkMipMap.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
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
    return GrSurface::ComputeSize(this->config(), this->width(), this->height(), 1,
                                  this->texturePriv().mipMapped());
}

/////////////////////////////////////////////////////////////////////////////
GrTexture::GrTexture(GrGpu* gpu, const GrSurfaceDesc& desc, GrProtected isProtected,
                     GrTextureType textureType, GrMipMapsStatus mipMapsStatus)
        : INHERITED(gpu, desc, isProtected)
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
    if (!texture->surfacePriv().hasUniqueRef() || texture->surfacePriv().hasPendingIO()) {
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
    if (!GrPixelConfigIsCompressed(this->config())) {
        int sampleCount = 1;
        GrRenderable renderable = GrRenderable::kNo;
        if (const auto* rt = this->asRenderTarget()) {
            sampleCount = rt->numSamples();
            renderable = GrRenderable::kYes;
        }
        GrTexturePriv::ComputeScratchKey(this->config(), this->width(), this->height(), renderable,
                                         sampleCount, this->texturePriv().mipMapped(), key);
    }
}

void GrTexturePriv::ComputeScratchKey(GrPixelConfig config, int width, int height,
                                      GrRenderable renderable, int sampleCnt, GrMipMapped mipMapped,
                                      GrScratchKey* key) {
    static const GrScratchKey::ResourceType kType = GrScratchKey::GenerateResourceType();
    SkASSERT(width > 0);
    SkASSERT(height > 0);
    SkASSERT(sampleCnt > 0);
    SkASSERT(1 == sampleCnt || renderable == GrRenderable::kYes);

    // make sure desc.fConfig fits in 5 bits
    SkASSERT(sk_float_log2(kLast_GrPixelConfig) <= 5);
    SkASSERT(static_cast<int>(config) < (1 << 5));
    SkASSERT(sampleCnt < (1 << 8));
    SkASSERT(static_cast<int>(mipMapped) <= 1);

    GrScratchKey::Builder builder(key, kType, 3);
    builder[0] = width;
    builder[1] = height;
    builder[2] = config
               | (static_cast<uint32_t>(mipMapped) << 5)
               | (sampleCnt << 6)
               | (static_cast<uint32_t>(renderable) << 14);
}

void GrTexturePriv::ComputeScratchKey(const GrSurfaceDesc& desc, GrRenderable renderable,
                                      int sampleCnt, GrScratchKey* key) {
    // Note: the fOrigin field is not used in the scratch key
    return ComputeScratchKey(desc.fConfig, desc.fWidth, desc.fHeight, renderable, sampleCnt,
                             GrMipMapped::kNo, key);
}
