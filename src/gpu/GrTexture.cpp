/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkMath.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrTypes.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrTexture.h"

#ifdef SK_DEBUG
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#endif

void GrTexture::markMipmapsDirty() {
    if (GrMipmapStatus::kValid == fMipmapStatus) {
        fMipmapStatus = GrMipmapStatus::kDirty;
    }
}

void GrTexture::markMipmapsClean() {
    SkASSERT(GrMipmapStatus::kNotAllocated != fMipmapStatus);
    fMipmapStatus = GrMipmapStatus::kValid;
}

size_t GrTexture::onGpuMemorySize() const {
    return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                  /*colorSamplesPerPixel=*/1, this->mipmapped());
}

/////////////////////////////////////////////////////////////////////////////
GrTexture::GrTexture(GrGpu* gpu,
                     const SkISize& dimensions,
                     GrProtected isProtected,
                     GrTextureType textureType,
                     GrMipmapStatus mipmapStatus)
        : INHERITED(gpu, dimensions, isProtected)
        , fTextureType(textureType)
        , fMipmapStatus(mipmapStatus) {
    if (fMipmapStatus == GrMipmapStatus::kNotAllocated) {
        fMaxMipmapLevel = 0;
    } else {
        fMaxMipmapLevel = SkMipmap::ComputeLevelCount(this->width(), this->height());
    }
    if (textureType == GrTextureType::kExternal) {
        this->setReadOnly();
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

void GrTexture::computeScratchKey(skgpu::ScratchKey* key) const {
    if (!this->getGpu()->caps()->isFormatCompressed(this->backendFormat())) {
        int sampleCount = 1;
        GrRenderable renderable = GrRenderable::kNo;
        if (const auto* rt = this->asRenderTarget()) {
            sampleCount = rt->numSamples();
            renderable = GrRenderable::kYes;
        }
        auto isProtected = this->isProtected() ? GrProtected::kYes : GrProtected::kNo;
        ComputeScratchKey(*this->getGpu()->caps(), this->backendFormat(), this->dimensions(),
                          renderable, sampleCount, this->mipmapped(), isProtected, key);
    }
}

void GrTexture::ComputeScratchKey(const GrCaps& caps,
                                  const GrBackendFormat& format,
                                  SkISize dimensions,
                                  GrRenderable renderable,
                                  int sampleCnt,
                                  GrMipmapped mipMapped,
                                  GrProtected isProtected,
                                  skgpu::ScratchKey* key) {
    static const skgpu::ScratchKey::ResourceType kType = skgpu::ScratchKey::GenerateResourceType();
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(sampleCnt > 0);
    SkASSERT(1 == sampleCnt || renderable == GrRenderable::kYes);

    SkASSERT(static_cast<uint32_t>(mipMapped) <= 1);
    SkASSERT(static_cast<uint32_t>(isProtected) <= 1);
    SkASSERT(static_cast<uint32_t>(renderable) <= 1);
    SkASSERT(static_cast<uint32_t>(sampleCnt) < (1 << (32 - 3)));

    uint64_t formatKey = caps.computeFormatKey(format);

    skgpu::ScratchKey::Builder builder(key, kType, 5);
    builder[0] = dimensions.width();
    builder[1] = dimensions.height();
    builder[2] = formatKey & 0xFFFFFFFF;
    builder[3] = (formatKey >> 32) & 0xFFFFFFFF;
    builder[4] = (static_cast<uint32_t>(mipMapped)   << 0)
               | (static_cast<uint32_t>(isProtected) << 1)
               | (static_cast<uint32_t>(renderable)  << 2)
               | (static_cast<uint32_t>(sampleCnt)   << 3);
}
