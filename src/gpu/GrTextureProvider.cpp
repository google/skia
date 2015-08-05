
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrTextureProvider.h"
#include "GrTexturePriv.h"
#include "GrResourceCache.h"
#include "GrGpu.h"

enum ScratchTextureFlags {
    kExact_ScratchTextureFlag           = 0x1,
    kNoPendingIO_ScratchTextureFlag     = 0x2,
    kNoCreate_ScratchTextureFlag        = 0x4,
};

GrTexture* GrTextureProvider::createTexture(const GrSurfaceDesc& desc, bool budgeted,
                                            const void* srcData, size_t rowBytes) {
    if (this->isAbandoned()) {
        return NULL;
    }
    if ((desc.fFlags & kRenderTarget_GrSurfaceFlag) &&
        !fGpu->caps()->isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
        return NULL;
    }
    if (!GrPixelConfigIsCompressed(desc.fConfig)) {
        static const uint32_t kFlags = kExact_ScratchTextureFlag |
                                       kNoCreate_ScratchTextureFlag;
        if (GrTexture* texture = this->refScratchTexture(desc, kFlags)) {
            if (!srcData || texture->writePixels(0, 0, desc.fWidth, desc.fHeight, desc.fConfig,
                                                 srcData, rowBytes)) {
                if (!budgeted) {
                    texture->resourcePriv().makeUnbudgeted();
                }
                return texture;
            }
            texture->unref();
        }
    }
    return fGpu->createTexture(desc, budgeted, srcData, rowBytes);
}

GrTexture* GrTextureProvider::createApproxTexture(const GrSurfaceDesc& desc) {
    return this->internalCreateApproxTexture(desc, 0);
}

GrTexture* GrTextureProvider::internalCreateApproxTexture(const GrSurfaceDesc& desc,
                                                          uint32_t scratchFlags) {
    if (this->isAbandoned()) {
        return NULL;
    }
    // Currently we don't recycle compressed textures as scratch.
    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        return NULL;
    } else {
        return this->refScratchTexture(desc, scratchFlags);
    }
}

GrTexture* GrTextureProvider::refScratchTexture(const GrSurfaceDesc& inDesc,
                                                uint32_t flags) {
    SkASSERT(!this->isAbandoned());
    SkASSERT(!GrPixelConfigIsCompressed(inDesc.fConfig));

    SkTCopyOnFirstWrite<GrSurfaceDesc> desc(inDesc);

    if (fGpu->caps()->reuseScratchTextures() || (desc->fFlags & kRenderTarget_GrSurfaceFlag)) {
        if (!(kExact_ScratchTextureFlag & flags)) {
            // bin by pow2 with a reasonable min
            const int minSize = SkTMin(16, fGpu->caps()->minTextureSize());
            GrSurfaceDesc* wdesc = desc.writable();
            wdesc->fWidth  = SkTMax(minSize, GrNextPow2(desc->fWidth));
            wdesc->fHeight = SkTMax(minSize, GrNextPow2(desc->fHeight));
        }

        GrScratchKey key;
        GrTexturePriv::ComputeScratchKey(*desc, &key);
        uint32_t scratchFlags = 0;
        if (kNoPendingIO_ScratchTextureFlag & flags) {
            scratchFlags = GrResourceCache::kRequireNoPendingIO_ScratchFlag;
        } else  if (!(desc->fFlags & kRenderTarget_GrSurfaceFlag)) {
            // If it is not a render target then it will most likely be populated by
            // writePixels() which will trigger a flush if the texture has pending IO.
            scratchFlags = GrResourceCache::kPreferNoPendingIO_ScratchFlag;
        }
        GrGpuResource* resource = fCache->findAndRefScratchResource(key, scratchFlags);
        if (resource) {
            GrSurface* surface = static_cast<GrSurface*>(resource);
            GrRenderTarget* rt = surface->asRenderTarget();
            if (rt && fGpu->caps()->discardRenderTargetSupport()) {
                rt->discard();
            }
            return surface->asTexture();
        }
    }

    if (!(kNoCreate_ScratchTextureFlag & flags)) {
        return fGpu->createTexture(*desc, true, NULL, 0);
    }

    return NULL;
}

GrTexture* GrTextureProvider::wrapBackendTexture(const GrBackendTextureDesc& desc,
                                                 GrWrapOwnership ownership) {
    if (this->isAbandoned()) {
        return NULL;
    }
    return fGpu->wrapBackendTexture(desc, ownership);
}

GrRenderTarget* GrTextureProvider::wrapBackendRenderTarget(const GrBackendRenderTargetDesc& desc) {
    return this->isAbandoned() ? NULL : fGpu->wrapBackendRenderTarget(desc,
                                                                      kBorrow_GrWrapOwnership);
}

void GrTextureProvider::assignUniqueKeyToResource(const GrUniqueKey& key, GrGpuResource* resource) {
    if (this->isAbandoned() || !resource) {
        return;
    }
    resource->resourcePriv().setUniqueKey(key);
}

bool GrTextureProvider::existsResourceWithUniqueKey(const GrUniqueKey& key) const {
    return this->isAbandoned() ? false : fCache->hasUniqueKey(key);
}

GrGpuResource* GrTextureProvider::findAndRefResourceByUniqueKey(const GrUniqueKey& key) {
    return this->isAbandoned() ? NULL : fCache->findAndRefUniqueResource(key);
}
