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
#include "src/gpu/GrImageInfo.h"
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

sk_sp<GrTexture> GrResourceProvider::createTexture(const GrSurfaceDesc& desc,
                                                   const GrBackendFormat& format,
                                                   GrColorType colorType,
                                                   GrRenderable renderable,
                                                   int renderTargetSampleCnt,
                                                   SkBudgeted budgeted,
                                                   GrProtected isProtected,
                                                   const GrMipLevel texels[],
                                                   int mipLevelCount) {
    ASSERT_SINGLE_OWNER

    SkASSERT(mipLevelCount > 0);

    if (this->isAbandoned()) {
        return nullptr;
    }

    GrMipMapped mipMapped = mipLevelCount > 1 ? GrMipMapped::kYes : GrMipMapped::kNo;
    if (!fCaps->validateSurfaceParams({desc.fWidth, desc.fHeight}, format, desc.fConfig, renderable,
                                      renderTargetSampleCnt, mipMapped)) {
        return nullptr;
    }
    // Current rule is that you can provide no level data, just the base, or all the levels.
    bool hasPixels = mipLevelCount && texels[0].fPixels;
    auto scratch = this->getExactScratch(desc, format, renderable, renderTargetSampleCnt, budgeted,
                                         mipMapped, isProtected);
    if (scratch) {
        if (!hasPixels) {
            return scratch;
        }
        return this->writePixels(std::move(scratch), colorType, {desc.fWidth, desc.fHeight}, texels,
                                 mipLevelCount);
    }
    SkAutoSTMalloc<14, GrMipLevel> tmpTexels;
    SkAutoSTArray<14, std::unique_ptr<char[]>> tmpDatas;
    GrColorType tempColorType = GrColorType::kUnknown;
    if (hasPixels) {
        tempColorType = this->prepareLevels(format, colorType, {desc.fWidth, desc.fHeight}, texels,
                                            mipLevelCount, &tmpTexels, &tmpDatas);
        if (tempColorType == GrColorType::kUnknown) {
            return nullptr;
        }
    }
    return fGpu->createTexture(desc, format, renderable, renderTargetSampleCnt, budgeted,
                               isProtected, colorType, tempColorType, tmpTexels.get(),
                               mipLevelCount);
}

sk_sp<GrTexture> GrResourceProvider::getExactScratch(const GrSurfaceDesc& desc,
                                                     const GrBackendFormat& format,
                                                     GrRenderable renderable,
                                                     int renderTargetSampleCnt,
                                                     SkBudgeted budgeted,
                                                     GrMipMapped mipMapped,
                                                     GrProtected isProtected) {
    sk_sp<GrTexture> tex(this->refScratchTexture(desc, format, renderable, renderTargetSampleCnt,
                                                 mipMapped, isProtected));
    if (tex && SkBudgeted::kNo == budgeted) {
        tex->resourcePriv().makeUnbudgeted();
    }

    return tex;
}

sk_sp<GrTexture> GrResourceProvider::createTexture(const GrSurfaceDesc& desc,
                                                   const GrBackendFormat& format,
                                                   GrColorType colorType,
                                                   GrRenderable renderable,
                                                   int renderTargetSampleCnt,
                                                   SkBudgeted budgeted,
                                                   SkBackingFit fit,
                                                   GrProtected isProtected,
                                                   const GrMipLevel& mipLevel) {
    ASSERT_SINGLE_OWNER

    if (!mipLevel.fPixels) {
        return nullptr;
    }

    if (SkBackingFit::kApprox == fit) {
        if (this->isAbandoned()) {
            return nullptr;
        }
        if (!fCaps->validateSurfaceParams({desc.fWidth, desc.fHeight}, format, desc.fConfig,
                                          renderable, renderTargetSampleCnt, GrMipMapped::kNo)) {
            return nullptr;
        }

        auto tex = this->createApproxTexture(desc, format, renderable, renderTargetSampleCnt,
                                             isProtected);
        if (!tex) {
            return nullptr;
        }
        return this->writePixels(std::move(tex), colorType, {desc.fWidth, desc.fHeight}, &mipLevel,
                                 1);
    } else {
        return this->createTexture(desc, format, colorType, renderable, renderTargetSampleCnt,
                                   budgeted, isProtected, &mipLevel, 1);
    }
}

sk_sp<GrTexture> GrResourceProvider::createCompressedTexture(int width, int height,
                                                             const GrBackendFormat& format,
                                                             SkImage::CompressionType compression,
                                                             SkBudgeted budgeted, SkData* data) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->createCompressedTexture(width, height, format, compression, budgeted, data->data(),
                                         data->size());
}

sk_sp<GrTexture> GrResourceProvider::createTexture(const GrSurfaceDesc& desc,
                                                   const GrBackendFormat& format,
                                                   GrRenderable renderable,
                                                   int renderTargetSampleCnt,
                                                   GrMipMapped mipMapped,
                                                   SkBudgeted budgeted,
                                                   GrProtected isProtected) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!fCaps->validateSurfaceParams({desc.fWidth, desc.fHeight}, format, desc.fConfig, renderable,
                                      renderTargetSampleCnt, mipMapped)) {
        return nullptr;
    }

    // Compressed textures are read-only so they don't support re-use for scratch.
    // TODO: Support GrMipMapped::kYes in scratch texture lookup here.
    if (!GrPixelConfigIsCompressed(desc.fConfig)) {
        sk_sp<GrTexture> tex = this->getExactScratch(
                desc, format, renderable, renderTargetSampleCnt, budgeted, mipMapped, isProtected);
        if (tex) {
            return tex;
        }
    }

    return fGpu->createTexture(desc, format, renderable, renderTargetSampleCnt, mipMapped, budgeted,
                               isProtected);
}

// Map 'value' to a larger multiple of 2. Values <= 'kMagicTol' will pop up to
// the next power of 2. Those above 'kMagicTol' will only go up half the floor power of 2.
uint32_t GrResourceProvider::MakeApprox(uint32_t value) {
    static const int kMagicTol = 1024;

    value = SkTMax(kMinScratchTextureSize, value);

    if (SkIsPow2(value)) {
        return value;
    }

    uint32_t ceilPow2 = GrNextPow2(value);
    if (value <= kMagicTol) {
        return ceilPow2;
    }

    uint32_t floorPow2 = ceilPow2 >> 1;
    uint32_t mid = floorPow2 + (floorPow2 >> 1);

    if (value <= mid) {
        return mid;
    }

    return ceilPow2;
}

sk_sp<GrTexture> GrResourceProvider::createApproxTexture(const GrSurfaceDesc& desc,
                                                         const GrBackendFormat& format,
                                                         GrRenderable renderable,
                                                         int renderTargetSampleCnt,
                                                         GrProtected isProtected) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    // Currently we don't recycle compressed textures as scratch.
    if (GrPixelConfigIsCompressed(desc.fConfig)) {
        return nullptr;
    }

    if (!fCaps->validateSurfaceParams({desc.fWidth, desc.fHeight}, format, desc.fConfig, renderable,
                                      renderTargetSampleCnt, GrMipMapped::kNo)) {
        return nullptr;
    }

    // bin by some multiple or power of 2 with a reasonable min
    GrSurfaceDesc copyDesc(desc);
    copyDesc.fWidth = MakeApprox(desc.fWidth);
    copyDesc.fHeight = MakeApprox(desc.fHeight);

    if (auto tex = this->refScratchTexture(copyDesc, format, renderable, renderTargetSampleCnt,
                                           GrMipMapped::kNo, isProtected)) {
        return tex;
    }

    return fGpu->createTexture(copyDesc, format, renderable, renderTargetSampleCnt,
                               GrMipMapped::kNo, SkBudgeted::kYes, isProtected);
}

sk_sp<GrTexture> GrResourceProvider::refScratchTexture(const GrSurfaceDesc& desc,
                                                       const GrBackendFormat& format,
                                                       GrRenderable renderable,
                                                       int renderTargetSampleCnt,
                                                       GrMipMapped mipMapped,
                                                       GrProtected isProtected) {
    ASSERT_SINGLE_OWNER
    SkASSERT(!this->isAbandoned());
    SkASSERT(!GrPixelConfigIsCompressed(desc.fConfig));
    SkASSERT(fCaps->validateSurfaceParams({desc.fWidth, desc.fHeight}, format, desc.fConfig,
                                          renderable, renderTargetSampleCnt, GrMipMapped::kNo));

    // We could make initial clears work with scratch textures but it is a rare case so we just opt
    // to fall back to making a new texture.
    if (fGpu->caps()->reuseScratchTextures() || renderable == GrRenderable::kYes) {
        GrScratchKey key;
        GrTexturePriv::ComputeScratchKey(desc.fConfig, desc.fWidth, desc.fHeight, renderable,
                                         renderTargetSampleCnt, mipMapped, isProtected, &key);
        GrGpuResource* resource = fCache->findAndRefScratchResource(key);
        if (resource) {
            fGpu->stats()->incNumScratchTexturesReused();
            GrSurface* surface = static_cast<GrSurface*>(resource);
            return sk_sp<GrTexture>(surface->asTexture());
        }
    }

    return nullptr;
}

sk_sp<GrTexture> GrResourceProvider::wrapBackendTexture(const GrBackendTexture& tex,
                                                        GrColorType colorType,
                                                        GrWrapOwnership ownership,
                                                        GrWrapCacheable cacheable,
                                                        GrIOType ioType) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->wrapBackendTexture(tex, colorType, ownership, cacheable, ioType);
}

sk_sp<GrTexture> GrResourceProvider::wrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                                  int sampleCnt,
                                                                  GrColorType colorType,
                                                                  GrWrapOwnership ownership,
                                                                  GrWrapCacheable cacheable) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->wrapRenderableBackendTexture(tex, sampleCnt, colorType, ownership, cacheable);
}

sk_sp<GrRenderTarget> GrResourceProvider::wrapBackendRenderTarget(
        const GrBackendRenderTarget& backendRT, GrColorType colorType)
{
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr : fGpu->wrapBackendRenderTarget(backendRT, colorType);
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
        return buffer;
    }
    if (auto buffer = this->createBuffer(size, intendedType, kStatic_GrAccessPattern, data)) {
        // We shouldn't bin and/or cache static buffers.
        SkASSERT(buffer->size() == size);
        SkASSERT(!buffer->resourcePriv().getScratchKey().isValid());
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
    return buffer;
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
                    key)));
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
        const GrBackendTexture& tex, int sampleCnt, GrColorType colorType)
{
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->wrapBackendTextureAsRenderTarget(tex, sampleCnt, colorType);
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

// Ensures the row bytes are populated (not 0) and makes a copy to a temporary
// to make the row bytes tight if necessary. Returns false if the input row bytes are invalid.
static bool prepare_level(const GrMipLevel& inLevel,
                          const SkISize& size,
                          bool rowBytesSupport,
                          GrColorType origColorType,
                          GrColorType allowedColorType,
                          GrMipLevel* outLevel,
                          std::unique_ptr<char[]>* data) {
    if (!inLevel.fPixels) {
        outLevel->fPixels = nullptr;
        outLevel->fRowBytes = 0;
        return true;
    }
    size_t minRB = size.fWidth * GrColorTypeBytesPerPixel(origColorType);
    size_t actualRB = inLevel.fRowBytes ? inLevel.fRowBytes : minRB;
    if (actualRB < minRB) {
        return false;
    }
    if (origColorType == allowedColorType && (actualRB == minRB || rowBytesSupport)) {
        outLevel->fRowBytes = actualRB;
        outLevel->fPixels = inLevel.fPixels;
        return true;
    }
    auto tempRB = size.fWidth * GrColorTypeBytesPerPixel(allowedColorType);
    data->reset(new char[tempRB * size.fHeight]);
    outLevel->fPixels = data->get();
    outLevel->fRowBytes = tempRB;
    GrImageInfo srcInfo(origColorType,    kUnpremul_SkAlphaType, nullptr, size);
    GrImageInfo dstInfo(allowedColorType, kUnpremul_SkAlphaType, nullptr, size);
    return GrConvertPixels(dstInfo, data->get(), tempRB, srcInfo, inLevel.fPixels, actualRB);
}

GrColorType GrResourceProvider::prepareLevels(const GrBackendFormat& format,
                                              GrColorType colorType,
                                              const SkISize& baseSize,
                                              const GrMipLevel texels[],
                                              int mipLevelCount,
                                              TempLevels* tempLevels,
                                              TempLevelDatas* tempLevelDatas) const {
    SkASSERT(mipLevelCount && texels && texels[0].fPixels);

    auto allowedColorType =
            this->caps()->supportedWritePixelsColorType(colorType, format, colorType).fColorType;
    if (allowedColorType == GrColorType::kUnknown) {
        return GrColorType::kUnknown;
    }
    bool rowBytesSupport = this->caps()->writePixelsRowBytesSupport();
    tempLevels->reset(mipLevelCount);
    tempLevelDatas->reset(mipLevelCount);
    auto size = baseSize;
    for (int i = 0; i < mipLevelCount; ++i) {
        if (!prepare_level(texels[i], size, rowBytesSupport, colorType, allowedColorType,
                           &(*tempLevels)[i], &(*tempLevelDatas)[i])) {
            return GrColorType::kUnknown;
        }
        size = {std::max(size.fWidth / 2, 1), std::max(size.fHeight / 2, 1)};
    }
    return allowedColorType;
}

sk_sp<GrTexture> GrResourceProvider::writePixels(sk_sp<GrTexture> texture,
                                                 GrColorType colorType,
                                                 const SkISize& baseSize,
                                                 const GrMipLevel texels[],
                                                 int mipLevelCount) const {
    SkASSERT(!this->isAbandoned());
    SkASSERT(texture);
    SkASSERT(colorType != GrColorType::kUnknown);
    SkASSERT(mipLevelCount && texels && texels[0].fPixels);

    SkAutoSTMalloc<14, GrMipLevel> tmpTexels;
    SkAutoSTArray<14, std::unique_ptr<char[]>> tmpDatas;
    auto tempColorType = this->prepareLevels(texture->backendFormat(), colorType, baseSize, texels,
                                             mipLevelCount, &tmpTexels, &tmpDatas);
    if (tempColorType == GrColorType::kUnknown) {
        return nullptr;
    }
    SkAssertResult(fGpu->writePixels(texture.get(), 0, 0, baseSize.fWidth, baseSize.fHeight,
                                     colorType, tempColorType, tmpTexels.get(), mipLevelCount));
    return texture;
}
