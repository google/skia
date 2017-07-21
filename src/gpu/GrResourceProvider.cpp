/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrResourceProvider.h"

#include "GrBackendSemaphore.h"
#include "GrBuffer.h"
#include "GrCaps.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGpu.h"
#include "GrPath.h"
#include "GrPathRendering.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceCache.h"
#include "GrResourceKey.h"
#include "GrSemaphore.h"
#include "GrStencilAttachment.h"
#include "GrSurfaceProxyPriv.h"
#include "GrTexturePriv.h"
#include "../private/GrSingleOwner.h"
#include "SkGr.h"
#include "SkMathPriv.h"

GR_DECLARE_STATIC_UNIQUE_KEY(gQuadIndexBufferKey);

const uint32_t GrResourceProvider::kMinScratchTextureSize = 16;

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fSingleOwner);)

GrResourceProvider::GrResourceProvider(GrGpu* gpu, GrResourceCache* cache, GrSingleOwner* owner)
        : fCache(cache)
        , fGpu(gpu)
#ifdef SK_DEBUG
        , fSingleOwner(owner)
#endif
        {
    fCaps = sk_ref_sp(fGpu->caps());

    GR_DEFINE_STATIC_UNIQUE_KEY(gQuadIndexBufferKey);
    fQuadIndexBufferKey = gQuadIndexBufferKey;
}

bool GrResourceProvider::IsFunctionallyExact(GrSurfaceProxy* proxy) {
    return proxy->priv().isExact() || (SkIsPow2(proxy->width()) && SkIsPow2(proxy->height()));
}

bool validate_desc(const GrSurfaceDesc& desc, const GrCaps& caps, int levelCount = 0) {
    if (desc.fWidth <= 0 || desc.fHeight <= 0) {
        return false;
    }
    if (!caps.isConfigTexturable(desc.fConfig)) {
        return false;
    }
    if (desc.fFlags & kRenderTarget_GrSurfaceFlag) {
        if (!caps.isConfigRenderable(desc.fConfig, desc.fSampleCnt > 0)) {
            return false;
        }
    } else {
        if (desc.fSampleCnt) {
            return false;
        }
    }
    if (levelCount > 1 && (GrPixelConfigIsSint(desc.fConfig) || !caps.mipMapSupport())) {
        return false;
    }
    return true;
}

sk_sp<GrTexture> GrResourceProvider::createTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                                   const GrMipLevel texels[], int mipLevelCount,
                                                   SkDestinationSurfaceColorMode mipColorMode) {
    ASSERT_SINGLE_OWNER

    SkASSERT(mipLevelCount >= 1);

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!validate_desc(desc, *fCaps, mipLevelCount)) {
        return nullptr;
    }

    sk_sp<GrTexture> tex(fGpu->createTexture(desc, budgeted, texels, mipLevelCount));
    if (tex) {
        tex->texturePriv().setMipColorMode(mipColorMode);
    }

    return tex;
}

sk_sp<GrTexture> GrResourceProvider::getExactScratch(const GrSurfaceDesc& desc,
                                                     SkBudgeted budgeted, uint32_t flags) {
    flags |= kExact_Flag | kNoCreate_Flag;
    sk_sp<GrTexture> tex(this->refScratchTexture(desc, flags));
    if (tex && SkBudgeted::kNo == budgeted) {
        tex->resourcePriv().makeUnbudgeted();
    }

    return tex;
}

static bool make_info(int w, int h, GrPixelConfig config, SkImageInfo* ii) {
    SkColorType colorType;
    if (!GrPixelConfigToColorType(config, &colorType)) {
        return false;
    }

    *ii = SkImageInfo::Make(w, h, colorType, kUnknown_SkAlphaType, nullptr);
    return true;
}

sk_sp<GrTextureProxy> GrResourceProvider::createTextureProxy(const GrSurfaceDesc& desc,
                                                             SkBudgeted budgeted,
                                                             const GrMipLevel& mipLevel) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!mipLevel.fPixels) {
        return nullptr;
    }

    if (!validate_desc(desc, *fCaps)) {
        return nullptr;
    }

    GrContext* context = fGpu->getContext();

    SkImageInfo srcInfo;

    if (make_info(desc.fWidth, desc.fHeight, desc.fConfig, &srcInfo)) {
        sk_sp<GrTexture> tex = this->getExactScratch(desc, budgeted, 0);
        sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeWrapped(std::move(tex));
        if (proxy) {
            sk_sp<GrSurfaceContext> sContext =
                       context->contextPriv().makeWrappedSurfaceContext(std::move(proxy), nullptr);
            if (sContext) {
                if (sContext->writePixels(srcInfo, mipLevel.fPixels, mipLevel.fRowBytes, 0, 0)) {
                    return sContext->asTextureProxyRef();
                }
            }
        }
    }

    sk_sp<GrTexture> tex(fGpu->createTexture(desc, budgeted, &mipLevel, 1));
    return GrSurfaceProxy::MakeWrapped(std::move(tex));
}

sk_sp<GrTexture> GrResourceProvider::createTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                                   uint32_t flags) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!validate_desc(desc, *fCaps)) {
        return nullptr;
    }

    sk_sp<GrTexture> tex = this->getExactScratch(desc, budgeted, flags);
    if (tex) {
        return tex;
    }

    return fGpu->createTexture(desc, budgeted);
}

sk_sp<GrTexture> GrResourceProvider::createApproxTexture(const GrSurfaceDesc& desc,
                                                         uint32_t flags) {
    ASSERT_SINGLE_OWNER
    SkASSERT(0 == flags || kNoPendingIO_Flag == flags);

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!validate_desc(desc, *fCaps)) {
        return nullptr;
    }

    return this->refScratchTexture(desc, flags);
}

sk_sp<GrTexture> GrResourceProvider::refScratchTexture(const GrSurfaceDesc& inDesc,
                                                       uint32_t flags) {
    ASSERT_SINGLE_OWNER
    SkASSERT(!this->isAbandoned());
    SkASSERT(validate_desc(inDesc, *fCaps));

    SkTCopyOnFirstWrite<GrSurfaceDesc> desc(inDesc);

    // We could make initial clears work with scratch textures but it is a rare case so we just opt
    // to fall back to making a new texture.
    if (!SkToBool(inDesc.fFlags & kPerformInitialClear_GrSurfaceFlag) &&
        (fGpu->caps()->reuseScratchTextures() || (desc->fFlags & kRenderTarget_GrSurfaceFlag))) {
        if (!(kExact_Flag & flags)) {
            // bin by pow2 with a reasonable min
            GrSurfaceDesc* wdesc = desc.writable();
            wdesc->fWidth  = SkTMax(kMinScratchTextureSize, GrNextPow2(desc->fWidth));
            wdesc->fHeight = SkTMax(kMinScratchTextureSize, GrNextPow2(desc->fHeight));
        }

        GrScratchKey key;
        GrTexturePriv::ComputeScratchKey(*desc, &key);
        uint32_t scratchFlags = 0;
        if (kNoPendingIO_Flag & flags) {
            scratchFlags = GrResourceCache::kRequireNoPendingIO_ScratchFlag;
        } else  if (!(desc->fFlags & kRenderTarget_GrSurfaceFlag)) {
            // If it is not a render target then it will most likely be populated by
            // writePixels() which will trigger a flush if the texture has pending IO.
            scratchFlags = GrResourceCache::kPreferNoPendingIO_ScratchFlag;
        }
        GrGpuResource* resource = fCache->findAndRefScratchResource(key,
                                                                   GrSurface::WorstCaseSize(*desc),
                                                                   scratchFlags);
        if (resource) {
            GrSurface* surface = static_cast<GrSurface*>(resource);
            return sk_sp<GrTexture>(surface->asTexture());
        }
    }

    if (!(kNoCreate_Flag & flags)) {
        return fGpu->createTexture(*desc, SkBudgeted::kYes);
    }

    return nullptr;
}

sk_sp<GrTexture> GrResourceProvider::wrapBackendTexture(const GrBackendTexture& tex,
                                                        GrSurfaceOrigin origin,
                                                        GrWrapOwnership ownership) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->wrapBackendTexture(tex, origin, ownership);
}

sk_sp<GrTexture> GrResourceProvider::wrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                                  GrSurfaceOrigin origin,
                                                                  int sampleCnt,
                                                                  GrWrapOwnership ownership) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->wrapRenderableBackendTexture(tex, origin, sampleCnt, ownership);
}

sk_sp<GrRenderTarget> GrResourceProvider::wrapBackendRenderTarget(
        const GrBackendRenderTarget& backendRT, GrSurfaceOrigin origin)
{
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr : fGpu->wrapBackendRenderTarget(backendRT, origin);
}

void GrResourceProvider::assignUniqueKeyToResource(const GrUniqueKey& key,
                                                   GrGpuResource* resource) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned() || !resource) {
        return;
    }
    resource->resourcePriv().setUniqueKey(key);
}

GrGpuResource* GrResourceProvider::findAndRefResourceByUniqueKey(const GrUniqueKey& key) {
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr : fCache->findAndRefUniqueResource(key);
}

GrTexture* GrResourceProvider::findAndRefTextureByUniqueKey(const GrUniqueKey& key) {
    ASSERT_SINGLE_OWNER
    GrGpuResource* resource = this->findAndRefResourceByUniqueKey(key);
    if (resource) {
        GrTexture* texture = static_cast<GrSurface*>(resource)->asTexture();
        SkASSERT(texture);
        return texture;
    }
    return NULL;
}

void GrResourceProvider::assignUniqueKeyToTexture(const GrUniqueKey& key, GrTexture* texture) {
    SkASSERT(key.isValid());
    this->assignUniqueKeyToResource(key, texture);
}

// MDB TODO (caching): this side-steps the issue of texture proxies with unique IDs
void GrResourceProvider::assignUniqueKeyToProxy(const GrUniqueKey& key, GrTextureProxy* proxy) {
    ASSERT_SINGLE_OWNER
    SkASSERT(key.isValid());
    if (this->isAbandoned() || !proxy) {
        return;
    }

    if (!proxy->instantiate(this)) {
        return;
    }
    GrTexture* texture = proxy->priv().peekTexture();

    this->assignUniqueKeyToResource(key, texture);
}

// MDB TODO (caching): this side-steps the issue of texture proxies with unique IDs
sk_sp<GrTextureProxy> GrResourceProvider::findProxyByUniqueKey(const GrUniqueKey& key) {
    ASSERT_SINGLE_OWNER

    sk_sp<GrTexture> texture(this->findAndRefTextureByUniqueKey(key));
    if (!texture) {
        return nullptr;
    }

    return GrSurfaceProxy::MakeWrapped(std::move(texture));
}

const GrBuffer* GrResourceProvider::createPatternedIndexBuffer(const uint16_t* pattern,
                                                               int patternSize,
                                                               int reps,
                                                               int vertCount,
                                                               const GrUniqueKey& key) {
    size_t bufferSize = patternSize * reps * sizeof(uint16_t);

    // This is typically used in GrMeshDrawOps, so we assume kNoPendingIO.
    GrBuffer* buffer = this->createBuffer(bufferSize, kIndex_GrBufferType, kStatic_GrAccessPattern,
                                          kNoPendingIO_Flag);
    if (!buffer) {
        return nullptr;
    }
    uint16_t* data = (uint16_t*) buffer->map();
    bool useTempData = (nullptr == data);
    if (useTempData) {
        data = new uint16_t[reps * patternSize];
    }
    for (int i = 0; i < reps; ++i) {
        int baseIdx = i * patternSize;
        uint16_t baseVert = (uint16_t)(i * vertCount);
        for (int j = 0; j < patternSize; ++j) {
            data[baseIdx+j] = baseVert + pattern[j];
        }
    }
    if (useTempData) {
        if (!buffer->updateData(data, bufferSize)) {
            buffer->unref();
            return nullptr;
        }
        delete[] data;
    } else {
        buffer->unmap();
    }
    this->assignUniqueKeyToResource(key, buffer);
    return buffer;
}

const GrBuffer* GrResourceProvider::createQuadIndexBuffer() {
    static const int kMaxQuads = 1 << 12; // max possible: (1 << 14) - 1;
    GR_STATIC_ASSERT(4 * kMaxQuads <= 65535);
    static const uint16_t kPattern[] = { 0, 1, 2, 0, 2, 3 };

    return this->createPatternedIndexBuffer(kPattern, 6, kMaxQuads, 4, fQuadIndexBufferKey);
}

sk_sp<GrPath> GrResourceProvider::createPath(const SkPath& path, const GrStyle& style) {
    SkASSERT(this->gpu()->pathRendering());
    return this->gpu()->pathRendering()->createPath(path, style);
}

sk_sp<GrPathRange> GrResourceProvider::createPathRange(GrPathRange::PathGenerator* gen,
                                                       const GrStyle& style) {
    SkASSERT(this->gpu()->pathRendering());
    return this->gpu()->pathRendering()->createPathRange(gen, style);
}

sk_sp<GrPathRange> GrResourceProvider::createGlyphs(const SkTypeface* tf,
                                                    const SkScalerContextEffects& effects,
                                                    const SkDescriptor* desc,
                                                    const GrStyle& style) {

    SkASSERT(this->gpu()->pathRendering());
    return this->gpu()->pathRendering()->createGlyphs(tf, effects, desc, style);
}

GrBuffer* GrResourceProvider::createBuffer(size_t size, GrBufferType intendedType,
                                           GrAccessPattern accessPattern, uint32_t flags,
                                           const void* data) {
    if (this->isAbandoned()) {
        return nullptr;
    }
    if (kDynamic_GrAccessPattern != accessPattern) {
        return this->gpu()->createBuffer(size, intendedType, accessPattern, data);
    }
    if (!(flags & kRequireGpuMemory_Flag) &&
        this->gpu()->caps()->preferClientSideDynamicBuffers() &&
        GrBufferTypeIsVertexOrIndex(intendedType) &&
        kDynamic_GrAccessPattern == accessPattern) {
        return GrBuffer::CreateCPUBacked(this->gpu(), size, intendedType, data);
    }

    // bin by pow2 with a reasonable min
    static const size_t MIN_SIZE = 1 << 12;
    size_t allocSize = SkTMax(MIN_SIZE, GrNextSizePow2(size));

    GrScratchKey key;
    GrBuffer::ComputeScratchKeyForDynamicVBO(allocSize, intendedType, &key);
    uint32_t scratchFlags = 0;
    if (flags & kNoPendingIO_Flag) {
        scratchFlags = GrResourceCache::kRequireNoPendingIO_ScratchFlag;
    } else {
        scratchFlags = GrResourceCache::kPreferNoPendingIO_ScratchFlag;
    }
    GrBuffer* buffer = static_cast<GrBuffer*>(
        this->cache()->findAndRefScratchResource(key, allocSize, scratchFlags));
    if (!buffer) {
        buffer = this->gpu()->createBuffer(allocSize, intendedType, kDynamic_GrAccessPattern);
        if (!buffer) {
            return nullptr;
        }
    }
    if (data) {
        buffer->updateData(data, size);
    }
    SkASSERT(!buffer->isCPUBacked()); // We should only cache real VBOs.
    return buffer;
}

GrStencilAttachment* GrResourceProvider::attachStencilAttachment(GrRenderTarget* rt) {
    SkASSERT(rt);
    if (rt->renderTargetPriv().getStencilAttachment()) {
        return rt->renderTargetPriv().getStencilAttachment();
    }

    if (!rt->wasDestroyed() && rt->canAttemptStencilAttachment()) {
        GrUniqueKey sbKey;

        int width = rt->width();
        int height = rt->height();
#if 0
        if (this->caps()->oversizedStencilSupport()) {
            width  = SkNextPow2(width);
            height = SkNextPow2(height);
        }
#endif
        bool newStencil = false;
        GrStencilAttachment::ComputeSharedStencilAttachmentKey(width, height,
                                                               rt->numStencilSamples(), &sbKey);
        GrStencilAttachment* stencil = static_cast<GrStencilAttachment*>(
            this->findAndRefResourceByUniqueKey(sbKey));
        if (!stencil) {
            // Need to try and create a new stencil
            stencil = this->gpu()->createStencilAttachmentForRenderTarget(rt, width, height);
            if (stencil) {
                this->assignUniqueKeyToResource(sbKey, stencil);
                newStencil = true;
            }
        }
        if (rt->renderTargetPriv().attachStencilAttachment(stencil)) {
            if (newStencil) {
                // Right now we're clearing the stencil attachment here after it is
                // attached to a RT for the first time. When we start matching
                // stencil buffers with smaller color targets this will no longer
                // be correct because it won't be guaranteed to clear the entire
                // sb.
                // We used to clear down in the GL subclass using a special purpose
                // FBO. But iOS doesn't allow a stencil-only FBO. It reports unsupported
                // FBO status.
                this->gpu()->clearStencil(rt);
            }
        }
    }
    return rt->renderTargetPriv().getStencilAttachment();
}

sk_sp<GrRenderTarget> GrResourceProvider::wrapBackendTextureAsRenderTarget(
        const GrBackendTexture& tex, GrSurfaceOrigin origin, int sampleCnt)
{
    if (this->isAbandoned()) {
        return nullptr;
    }
    return this->gpu()->wrapBackendTextureAsRenderTarget(tex, origin, sampleCnt);
}

sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT GrResourceProvider::makeSemaphore(bool isOwned) {
    return fGpu->makeSemaphore(isOwned);
}

sk_sp<GrSemaphore> GrResourceProvider::wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                                            GrWrapOwnership ownership) {
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr : fGpu->wrapBackendSemaphore(semaphore, ownership);
}

void GrResourceProvider::takeOwnershipOfSemaphore(sk_sp<GrSemaphore> semaphore) {
    semaphore->resetGpu(fGpu);
}

void GrResourceProvider::releaseOwnershipOfSemaphore(sk_sp<GrSemaphore> semaphore) {
    semaphore->resetGpu(nullptr);
}
