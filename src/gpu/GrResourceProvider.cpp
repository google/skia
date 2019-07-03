/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/GrResourceProvider.h"

#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrContext.h"
#include "include/private/GrResourceKey.h"
#include "include/private/GrSingleOwner.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrGpuBuffer.h"
#include "src/gpu/GrPath.h"
#include "src/gpu/GrPathRendering.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrResourceCache.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/SkGr.h"

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
}

sk_sp<GrTexture> GrResourceProvider::createTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                                   const GrMipLevel texels[], int mipLevelCount) {
    ASSERT_SINGLE_OWNER

    SkASSERT(mipLevelCount > 0);

    if (this->isAbandoned()) {
        return nullptr;
    }

    GrMipMapped mipMapped = mipLevelCount > 1 ? GrMipMapped::kYes : GrMipMapped::kNo;
    if (!fCaps->validateSurfaceDesc(desc, mipMapped)) {
        return nullptr;
    }

    SkAutoSTMalloc<14, GrMipLevel> tmpTexels;
    SkAutoSTArray<14, std::unique_ptr<char[]>> tmpDatas;
    if (mipLevelCount > 0 && texels) {
        tmpTexels.reset(mipLevelCount);
        tmpDatas.reset(mipLevelCount);
        int w = desc.fWidth;
        int h = desc.fHeight;
        size_t bpp = GrBytesPerPixel(desc.fConfig);
        for (int i = 0; i < mipLevelCount; ++i) {
            if (texels->fPixels) {
                size_t minRB = w * bpp;
                if (!texels->fRowBytes) {
                    tmpTexels[i].fRowBytes = minRB;
                    tmpTexels[i].fPixels = texels[i].fPixels;
                } else {
                    if (texels[i].fRowBytes < minRB) {
                        return nullptr;
                    }
                    if (this->caps()->writePixelsRowBytesSupport() &&
                        texels[i].fRowBytes != minRB) {
                        auto copy = new char[minRB * h];
                        tmpDatas[i].reset(copy);
                        SkRectMemcpy(copy, minRB, texels[i].fPixels, texels[i].fRowBytes, minRB, h);
                        tmpTexels[i].fPixels = copy;
                        tmpTexels[i].fRowBytes = minRB;
                    } else {
                        tmpTexels[i].fPixels = texels[i].fPixels;
                        tmpTexels[i].fRowBytes = texels[i].fRowBytes;
                    }
                }
            } else {
                tmpTexels[i].fPixels = nullptr;
                tmpTexels[i].fRowBytes = 0;
            }
            w = std::max(w / 2, 1);
            h = std::max(h / 2, 1);
        }
    }
    return fGpu->createTexture(desc, budgeted, tmpTexels.get(), mipLevelCount);
}

sk_sp<GrTexture> GrResourceProvider::getExactScratch(const GrSurfaceDesc& desc,
                                                     SkBudgeted budgeted, Flags flags) {
    sk_sp<GrTexture> tex(this->refScratchTexture(desc, flags));
    if (tex && SkBudgeted::kNo == budgeted) {
        tex->resourcePriv().makeUnbudgeted();
    }

    return tex;
}

sk_sp<GrTexture> GrResourceProvider::createTexture(const GrSurfaceDesc& desc,
                                                   SkBudgeted budgeted,
                                                   SkBackingFit fit,
                                                   const GrMipLevel& mipLevel,
                                                   Flags flags) {
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
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();

    SkColorType colorType;
    if (GrPixelConfigToColorType(desc.fConfig, &colorType)) {
        sk_sp<GrTexture> tex = (SkBackingFit::kApprox == fit)
                ? this->createApproxTexture(desc, flags)
                : this->createTexture(desc, budgeted, flags);
        if (!tex) {
            return nullptr;
        }

        sk_sp<GrTextureProxy> proxy = proxyProvider->createWrapped(std::move(tex),
                                                                   kTopLeft_GrSurfaceOrigin);
        if (!proxy) {
            return nullptr;
        }
        // Here we don't really know the alpha type of the data we want to upload. All we really
        // care about is that it is not converted. So we use the same alpha type of the data
        // and the surface context.
        static constexpr auto kAlphaType = kPremul_SkAlphaType;
        auto srcInfo = SkImageInfo::Make(desc.fWidth, desc.fHeight, colorType, kAlphaType);
        sk_sp<GrSurfaceContext> sContext = context->priv().makeWrappedSurfaceContext(
                std::move(proxy), SkColorTypeToGrColorType(colorType), kAlphaType);
        if (!sContext) {
            return nullptr;
        }
        SkAssertResult(
                sContext->writePixels(srcInfo, mipLevel.fPixels, mipLevel.fRowBytes, {0, 0}));
        return sk_ref_sp(sContext->asTextureProxy()->peekTexture());
    } else {
        return fGpu->createTexture(desc, budgeted, &mipLevel, 1);
    }
}

sk_sp<GrTexture> GrResourceProvider::createCompressedTexture(int width, int height,
                                                             SkImage::CompressionType compression,
                                                             SkBudgeted budgeted, SkData* data) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->createCompressedTexture(width, height, compression, budgeted, data->data(),
                                         data->size());
}

sk_sp<GrTexture> GrResourceProvider::createTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                                   Flags flags) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!fCaps->validateSurfaceDesc(desc, GrMipMapped::kNo)) {
        return nullptr;
    }

    // Compressed textures are read-only so they don't support re-use for scratch.
    if (!GrPixelConfigIsCompressed(desc.fConfig)) {
        sk_sp<GrTexture> tex = this->getExactScratch(desc, budgeted, flags);
        if (tex) {
            return tex;
        }
    }

    return fGpu->createTexture(desc, budgeted);
}

sk_sp<GrTexture> GrResourceProvider::createApproxTexture(const GrSurfaceDesc& desc,
                                                         Flags flags) {
    ASSERT_SINGLE_OWNER
    SkASSERT(Flags::kNone == flags || Flags::kNoPendingIO == flags);

    if (this->isAbandoned()) {
        return nullptr;
    }

    // Currently we don't recycle compressed textures as scratch.
    if (GrPixelConfigIsCompressed(desc.fConfig)) {
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

sk_sp<GrTexture> GrResourceProvider::refScratchTexture(const GrSurfaceDesc& desc, Flags flags) {
    ASSERT_SINGLE_OWNER
    SkASSERT(!this->isAbandoned());
    SkASSERT(!GrPixelConfigIsCompressed(desc.fConfig));
    SkASSERT(fCaps->validateSurfaceDesc(desc, GrMipMapped::kNo));

    // We could make initial clears work with scratch textures but it is a rare case so we just opt
    // to fall back to making a new texture.
    if (!SkToBool(desc.fFlags & kPerformInitialClear_GrSurfaceFlag) &&
        (fGpu->caps()->reuseScratchTextures() || (desc.fFlags & kRenderTarget_GrSurfaceFlag))) {

        GrScratchKey key;
        GrTexturePriv::ComputeScratchKey(desc, &key);
        auto scratchFlags = GrResourceCache::ScratchFlags::kNone;
        if (Flags::kNoPendingIO & flags) {
            scratchFlags |= GrResourceCache::ScratchFlags::kRequireNoPendingIO;
        } else  if (!(desc.fFlags & kRenderTarget_GrSurfaceFlag)) {
            // If it is not a render target then it will most likely be populated by
            // writePixels() which will trigger a flush if the texture has pending IO.
            scratchFlags |= GrResourceCache::ScratchFlags::kPreferNoPendingIO;
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
                                                        GrWrapOwnership ownership,
                                                        GrWrapCacheable cacheable,
                                                        GrIOType ioType) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->wrapBackendTexture(tex, ownership, cacheable, ioType);
}

sk_sp<GrTexture> GrResourceProvider::wrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                                  int sampleCnt,
                                                                  GrWrapOwnership ownership,
                                                                  GrWrapCacheable cacheable) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->wrapRenderableBackendTexture(tex, sampleCnt, ownership, cacheable);
}

sk_sp<GrRenderTarget> GrResourceProvider::wrapBackendRenderTarget(
        const GrBackendRenderTarget& backendRT)
{
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr : fGpu->wrapBackendRenderTarget(backendRT);
}

sk_sp<GrRenderTarget> GrResourceProvider::wrapVulkanSecondaryCBAsRenderTarget(
        const SkImageInfo& imageInfo, const GrVkDrawableInfo& vkInfo) {
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr : fGpu->wrapVulkanSecondaryCBAsRenderTarget(imageInfo,
                                                                                     vkInfo);

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

sk_sp<const GrGpuBuffer> GrResourceProvider::findOrMakeStaticBuffer(GrGpuBufferType intendedType,
                                                                    size_t size,
                                                                    const void* data,
                                                                    const GrUniqueKey& key) {
    if (auto buffer = this->findByUniqueKey<GrGpuBuffer>(key)) {
        return std::move(buffer);
    }
    if (auto buffer = this->createBuffer(size, intendedType, kStatic_GrAccessPattern, data)) {
        // We shouldn't bin and/or cache static buffers.
        SkASSERT(buffer->size() == size);
        SkASSERT(!buffer->resourcePriv().getScratchKey().isValid());
        SkASSERT(!buffer->resourcePriv().hasPendingIO_debugOnly());
        buffer->resourcePriv().setUniqueKey(key);
        return sk_sp<const GrGpuBuffer>(buffer);
    }
    return nullptr;
}

sk_sp<const GrGpuBuffer> GrResourceProvider::createPatternedIndexBuffer(const uint16_t* pattern,
                                                                        int patternSize,
                                                                        int reps,
                                                                        int vertCount,
                                                                        const GrUniqueKey* key) {
    size_t bufferSize = patternSize * reps * sizeof(uint16_t);

    // This is typically used in GrMeshDrawOps, so we assume kNoPendingIO.
    sk_sp<GrGpuBuffer> buffer(
            this->createBuffer(bufferSize, GrGpuBufferType::kIndex, kStatic_GrAccessPattern));
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
    if (key) {
        SkASSERT(key->isValid());
        this->assignUniqueKeyToResource(*key, buffer.get());
    }
    return std::move(buffer);
}

static constexpr int kMaxQuads = 1 << 12;  // max possible: (1 << 14) - 1;

sk_sp<const GrGpuBuffer> GrResourceProvider::createQuadIndexBuffer() {
    GR_STATIC_ASSERT(4 * kMaxQuads <= 65535);
    static const uint16_t kPattern[] = { 0, 1, 2, 2, 1, 3 };
    return this->createPatternedIndexBuffer(kPattern, 6, kMaxQuads, 4, nullptr);
}

int GrResourceProvider::QuadCountOfQuadBuffer() { return kMaxQuads; }

sk_sp<GrPath> GrResourceProvider::createPath(const SkPath& path, const GrStyle& style) {
    if (this->isAbandoned()) {
        return nullptr;
    }

    SkASSERT(this->gpu()->pathRendering());
    return this->gpu()->pathRendering()->createPath(path, style);
}

sk_sp<GrGpuBuffer> GrResourceProvider::createBuffer(size_t size, GrGpuBufferType intendedType,
                                                    GrAccessPattern accessPattern,
                                                    const void* data) {
    if (this->isAbandoned()) {
        return nullptr;
    }
    if (kDynamic_GrAccessPattern != accessPattern) {
        return this->gpu()->createBuffer(size, intendedType, accessPattern, data);
    }
    // bin by pow2 with a reasonable min
    static const size_t MIN_SIZE = 1 << 12;
    size_t allocSize = SkTMax(MIN_SIZE, GrNextSizePow2(size));

    GrScratchKey key;
    GrGpuBuffer::ComputeScratchKeyForDynamicVBO(allocSize, intendedType, &key);
    auto buffer =
            sk_sp<GrGpuBuffer>(static_cast<GrGpuBuffer*>(this->cache()->findAndRefScratchResource(
                    key, allocSize, GrResourceCache::ScratchFlags::kNone)));
    if (!buffer) {
        buffer = this->gpu()->createBuffer(allocSize, intendedType, kDynamic_GrAccessPattern);
        if (!buffer) {
            return nullptr;
        }
    }
    if (data) {
        buffer->updateData(data, size);
    }
    return buffer;
}

bool GrResourceProvider::attachStencilAttachment(GrRenderTarget* rt, int minStencilSampleCount) {
    SkASSERT(rt);
    GrStencilAttachment* stencil = rt->renderTargetPriv().getStencilAttachment();
    if (stencil && stencil->numSamples() >= minStencilSampleCount) {
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
        GrStencilAttachment::ComputeSharedStencilAttachmentKey(
                width, height, minStencilSampleCount, &sbKey);
        auto stencil = this->findByUniqueKey<GrStencilAttachment>(sbKey);
        if (!stencil) {
            // Need to try and create a new stencil
            stencil.reset(this->gpu()->createStencilAttachmentForRenderTarget(
                    rt, width, height, minStencilSampleCount));
            if (!stencil) {
                return false;
            }
            this->assignUniqueKeyToResource(sbKey, stencil.get());
        }
        rt->renderTargetPriv().attachStencilAttachment(std::move(stencil));
    }

    if (GrStencilAttachment* stencil = rt->renderTargetPriv().getStencilAttachment()) {
        return stencil->numSamples() >= minStencilSampleCount;
    }
    return false;
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
