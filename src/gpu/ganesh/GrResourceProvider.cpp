/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrResourceProvider.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkData.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrTypes.h"
#include "include/private/base/SingleOwner.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkMathPriv.h"
#include "src/core/SkMipmap.h"
#include "src/gpu/BufferWriter.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/ganesh/GrAttachment.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/GrGpuBuffer.h"
#include "src/gpu/ganesh/GrGpuResourcePriv.h"
#include "src/gpu/ganesh/GrImageInfo.h"
#include "src/gpu/ganesh/GrPixmap.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrResourceCache.h"
#include "src/gpu/ganesh/GrSemaphore.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrTexture.h"

#include <algorithm>
#include <utility>

struct SkImageInfo;

using namespace skia_private;

#define ASSERT_SINGLE_OWNER SKGPU_ASSERT_SINGLE_OWNER(fSingleOwner)

GrResourceProvider::GrResourceProvider(GrGpu* gpu,
                                       GrResourceCache* cache,
                                       skgpu::SingleOwner* owner)
        : fCache(cache)
        , fGpu(gpu)
#ifdef SK_DEBUG
        , fSingleOwner(owner)
#endif
{
    fCaps = sk_ref_sp(fGpu->caps());
}

sk_sp<GrTexture> GrResourceProvider::createTexture(SkISize dimensions,
                                                   const GrBackendFormat& format,
                                                   GrTextureType textureType,
                                                   GrColorType colorType,
                                                   GrRenderable renderable,
                                                   int renderTargetSampleCnt,
                                                   skgpu::Budgeted budgeted,
                                                   skgpu::Mipmapped mipmapped,
                                                   GrProtected isProtected,
                                                   const GrMipLevel texels[],
                                                   std::string_view label) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    int numMipLevels = 1;
    if (mipmapped == skgpu::Mipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.fWidth, dimensions.fHeight) + 1;
    }

    if (!fCaps->validateSurfaceParams(dimensions,
                                      format,
                                      renderable,
                                      renderTargetSampleCnt,
                                      mipmapped,
                                      textureType)) {
        return nullptr;
    }
    // Current rule is that you can provide no level data, just the base, or all the levels.
    bool hasPixels = texels[0].fPixels;
    auto scratch = this->getExactScratch(dimensions,
                                         format,
                                         textureType,
                                         renderable,
                                         renderTargetSampleCnt,
                                         budgeted,
                                         mipmapped,
                                         isProtected,
                                         label);
    if (scratch) {
        if (!hasPixels) {
            return scratch;
        }
        return this->writePixels(std::move(scratch), colorType, dimensions, texels, numMipLevels);
    }
    AutoSTArray<14, GrMipLevel> tmpTexels;
    AutoSTArray<14, std::unique_ptr<char[]>> tmpDatas;
    GrColorType tempColorType = GrColorType::kUnknown;
    if (hasPixels) {
        tempColorType = this->prepareLevels(format, colorType, dimensions, texels, numMipLevels,
                                            &tmpTexels, &tmpDatas);
        if (tempColorType == GrColorType::kUnknown) {
            return nullptr;
        }
    }
    return fGpu->createTexture(dimensions,
                               format,
                               textureType,
                               renderable,
                               renderTargetSampleCnt,
                               budgeted,
                               isProtected,
                               colorType,
                               tempColorType,
                               tmpTexels.get(),
                               numMipLevels,
                               label);
}

sk_sp<GrTexture> GrResourceProvider::getExactScratch(SkISize dimensions,
                                                     const GrBackendFormat& format,
                                                     GrTextureType textureType,
                                                     GrRenderable renderable,
                                                     int renderTargetSampleCnt,
                                                     skgpu::Budgeted budgeted,
                                                     skgpu::Mipmapped mipmapped,
                                                     GrProtected isProtected,
                                                     std::string_view label) {
    sk_sp<GrTexture> tex(this->findAndRefScratchTexture(dimensions,
                                                        format,
                                                        textureType,
                                                        renderable,
                                                        renderTargetSampleCnt,
                                                        mipmapped,
                                                        isProtected,
                                                        label));
    if (tex && skgpu::Budgeted::kNo == budgeted) {
        tex->resourcePriv().makeUnbudgeted();
    }

    return tex;
}

sk_sp<GrTexture> GrResourceProvider::createTexture(SkISize dimensions,
                                                   const GrBackendFormat& format,
                                                   GrTextureType textureType,
                                                   GrColorType colorType,
                                                   GrRenderable renderable,
                                                   int renderTargetSampleCnt,
                                                   skgpu::Budgeted budgeted,
                                                   SkBackingFit fit,
                                                   GrProtected isProtected,
                                                   const GrMipLevel& mipLevel,
                                                   std::string_view label) {
    ASSERT_SINGLE_OWNER

    if (!mipLevel.fPixels) {
        return nullptr;
    }

    if (SkBackingFit::kApprox == fit) {
        if (this->isAbandoned()) {
            return nullptr;
        }
        if (!fCaps->validateSurfaceParams(dimensions,
                                          format,
                                          renderable,
                                          renderTargetSampleCnt,
                                          skgpu::Mipmapped::kNo,
                                          textureType)) {
            return nullptr;
        }

        auto tex = this->createApproxTexture(dimensions,
                                             format,
                                             textureType,
                                             renderable,
                                             renderTargetSampleCnt,
                                             isProtected,
                                             label);
        if (!tex) {
            return nullptr;
        }
        return this->writePixels(std::move(tex), colorType, dimensions, &mipLevel, 1);
    } else {
        return this->createTexture(dimensions,
                                   format,
                                   textureType,
                                   colorType,
                                   renderable,
                                   renderTargetSampleCnt,
                                   budgeted,
                                   skgpu::Mipmapped::kNo,
                                   isProtected,
                                   &mipLevel,
                                   label);
    }
}

sk_sp<GrTexture> GrResourceProvider::createCompressedTexture(SkISize dimensions,
                                                             const GrBackendFormat& format,
                                                             skgpu::Budgeted budgeted,
                                                             skgpu::Mipmapped mipmapped,
                                                             GrProtected isProtected,
                                                             SkData* data,
                                                             std::string_view label) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }
    return fGpu->createCompressedTexture(dimensions,
                                         format,
                                         budgeted,
                                         mipmapped,
                                         isProtected,
                                         data->data(),
                                         data->size());
}

sk_sp<GrTexture> GrResourceProvider::createTexture(SkISize dimensions,
                                                   const GrBackendFormat& format,
                                                   GrTextureType textureType,
                                                   GrRenderable renderable,
                                                   int renderTargetSampleCnt,
                                                   skgpu::Mipmapped mipmapped,
                                                   skgpu::Budgeted budgeted,
                                                   GrProtected isProtected,
                                                   std::string_view label) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!fCaps->validateSurfaceParams(dimensions, format, renderable, renderTargetSampleCnt,
                                      mipmapped, textureType)) {
        return nullptr;
    }

    // Currently we don't recycle compressed textures as scratch. Additionally all compressed
    // textures should be created through the createCompressedTexture function.
    SkASSERT(!this->caps()->isFormatCompressed(format));

    // TODO: Support skgpu::Mipmapped::kYes in scratch texture lookup here.
    sk_sp<GrTexture> tex =
            this->getExactScratch(dimensions,
                                  format,
                                  textureType,
                                  renderable,
                                  renderTargetSampleCnt,
                                  budgeted,
                                  mipmapped,
                                  isProtected,
                                  label);
    if (tex) {
        return tex;
    }

    return fGpu->createTexture(dimensions,
                               format,
                               textureType,
                               renderable,
                               renderTargetSampleCnt,
                               mipmapped,
                               budgeted,
                               isProtected,
                               label);
}

sk_sp<GrTexture> GrResourceProvider::createApproxTexture(SkISize dimensions,
                                                         const GrBackendFormat& format,
                                                         GrTextureType textureType,
                                                         GrRenderable renderable,
                                                         int renderTargetSampleCnt,
                                                         GrProtected isProtected,
                                                         std::string_view label) {
    ASSERT_SINGLE_OWNER

    if (this->isAbandoned()) {
        return nullptr;
    }

    // Currently we don't recycle compressed textures as scratch. Additionally all compressed
    // textures should be created through the createCompressedTexture function.
    SkASSERT(!this->caps()->isFormatCompressed(format));

    if (!fCaps->validateSurfaceParams(dimensions,
                                      format,
                                      renderable,
                                      renderTargetSampleCnt,
                                      skgpu::Mipmapped::kNo,
                                      textureType)) {
        return nullptr;
    }

    auto copyDimensions = skgpu::GetApproxSize(dimensions);

    if (auto tex = this->findAndRefScratchTexture(copyDimensions,
                                                  format,
                                                  textureType,
                                                  renderable,
                                                  renderTargetSampleCnt,
                                                  skgpu::Mipmapped::kNo,
                                                  isProtected,
                                                  label)) {
        return tex;
    }

    return fGpu->createTexture(copyDimensions,
                               format,
                               textureType,
                               renderable,
                               renderTargetSampleCnt,
                               skgpu::Mipmapped::kNo,
                               skgpu::Budgeted::kYes,
                               isProtected,
                               label);
}

sk_sp<GrTexture> GrResourceProvider::findAndRefScratchTexture(const skgpu::ScratchKey& key,
                                                              std::string_view label) {
    ASSERT_SINGLE_OWNER
    SkASSERT(!this->isAbandoned());
    SkASSERT(key.isValid());

    if (GrGpuResource* resource = fCache->findAndRefScratchResource(key)) {
        fGpu->stats()->incNumScratchTexturesReused();
        GrSurface* surface = static_cast<GrSurface*>(resource);
        resource->setLabel(std::move(label));
        return sk_sp<GrTexture>(surface->asTexture());
    }
    return nullptr;
}

sk_sp<GrTexture> GrResourceProvider::findAndRefScratchTexture(SkISize dimensions,
                                                              const GrBackendFormat& format,
                                                              GrTextureType textureType,
                                                              GrRenderable renderable,
                                                              int renderTargetSampleCnt,
                                                              skgpu::Mipmapped mipmapped,
                                                              GrProtected isProtected,
                                                              std::string_view label) {
    ASSERT_SINGLE_OWNER
    SkASSERT(!this->isAbandoned());
    SkASSERT(!this->caps()->isFormatCompressed(format));
    SkASSERT(fCaps->validateSurfaceParams(dimensions,
                                          format,
                                          renderable,
                                          renderTargetSampleCnt,
                                          skgpu::Mipmapped::kNo,
                                          textureType));

    // We could make initial clears work with scratch textures but it is a rare case so we just opt
    // to fall back to making a new texture.
    if (fGpu->caps()->reuseScratchTextures() || renderable == GrRenderable::kYes) {
        skgpu::ScratchKey key;
        GrTexture::ComputeScratchKey(*this->caps(), format, dimensions, renderable,
                                     renderTargetSampleCnt, mipmapped, isProtected, &key);
        return this->findAndRefScratchTexture(key, label);
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

sk_sp<GrTexture> GrResourceProvider::wrapCompressedBackendTexture(const GrBackendTexture& tex,
                                                                  GrWrapOwnership ownership,
                                                                  GrWrapCacheable cacheable) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned()) {
        return nullptr;
    }

    return fGpu->wrapCompressedBackendTexture(tex, ownership, cacheable);
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
        const GrBackendRenderTarget& backendRT) {
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr : fGpu->wrapBackendRenderTarget(backendRT);
}

sk_sp<GrRenderTarget> GrResourceProvider::wrapVulkanSecondaryCBAsRenderTarget(
        const SkImageInfo& imageInfo, const GrVkDrawableInfo& vkInfo) {
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr : fGpu->wrapVulkanSecondaryCBAsRenderTarget(imageInfo,
                                                                                     vkInfo);

}

void GrResourceProvider::assignUniqueKeyToResource(const skgpu::UniqueKey& key,
                                                   GrGpuResource* resource) {
    ASSERT_SINGLE_OWNER
    if (this->isAbandoned() || !resource) {
        return;
    }
    resource->resourcePriv().setUniqueKey(key);
}

sk_sp<GrGpuResource> GrResourceProvider::findResourceByUniqueKey(const skgpu::UniqueKey& key) {
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr
                               : sk_sp<GrGpuResource>(fCache->findAndRefUniqueResource(key));
}

sk_sp<const GrGpuBuffer> GrResourceProvider::findOrMakeStaticBuffer(GrGpuBufferType intendedType,
                                                                    size_t size,
                                                                    const void* staticData,
                                                                    const skgpu::UniqueKey& key) {
    if (auto buffer = this->findByUniqueKey<GrGpuBuffer>(key)) {
        return buffer;
    }

    auto buffer = this->createBuffer(staticData, size, intendedType, kStatic_GrAccessPattern);
    if (!buffer) {
        return nullptr;
    }

    // We shouldn't bin and/or cache static buffers.
    SkASSERT(buffer->size() == size);
    SkASSERT(!buffer->resourcePriv().getScratchKey().isValid());

    buffer->resourcePriv().setUniqueKey(key);

    return buffer;
}

sk_sp<const GrGpuBuffer> GrResourceProvider::findOrMakeStaticBuffer(
        GrGpuBufferType intendedType,
        size_t size,
        const skgpu::UniqueKey& uniqueKey,
        InitializeBufferFn initializeBufferFn) {
    if (auto buffer = this->findByUniqueKey<GrGpuBuffer>(uniqueKey)) {
        return buffer;
    }

    auto buffer = this->createBuffer(size,
                                     intendedType,
                                     kStatic_GrAccessPattern,
                                     ZeroInit::kNo);
    if (!buffer) {
        return nullptr;
    }

    // We shouldn't bin and/or cache static buffers.
    SkASSERT(buffer->size() == size);
    SkASSERT(!buffer->resourcePriv().getScratchKey().isValid());

    buffer->resourcePriv().setUniqueKey(uniqueKey);

    // Map the buffer. Use a staging buffer on the heap if mapping isn't supported.
    skgpu::VertexWriter vertexWriter = {buffer->map(), size};
    AutoTMalloc<char> stagingBuffer;
    if (!vertexWriter) {
        SkASSERT(!buffer->isMapped());
        vertexWriter = {stagingBuffer.reset(size), size};
    }

    initializeBufferFn(std::move(vertexWriter), size);

    if (buffer->isMapped()) {
        buffer->unmap();
    } else {
        buffer->updateData(stagingBuffer, /*offset=*/0, size, /*preserve=*/false);
    }

    return buffer;
}

sk_sp<const GrGpuBuffer> GrResourceProvider::createPatternedIndexBuffer(
        const uint16_t* pattern,
        int patternSize,
        int reps,
        int vertCount,
        const skgpu::UniqueKey* key) {
    size_t bufferSize = patternSize * reps * sizeof(uint16_t);

    sk_sp<GrGpuBuffer> buffer = this->createBuffer(bufferSize,
                                                   GrGpuBufferType::kIndex,
                                                   kStatic_GrAccessPattern,
                                                   ZeroInit::kNo);
    if (!buffer) {
        return nullptr;
    }
    uint16_t* data = (uint16_t*) buffer->map();
    AutoTArray<uint16_t> temp;
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
        if (!buffer->updateData(data, /*offset=*/0, bufferSize, /*preserve=*/false)) {
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

///////////////////////////////////////////////////////////////////////////////////////////////////
static constexpr int kMaxNumNonAAQuads = 1 << 12;  // max possible: (1 << 14) - 1;
static const int kVertsPerNonAAQuad = 4;
static const int kIndicesPerNonAAQuad = 6;

sk_sp<const GrGpuBuffer> GrResourceProvider::createNonAAQuadIndexBuffer() {
    static_assert(kVertsPerNonAAQuad * kMaxNumNonAAQuads <= 65535);  // indices fit in a uint16_t

    static const uint16_t kNonAAQuadIndexPattern[] = {
        0, 1, 2, 2, 1, 3
    };

    static_assert(std::size(kNonAAQuadIndexPattern) == kIndicesPerNonAAQuad);

    return this->createPatternedIndexBuffer(kNonAAQuadIndexPattern, kIndicesPerNonAAQuad,
                                            kMaxNumNonAAQuads, kVertsPerNonAAQuad, nullptr);
}

int GrResourceProvider::MaxNumNonAAQuads() { return kMaxNumNonAAQuads; }
int GrResourceProvider::NumVertsPerNonAAQuad() { return kVertsPerNonAAQuad; }
int GrResourceProvider::NumIndicesPerNonAAQuad() { return kIndicesPerNonAAQuad; }

///////////////////////////////////////////////////////////////////////////////////////////////////
static constexpr int kMaxNumAAQuads = 1 << 9;  // max possible: (1 << 13) - 1;
static const int kVertsPerAAQuad = 8;
static const int kIndicesPerAAQuad = 30;

sk_sp<const GrGpuBuffer> GrResourceProvider::createAAQuadIndexBuffer() {
    static_assert(kVertsPerAAQuad * kMaxNumAAQuads <= 65535);  // indices fit in a uint16_t

    // clang-format off
    static const uint16_t kAAQuadIndexPattern[] = {
        0, 1, 2, 1, 3, 2,
        0, 4, 1, 4, 5, 1,
        0, 6, 4, 0, 2, 6,
        2, 3, 6, 3, 7, 6,
        1, 5, 3, 3, 5, 7,
    };
    // clang-format on

    static_assert(std::size(kAAQuadIndexPattern) == kIndicesPerAAQuad);

    return this->createPatternedIndexBuffer(kAAQuadIndexPattern, kIndicesPerAAQuad,
                                            kMaxNumAAQuads, kVertsPerAAQuad, nullptr);
}

int GrResourceProvider::MaxNumAAQuads() { return kMaxNumAAQuads; }
int GrResourceProvider::NumVertsPerAAQuad() { return kVertsPerAAQuad; }
int GrResourceProvider::NumIndicesPerAAQuad() { return kIndicesPerAAQuad; }

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<GrGpuBuffer> GrResourceProvider::createBuffer(size_t size,
                                                    GrGpuBufferType intendedType,
                                                    GrAccessPattern accessPattern,
                                                    ZeroInit zeroInit) {
    if (this->isAbandoned()) {
        return nullptr;
    }
    if (kDynamic_GrAccessPattern != accessPattern) {
        if (this->caps()->buffersAreInitiallyZero()) {
            zeroInit = ZeroInit::kNo;
        }
        sk_sp<GrGpuBuffer> buffer = this->gpu()->createBuffer(size, intendedType, accessPattern);
        if (buffer && zeroInit == ZeroInit::kYes && !buffer->clearToZero()) {
            return nullptr;
        }
        return buffer;
    }
    // bin by pow2+midpoint with a reasonable min
    static const size_t MIN_SIZE = 1 << 12;
    static const size_t MIN_UNIFORM_SIZE = 1 << 7;
    size_t allocSize = intendedType == GrGpuBufferType::kUniform ? std::max(size, MIN_UNIFORM_SIZE)
                                                                 : std::max(size, MIN_SIZE);
    size_t ceilPow2 = GrNextSizePow2(allocSize);
    size_t floorPow2 = ceilPow2 >> 1;
    size_t mid = floorPow2 + (floorPow2 >> 1);
    allocSize = (allocSize <= mid) ? mid : ceilPow2;

    skgpu::ScratchKey key;
    GrGpuBuffer::ComputeScratchKeyForDynamicBuffer(allocSize, intendedType, &key);
    auto buffer =
            sk_sp<GrGpuBuffer>(static_cast<GrGpuBuffer*>(this->cache()->findAndRefScratchResource(
                    key)));
    if (!buffer) {
        if (this->caps()->buffersAreInitiallyZero()) {
            zeroInit = ZeroInit::kNo;
        }
        buffer = this->gpu()->createBuffer(allocSize, intendedType, kDynamic_GrAccessPattern);
    }
    if (buffer && zeroInit == ZeroInit::kYes && !buffer->clearToZero()) {
        return nullptr;
    }
    return buffer;
}

sk_sp<GrGpuBuffer> GrResourceProvider::createBuffer(const void* data,
                                                    size_t size,
                                                    GrGpuBufferType type,
                                                    GrAccessPattern pattern) {
    SkASSERT(data);
    auto buffer = this->createBuffer(size, type, pattern, ZeroInit::kNo);
    if (!buffer) {
        return nullptr;
    }
    if (!buffer->updateData(data, /*offset=*/0, size, /*preserve=*/false)) {
        return nullptr;
    }
    return buffer;
}

static int num_stencil_samples(const GrRenderTarget* rt, bool useMSAASurface, const GrCaps& caps) {
    int numSamples = rt->numSamples();
    if (numSamples == 1 && useMSAASurface) {  // Are we using dynamic msaa?
        numSamples = caps.internalMultisampleCount(rt->backendFormat());
        SkASSERT(numSamples > 1);  // Caller must ensure dmsaa is supported before trying to use it.
    }
    return numSamples;
}

bool GrResourceProvider::attachStencilAttachment(GrRenderTarget* rt, bool useMSAASurface) {
    SkASSERT(rt);
    SkASSERT(!this->caps()->avoidStencilBuffers());

    GrAttachment* stencil = rt->getStencilAttachment(useMSAASurface);
    if (stencil) {
        SkASSERT(stencil->numSamples() == num_stencil_samples(rt, useMSAASurface, *this->caps()));
        return true;
    }

    if (!rt->wasDestroyed() && rt->canAttemptStencilAttachment(useMSAASurface)) {
        skgpu::UniqueKey sbKey;

#if 0
        if (this->caps()->oversizedStencilSupport()) {
            width  = SkNextPow2(width);
            height = SkNextPow2(height);
        }
#endif
        GrBackendFormat stencilFormat = this->gpu()->getPreferredStencilFormat(rt->backendFormat());
        if (!stencilFormat.isValid()) {
            return false;
        }
        GrProtected isProtected = rt->isProtected() ? GrProtected::kYes : GrProtected::kNo;
        int numStencilSamples = num_stencil_samples(rt, useMSAASurface, *this->caps());
        GrAttachment::ComputeSharedAttachmentUniqueKey(*this->caps(),
                                                       stencilFormat,
                                                       rt->dimensions(),
                                                       GrAttachment::UsageFlags::kStencilAttachment,
                                                       numStencilSamples,
                                                       skgpu::Mipmapped::kNo,
                                                       isProtected,
                                                       GrMemoryless::kNo,
                                                       &sbKey);
        auto keyedStencil = this->findByUniqueKey<GrAttachment>(sbKey);
        if (!keyedStencil) {
            // Need to try and create a new stencil
            keyedStencil = this->gpu()->makeStencilAttachment(rt->backendFormat(), rt->dimensions(),
                                                              numStencilSamples);
            if (!keyedStencil) {
                return false;
            }
            this->assignUniqueKeyToResource(sbKey, keyedStencil.get());
        }
        rt->attachStencilAttachment(std::move(keyedStencil), useMSAASurface);
    }
    stencil = rt->getStencilAttachment(useMSAASurface);
    SkASSERT(!stencil ||
             stencil->numSamples() == num_stencil_samples(rt, useMSAASurface, *this->caps()));
    return stencil != nullptr;
}

sk_sp<GrAttachment> GrResourceProvider::getDiscardableMSAAAttachment(SkISize dimensions,
                                                                     const GrBackendFormat& format,
                                                                     int sampleCnt,
                                                                     GrProtected isProtected,
                                                                     GrMemoryless memoryless) {
    ASSERT_SINGLE_OWNER

    SkASSERT(sampleCnt > 1);

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!fCaps->validateSurfaceParams(dimensions,
                                      format,
                                      GrRenderable::kYes,
                                      sampleCnt,
                                      skgpu::Mipmapped::kNo,
                                      GrTextureType::kNone)) {
        return nullptr;
    }

    skgpu::UniqueKey key;
    GrAttachment::ComputeSharedAttachmentUniqueKey(*this->caps(),
                                                   format,
                                                   dimensions,
                                                   GrAttachment::UsageFlags::kColorAttachment,
                                                   sampleCnt,
                                                   skgpu::Mipmapped::kNo,
                                                   isProtected,
                                                   memoryless,
                                                   &key);
    auto msaaAttachment = this->findByUniqueKey<GrAttachment>(key);
    if (msaaAttachment) {
        return msaaAttachment;
    }
    msaaAttachment = this->makeMSAAAttachment(dimensions, format, sampleCnt, isProtected,
                                              memoryless);
    if (msaaAttachment) {
        this->assignUniqueKeyToResource(key, msaaAttachment.get());
    }
    return msaaAttachment;
}

sk_sp<GrAttachment> GrResourceProvider::makeMSAAAttachment(SkISize dimensions,
                                                           const GrBackendFormat& format,
                                                           int sampleCnt,
                                                           GrProtected isProtected,
                                                           GrMemoryless memoryless) {
    ASSERT_SINGLE_OWNER

    SkASSERT(sampleCnt > 1);

    if (this->isAbandoned()) {
        return nullptr;
    }

    if (!fCaps->validateSurfaceParams(dimensions,
                                      format,
                                      GrRenderable::kYes,
                                      sampleCnt,
                                      skgpu::Mipmapped::kNo,
                                      GrTextureType::kNone)) {
        return nullptr;
    }

    auto scratch = this->refScratchMSAAAttachment(dimensions,
                                                  format,
                                                  sampleCnt,
                                                  isProtected,
                                                  memoryless,
                                                  /*label=*/"MakeMSAAAttachment");
    if (scratch) {
        return scratch;
    }

    return fGpu->makeMSAAAttachment(dimensions, format, sampleCnt, isProtected, memoryless);
}

sk_sp<GrAttachment> GrResourceProvider::refScratchMSAAAttachment(SkISize dimensions,
                                                                 const GrBackendFormat& format,
                                                                 int sampleCnt,
                                                                 GrProtected isProtected,
                                                                 GrMemoryless memoryless,
                                                                 std::string_view label) {
    ASSERT_SINGLE_OWNER
    SkASSERT(!this->isAbandoned());
    SkASSERT(!this->caps()->isFormatCompressed(format));
    SkASSERT(fCaps->validateSurfaceParams(dimensions,
                                          format,
                                          GrRenderable::kYes,
                                          sampleCnt,
                                          skgpu::Mipmapped::kNo,
                                          GrTextureType::kNone));

    skgpu::ScratchKey key;
    GrAttachment::ComputeScratchKey(*this->caps(),
                                    format,
                                    dimensions,
                                    GrAttachment::UsageFlags::kColorAttachment,
                                    sampleCnt,
                                    skgpu::Mipmapped::kNo,
                                    isProtected,
                                    memoryless,
                                    &key);
    GrGpuResource* resource = fCache->findAndRefScratchResource(key);
    if (resource) {
        fGpu->stats()->incNumScratchMSAAAttachmentsReused();
        GrAttachment* attachment = static_cast<GrAttachment*>(resource);
        resource->setLabel(std::move(label));
        return sk_sp<GrAttachment>(attachment);
    }

    return nullptr;
}

[[nodiscard]] std::unique_ptr<GrSemaphore> GrResourceProvider::makeSemaphore(bool isOwned) {
    return this->isAbandoned() ? nullptr : fGpu->makeSemaphore(isOwned);
}

std::unique_ptr<GrSemaphore> GrResourceProvider::wrapBackendSemaphore(
        const GrBackendSemaphore& semaphore,
        GrSemaphoreWrapType wrapType,
        GrWrapOwnership ownership) {
    ASSERT_SINGLE_OWNER
    return this->isAbandoned() ? nullptr : fGpu->wrapBackendSemaphore(semaphore,
                                                                      wrapType,
                                                                      ownership);
}

// Ensures the row bytes are populated (not 0) and makes a copy to a temporary
// to make the row bytes tight if necessary. Returns false if the input row bytes are invalid.
static bool prepare_level(const GrMipLevel& inLevel,
                          SkISize dimensions,
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
    size_t minRB = dimensions.fWidth * GrColorTypeBytesPerPixel(origColorType);
    size_t actualRB = inLevel.fRowBytes ? inLevel.fRowBytes : minRB;
    if (actualRB < minRB) {
        return false;
    }
    if (origColorType == allowedColorType && (actualRB == minRB || rowBytesSupport)) {
        outLevel->fRowBytes = actualRB;
        outLevel->fPixels = inLevel.fPixels;
        return true;
    }
    auto tempRB = dimensions.fWidth * GrColorTypeBytesPerPixel(allowedColorType);
    data->reset(new char[tempRB * dimensions.fHeight]);
    outLevel->fPixels = data->get();
    outLevel->fRowBytes = tempRB;
    GrImageInfo srcInfo(   origColorType, kUnpremul_SkAlphaType, nullptr, dimensions);
    GrImageInfo dstInfo(allowedColorType, kUnpremul_SkAlphaType, nullptr, dimensions);
    return GrConvertPixels( GrPixmap(dstInfo,     data->get(),   tempRB),
                           GrCPixmap(srcInfo, inLevel.fPixels, actualRB));
}

GrColorType GrResourceProvider::prepareLevels(const GrBackendFormat& format,
                                              GrColorType colorType,
                                              SkISize baseSize,
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
                                                 SkISize baseSize,
                                                 const GrMipLevel texels[],
                                                 int mipLevelCount) const {
    SkASSERT(!this->isAbandoned());
    SkASSERT(texture);
    SkASSERT(colorType != GrColorType::kUnknown);
    SkASSERT(mipLevelCount && texels && texels[0].fPixels);

    AutoSTArray<14, GrMipLevel> tmpTexels;
    AutoSTArray<14, std::unique_ptr<char[]>> tmpDatas;
    auto tempColorType = this->prepareLevels(texture->backendFormat(), colorType, baseSize, texels,
                                             mipLevelCount, &tmpTexels, &tmpDatas);
    if (tempColorType == GrColorType::kUnknown) {
        return nullptr;
    }
    SkAssertResult(fGpu->writePixels(texture.get(),
                                     SkIRect::MakeSize(baseSize),
                                     colorType,
                                     tempColorType,
                                     tmpTexels.get(),
                                     mipLevelCount));
    return texture;
}
