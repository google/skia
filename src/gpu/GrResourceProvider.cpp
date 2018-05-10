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
#include "GrProxyProvider.h"
#include "GrRenderTargetPriv.h"
#include "GrResourceCache.h"
#include "GrResourceKey.h"
#include "GrSemaphore.h"
#include "GrStencilAttachment.h"
#include "GrTexturePriv.h"
#include "../private/GrSingleOwner.h"
#include "SkGr.h"
#include "SkMathPriv.h"

GR_DECLARE_STATIC_UNIQUE_KEY(gQuadIndexBufferKey);

const uint32_t GrResourceProvider::kMinScratchTextureSize = 16;

#ifdef SK_DISABLE_EXPLICIT_GPU_RESOURCE_ALLOCATION
static const bool kDefaultExplicitlyAllocateGPUResources = false;
#else
static const bool kDefaultExplicitlyAllocateGPUResources = true;
#endif

#define ASSERT_SINGLE_OWNER \
    SkDEBUGCODE(GrSingleOwner::AutoEnforce debug_SingleOwner(fSingleOwner);)

GrResourceProvider::GrResourceProvider(GrGpu* gpu, GrResourceCache* cache, GrSingleOwner* owner,
                                       GrContextOptions::Enable explicitlyAllocateGPUResources)
        : fCache(cache)
        , fGpu(gpu)
#ifdef SK_DEBUG
        , fSingleOwner(owner)
#endif
{
    if (GrContextOptions::Enable::kNo == explicitlyAllocateGPUResources) {
        fExplicitlyAllocateGPUResources = false;
    } else if (GrContextOptions::Enable::kYes == explicitlyAllocateGPUResources) {
        fExplicitlyAllocateGPUResources = true;
    } else {
        fExplicitlyAllocateGPUResources = kDefaultExplicitlyAllocateGPUResources;
    }

    fCaps = sk_ref_sp(fGpu->caps());

    GR_DEFINE_STATIC_UNIQUE_KEY(gQuadIndexBufferKey);
    fQuadIndexBufferKey = gQuadIndexBufferKey;
}

sk_sp<GrTexture> GrResourceProvider::createTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                                   const GrMipLevel texels[], int mipLevelCount,
                                                   SkDestinationSurfaceColorMode mipColorMode) {
    ASSERT_SINGLE_OWNER

    SkASSERT(mipLevelCount > 0);

    if (this->isAbandoned()) {
        return nullptr;
    }

    GrMipMapped mipMapped = mipLevelCount > 1 ? GrMipMapped::kYes : GrMipMapped::kNo;
    if (!fCaps->validateSurfaceDesc(desc, mipMapped)) {
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
    sk_sp<GrTexture> tex(this->refScratchTexture(desc, flags));
    if (tex && SkBudgeted::kNo == budgeted) {
        tex->resourcePriv().makeUnbudgeted();
    }

    return tex;
}

sk_sp<GrTexture> GrResourceProvider::createTexture(const GrSurfaceDesc& desc,
                                                   SkBudgeted budgeted,
                                                   SkBackingFit fit,
                                                   const GrMipLevel& mipLevel) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!mipLevel.fPixels) {
        return nullptr;
    }

    if (!fCaps->validateSurfaceDesc(desc, GrMipMapped::kNo)) {
        return nullptr;
    }

    GrContext* context = fGpu->getContext();
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();

    SkColorType colorType;
    if (GrPixelConfigToColorType(desc.fConfig, &colorType)) {
        // DDL TODO: remove this use of createInstantiatedProxy and convert it to a testing-only
        // method.
        sk_sp<GrTextureProxy> proxy = proxyProvider->createInstantiatedProxy(desc,
                                                                             fit,
                                                                             budgeted);
        if (!proxy) {
            return nullptr;
        }
        // We use an ephemeral surface context to do the write pixels. Here it isn't clear what
        // color space to tag it with. That's ok because GrSurfaceContext::writePixels doesn't
        // do any color space conversions. Though, that is likely to change. However, if the
        // pixel config is sRGB then the passed color space here must have sRGB gamma or
        // GrSurfaceContext creation fails.
        sk_sp<SkColorSpace> colorSpace;
        if (GrPixelConfigIsSRGB(desc.fConfig)) {
            colorSpace = SkColorSpace::MakeSRGB();
        }
        auto srcInfo = SkImageInfo::Make(desc.fWidth, desc.fHeight, colorType,
                                         kUnknown_SkAlphaType, colorSpace);
        sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeWrappedSurfaceContext(
                std::move(proxy), std::move(colorSpace));
        if (!sContext) {
            return nullptr;
        }
        SkAssertResult(sContext->writePixels(srcInfo, mipLevel.fPixels, mipLevel.fRowBytes, 0, 0));
        return sk_ref_sp(sContext->asTextureProxy()->priv().peekTexture());
    } else {
        return fGpu->createTexture(desc, budgeted, &mipLevel, 1);
    }
}

sk_sp<GrTexture> GrResourceProvider::createTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                                   uint32_t flags) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!fCaps->validateSurfaceDesc(desc, GrMipMapped::kNo)) {
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

    if (!fCaps->validateSurfaceDesc(desc, GrMipMapped::kNo)) {
        return nullptr;
    }

    if (auto tex = this->refScratchTexture(desc, flags)) {
        return tex;
    }

    SkTCopyOnFirstWrite<GrSurfaceDesc> copyDesc(desc);

    // bin by pow2 with a reasonable min
    if (!SkToBool(desc.fFlags & kPerformInitialClear_GrSurfaceFlag) &&
        (fGpu->caps()->reuseScratchTextures() || (desc.fFlags & kRenderTarget_GrSurfaceFlag))) {
        GrSurfaceDesc* wdesc = copyDesc.writable();
        wdesc->fWidth  = SkTMax(kMinScratchTextureSize, GrNextPow2(desc.fWidth));
        wdesc->fHeight = SkTMax(kMinScratchTextureSize, GrNextPow2(desc.fHeight));
    }

    if (auto tex = this->refScratchTexture(*copyDesc, flags)) {
        return tex;
    }

    return fGpu->createTexture(*copyDesc, SkBudgeted::kYes);
}

sk_sp<GrTexture> GrResourceProvider::refScratchTexture(const GrSurfaceDesc& desc,
                                                       uint32_t flags) {
    ASSERT_SINGLE_OWNER
    SkASSERT(!this->isAbandoned());
    SkASSERT(fCaps->validateSurfaceDesc(desc, GrMipMapped::kNo));

    // We could make initial clears work with scratch textures but it is a rare case so we just opt
    // to fall back to making a new texture.
    if (!SkToBool(desc.fFlags & kPerformInitialClear_GrSurfaceFlag) &&
        (fGpu->caps()->reuseScratchTextures() || (desc.fFlags & kRenderTarget_GrSurfaceFlag))) {

        GrScratchKey key;
        GrTexturePriv::ComputeScratchKey(desc, &key);
        uint32_t scratchFlags = 0;
        if (kNoPendingIO_Flag & flags) {
            scratchFlags = GrResourceCache::kRequireNoPendingIO_ScratchFlag;
        } else  if (!(desc.fFlags & kRenderTarget_GrSurfaceFlag)) {
            // If it is not a render target then it will most likely be populated by
            // writePixels() which will trigger a flush if the texture has pending IO.
            scratchFlags = GrResourceCache::kPreferNoPendingIO_ScratchFlag;
        }
        GrGpuResource* resource = fCache->findAndRefScratchResource(key,
                                                                    GrSurface::WorstCaseSize(desc),
                                                                    scratchFlags);
        if (resource) {
            GrSurface* surface = static_cast<GrSurface*>(resource);
            return sk_sp<GrTexture>(surface->asTexture());
        }
    }

    return nullptr;
}

sk_sp<GrTexture> GrResourceProvider::wrapBackendTexture(const GrBackendTexture& tex,
                                                        GrWrapOwnership ownership) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->wrapBackendTexture(tex, ownership);
}

sk_sp<GrTexture> GrResourceProvider::wrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                                  int sampleCnt,
                                                                  GrWrapOwnership ownership) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->wrapRenderableBackendTexture(tex, sampleCnt, ownership);
}

sk_sp<GrRenderTarget> GrResourceProvider::wrapBackendRenderTarget(
        const GrBackendRenderTarget& backendRT)
{
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr : fGpu->wrapBackendRenderTarget(backendRT);
}

void GrResourceProvider::assignUniqueKeyToResource(const GrUniqueKey& key,
                                                   GrGpuResource* resource) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned() || !resource) {
        return;
    }
    resource->resourcePriv().setUniqueKey(key);
}

sk_sp<GrGpuResource> GrResourceProvider::findResourceByUniqueKey(const GrUniqueKey& key) {
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr
                               : sk_sp<GrGpuResource>(fCache->findAndRefUniqueResource(key));
}

sk_sp<const GrBuffer> GrResourceProvider::findOrMakeStaticBuffer(GrBufferType intendedType,
                                                                 size_t size,
                                                                 const void* data,
                                                                 const GrUniqueKey& key) {
    if (auto buffer = this->findByUniqueKey<GrBuffer>(key)) {
        return buffer;
    }
    if (auto buffer = this->createBuffer(size, intendedType, kStatic_GrAccessPattern, 0,
                                         data)) {
        // We shouldn't bin and/or cachestatic buffers.
        SkASSERT(buffer->sizeInBytes() == size);
        SkASSERT(!buffer->resourcePriv().getScratchKey().isValid());
        SkASSERT(!buffer->resourcePriv().hasPendingIO_debugOnly());
        buffer->resourcePriv().setUniqueKey(key);
        return sk_sp<const GrBuffer>(buffer);
    }
    return nullptr;
}

sk_sp<const GrBuffer> GrResourceProvider::createPatternedIndexBuffer(const uint16_t* pattern,
                                                                     int patternSize,
                                                                     int reps,
                                                                     int vertCount,
                                                                     const GrUniqueKey& key) {
    size_t bufferSize = patternSize * reps * sizeof(uint16_t);

    // This is typically used in GrMeshDrawOps, so we assume kNoPendingIO.
    sk_sp<GrBuffer> buffer(this->createBuffer(bufferSize, kIndex_GrBufferType,
                                              kStatic_GrAccessPattern, kNoPendingIO_Flag));
    if (!buffer) {
        return nullptr;
    }
    uint16_t* data = (uint16_t*) buffer->map();
    SkAutoTArray<uint16_t> temp;
    if (!data) {
        temp.reset(reps * patternSize);
        data = temp.get();
    }
    for (int i = 0; i < reps; ++i) {
        int baseIdx = i * patternSize;
        uint16_t baseVert = (uint16_t)(i * vertCount);
        for (int j = 0; j < patternSize; ++j) {
            data[baseIdx+j] = baseVert + pattern[j];
        }
    }
    if (temp.get()) {
        if (!buffer->updateData(data, bufferSize)) {
            return nullptr;
        }
    } else {
        buffer->unmap();
    }
    this->assignUniqueKeyToResource(key, buffer.get());
    return std::move(buffer);
}

static constexpr int kMaxQuads = 1 << 12;  // max possible: (1 << 14) - 1;

sk_sp<const GrBuffer> GrResourceProvider::createQuadIndexBuffer() {
    GR_STATIC_ASSERT(4 * kMaxQuads <= 65535);
    static const uint16_t kPattern[] = { 0, 1, 2, 2, 1, 3 };
    return this->createPatternedIndexBuffer(kPattern, 6, kMaxQuads, 4, fQuadIndexBufferKey);
}

int GrResourceProvider::QuadCountOfQuadBuffer() { return kMaxQuads; }

sk_sp<GrPath> GrResourceProvider::createPath(const SkPath& path, const GrStyle& style) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    SkASSERT(this->gpu()->pathRendering());
    return this->gpu()->pathRendering()->createPath(path, style);
}

sk_sp<GrPathRange> GrResourceProvider::createPathRange(GrPathRange::PathGenerator* gen,
                                                       const GrStyle& style) {
    if (this->isAbandoned()) {
        return nullptr;
    }

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

bool GrResourceProvider::attachStencilAttachment(GrRenderTarget* rt) {
    SkASSERT(rt);
    if (rt->renderTargetPriv().getStencilAttachment()) {
        return true;
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
        SkDEBUGCODE(bool newStencil = false;)
        GrStencilAttachment::ComputeSharedStencilAttachmentKey(width, height,
                                                               rt->numStencilSamples(), &sbKey);
        auto stencil = this->findByUniqueKey<GrStencilAttachment>(sbKey);
        if (!stencil) {
            // Need to try and create a new stencil
            stencil.reset(this->gpu()->createStencilAttachmentForRenderTarget(rt, width, height));
            if (stencil) {
                this->assignUniqueKeyToResource(sbKey, stencil.get());
                SkDEBUGCODE(newStencil = true;)
            }
        }
        if (rt->renderTargetPriv().attachStencilAttachment(std::move(stencil))) {
#ifdef SK_DEBUG
            // Fill the SB with an inappropriate value. opLists that use the
            // SB should clear it properly.
            if (newStencil) {
                SkASSERT(rt->renderTargetPriv().getStencilAttachment()->isDirty());
                this->gpu()->clearStencil(rt, 0xFFFF);
                SkASSERT(rt->renderTargetPriv().getStencilAttachment()->isDirty());
            }
#endif
        }
    }
    return SkToBool(rt->renderTargetPriv().getStencilAttachment());
}

sk_sp<GrRenderTarget> GrResourceProvider::wrapBackendTextureAsRenderTarget(
        const GrBackendTexture& tex, int sampleCnt)
{
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->wrapBackendTextureAsRenderTarget(tex, sampleCnt);
}

sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT GrResourceProvider::makeSemaphore(bool isOwned) {
    return fGpu->makeSemaphore(isOwned);
}

sk_sp<GrSemaphore> GrResourceProvider::wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                                            SemaphoreWrapType wrapType,
                                                            GrWrapOwnership ownership) {
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr : fGpu->wrapBackendSemaphore(semaphore,
                                                                      wrapType,
                                                                      ownership);
}

void GrResourceProvider::takeOwnershipOfSemaphore(sk_sp<GrSemaphore> semaphore) {
    semaphore->resetGpu(fGpu);
}

void GrResourceProvider::releaseOwnershipOfSemaphore(sk_sp<GrSemaphore> semaphore) {
    semaphore->resetGpu(nullptr);
}
