/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/dawn/GrDawnGpu.h"

#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpuResourceCacheAccess.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/dawn/GrDawnBuffer.h"
#include "src/gpu/dawn/GrDawnCaps.h"
#include "src/gpu/dawn/GrDawnGpuCommandBuffer.h"
#include "src/gpu/dawn/GrDawnRenderTarget.h"
#include "src/gpu/dawn/GrDawnStencilAttachment.h"
#include "src/gpu/dawn/GrDawnTexture.h"
#include "src/gpu/dawn/GrDawnUtil.h"

#include "src/core/SkAutoMalloc.h"
#include "src/core/SkMipMap.h"
#include "src/sksl/SkSLCompiler.h"

#if !defined(SK_BUILD_FOR_WIN)
#include <unistd.h>
#endif // !defined(SK_BUILD_FOR_WIN)

sk_sp<GrGpu> GrDawnGpu::Make(const dawn::Device& device,
                             const GrContextOptions& options, GrContext* context) {
    if (!device) {
        return nullptr;
    }

    return sk_sp<GrGpu>(new GrDawnGpu(context, options, device));
}

////////////////////////////////////////////////////////////////////////////////

GrDawnGpu::GrDawnGpu(GrContext* context, const GrContextOptions& options,
                     const dawn::Device& device)
        : INHERITED(context)
        , fDevice(device)
        , fQueue(device.CreateQueue())
        , fCompiler(new SkSL::Compiler())
        , fUniformRingBuffer(this, dawn::BufferUsageBit::Uniform) {
    fCaps.reset(new GrDawnCaps(options));
}

GrDawnGpu::~GrDawnGpu() {
}


void GrDawnGpu::disconnect(DisconnectType type) {
    SkASSERT(!"unimplemented");
}

///////////////////////////////////////////////////////////////////////////////

GrGpuRTCommandBuffer* GrDawnGpu::getCommandBuffer(
            GrRenderTarget* rt, GrSurfaceOrigin origin, const SkRect& bounds,
            const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo) {
    fRTCommandBuffer.reset(new GrDawnGpuRTCommandBuffer(this, rt, origin, colorInfo, stencilInfo));
    return fRTCommandBuffer.get();
}

GrGpuTextureCommandBuffer* GrDawnGpu::getCommandBuffer(GrTexture* texture,
                                                       GrSurfaceOrigin origin) {
    fTextureCommandBuffer.reset(new GrDawnGpuTextureCommandBuffer(this, texture, origin));
    return fTextureCommandBuffer.get();
}

///////////////////////////////////////////////////////////////////////////////
sk_sp<GrGpuBuffer> GrDawnGpu::onCreateBuffer(size_t size, GrGpuBufferType type,
                                             GrAccessPattern accessPattern, const void* data) {
    sk_sp<GrGpuBuffer> b(new GrDawnBuffer(this, size, type, accessPattern));
    if (data && b) {
        b->updateData(data, size);
    }
    return b;
}

////////////////////////////////////////////////////////////////////////////////
bool GrDawnGpu::onWritePixels(GrSurface* surface, int left, int top, int width, int height,
                              GrColorType textureColorType, GrColorType bufferColorType,
                              const GrMipLevel texels[], int mipLevelCount) {
    GrDawnTexture* texture = static_cast<GrDawnTexture*>(surface->asTexture());
    if (!texture) {
        SkASSERT(!"uploading to non-texture unimplemented");
        return false;
    }
    texture->upload(texels, mipLevelCount, SkIRect::MakeXYWH(left, top, width, height));
    return true;
}

bool GrDawnGpu::onTransferPixelsTo(GrTexture* texture, int left, int top, int width, int height,
                                   GrColorType textureColorType, GrColorType bufferColorType,
                                   GrGpuBuffer* transferBuffer, size_t bufferOffset,
                                   size_t rowBytes) {
    SkASSERT(!"unimplemented");
    return false;
}

bool GrDawnGpu::onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                                     GrColorType surfaceColorType, GrColorType bufferColorType,
                                     GrGpuBuffer* transferBuffer, size_t offset) {
    SkASSERT(!"unimplemented");
    return false;
}

////////////////////////////////////////////////////////////////////////////////
sk_sp<GrTexture> GrDawnGpu::onCreateTexture(const GrSurfaceDesc& desc,
                                            const GrBackendFormat& backendFormat,
                                            GrRenderable renderable,
                                            int renderTargetSampleCnt,
                                            SkBudgeted budgeted,
                                            GrProtected,
                                            const GrMipLevel texels[],
                                            int mipLevelCount) {
    dawn::TextureFormat format;
    if (!backendFormat.asDawnFormat(&format)) {
        return nullptr;
    }

    GrMipMapsStatus mipMapsStatus = GrMipMapsStatus::kNotAllocated;
    if (mipLevelCount > 1) {
        mipMapsStatus = GrMipMapsStatus::kValid;
        for (int i = 0; i < mipLevelCount; ++i) {
            if (!texels[i].fPixels) {
                mipMapsStatus = GrMipMapsStatus::kDirty;
                break;
            }
        }
    }

    sk_sp<GrDawnTexture> tex = GrDawnTexture::Make(this, { desc.fWidth, desc.fHeight },
                                                   desc.fConfig, format, renderable,
                                                   renderTargetSampleCnt, budgeted, mipLevelCount,
                                                   mipMapsStatus);
    if (!tex) {
        return nullptr;
    }
    tex->upload(texels, mipLevelCount);
    return tex;
}

sk_sp<GrTexture> GrDawnGpu::onCreateCompressedTexture(int width, int height, const GrBackendFormat&,
                                                      SkImage::CompressionType, SkBudgeted,
                                                      const void* data) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrTexture> GrDawnGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                                 GrColorType colorType,
                                                 GrWrapOwnership ownership,
                                                 GrWrapCacheable cacheable,
                                                 GrIOType) {
    GrDawnImageInfo info;
    if (!backendTex.getDawnImageInfo(&info)) {
        return nullptr;
    }

    SkISize size = { backendTex.width(), backendTex.height() };
    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(backendTex.getBackendFormat(),
                                                                    colorType);
    GrMipMapsStatus status = GrMipMapsStatus::kNotAllocated;
    return GrDawnTexture::MakeWrapped(this, size, config, GrRenderable::kNo, 1, status, cacheable,
                                      info);
}

sk_sp<GrTexture> GrDawnGpu::onWrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                           int sampleCnt, GrColorType colorType,
                                                           GrWrapOwnership,
                                                           GrWrapCacheable cacheable) {
    GrDawnImageInfo info;
    if (!tex.getDawnImageInfo(&info) || !info.fTexture) {
        return nullptr;
    }

    SkISize size = { tex.width(), tex.height() };
    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(tex.getBackendFormat(),
                                                                    colorType);
    sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, tex.getBackendFormat());
    if (sampleCnt < 1) {
        return nullptr;
    }

    GrMipMapsStatus status = GrMipMapsStatus::kNotAllocated;
    return GrDawnTexture::MakeWrapped(this, size, config, GrRenderable::kYes, sampleCnt, status,
                                      cacheable, info);
}

sk_sp<GrRenderTarget> GrDawnGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& rt,
                                                           GrColorType colorType) {
    GrDawnImageInfo info;
    if (!rt.getDawnImageInfo(&info) && !info.fTexture) {
        return nullptr;
    }

    SkISize size = { rt.width(), rt.height() };
    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(rt.getBackendFormat(),
                                                                    colorType);
    int sampleCnt = 1;
    return GrDawnRenderTarget::MakeWrapped(this, size, config, sampleCnt, info);
}

sk_sp<GrRenderTarget> GrDawnGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                    int sampleCnt,
                                                                    GrColorType colorType) {
    GrDawnImageInfo info;
    if (!tex.getDawnImageInfo(&info) || !info.fTexture) {
        return nullptr;
    }

    SkISize size = { tex.width(), tex.height() };
    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(tex.getBackendFormat(),
                                                                    colorType);
    sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, tex.getBackendFormat());
    if (sampleCnt < 1) {
        return nullptr;
    }

    return GrDawnRenderTarget::MakeWrapped(this, size, config, sampleCnt, info);
}

GrStencilAttachment* GrDawnGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                       int width,
                                                                       int height,
                                                                       int numStencilSamples) {
    GrDawnStencilAttachment* stencil(GrDawnStencilAttachment::Create(this,
                                                                     width,
                                                                     height,
                                                                     numStencilSamples));
    fStats.incStencilAttachmentCreates();
    return stencil;
}

GrBackendTexture GrDawnGpu::createBackendTexture(int width, int height,
                                                 const GrBackendFormat& backendFormat,
                                                 GrMipMapped mipMapped,
                                                 GrRenderable renderable,
                                                 const void* pixels,
                                                 size_t rowBytes,
                                                 const SkColor4f* color,
                                                 GrProtected isProtected) {
    dawn::TextureFormat format;
    if (!backendFormat.asDawnFormat(&format)) {
        return GrBackendTexture();
    }

    GrPixelConfig config = GrDawnFormatToPixelConfig(format);

    if (width > this->caps()->maxTextureSize() || height > this->caps()->maxTextureSize()) {
        return GrBackendTexture();
    }

    // FIXME: Dawn doesn't support mipmapped render targets (yet).
    if (GrMipMapped::kYes == mipMapped && GrRenderable::kYes == renderable) {
        return GrBackendTexture();
    }

    dawn::TextureDescriptor desc;
    desc.usage =
        dawn::TextureUsageBit::Sampled |
        dawn::TextureUsageBit::CopySrc |
        dawn::TextureUsageBit::CopyDst;

    if (GrRenderable::kYes == renderable) {
        desc.usage |= dawn::TextureUsageBit::OutputAttachment;
    }

    desc.size.width = width;
    desc.size.height = height;
    desc.size.depth = 1;
    desc.format = format;

    // Figure out the number of mip levels.
    if (GrMipMapped::kYes == mipMapped) {
        desc.mipLevelCount = SkMipMap::ComputeLevelCount(width, height) + 1;
    }

    dawn::Texture tex = this->device().CreateTexture(&desc);

    size_t bpp = GrBytesPerPixel(config);
    size_t baseLayerSize = bpp * width * height;
    SkAutoMalloc defaultStorage(baseLayerSize);
    if (!pixels) {
        // Fill in the texture with all zeros so we don't have random garbage
        pixels = defaultStorage.get();
        memset(defaultStorage.get(), 0, baseLayerSize);
    }
    dawn::Device device = this->device();
    dawn::CommandEncoder copyEncoder = fDevice.CreateCommandEncoder();
    int w = width, h = height;
    for (uint32_t i = 0; i < desc.mipLevelCount; i++) {
        size_t origRowBytes = bpp * w;
        size_t rowBytes = GrDawnRoundRowBytes(origRowBytes);
        size_t size = rowBytes * h;
        dawn::BufferDescriptor bufferDesc;
        bufferDesc.size = size;
        bufferDesc.usage = dawn::BufferUsageBit::CopySrc | dawn::BufferUsageBit::CopyDst;
        dawn::Buffer buffer = this->device().CreateBuffer(&bufferDesc);
        const uint8_t* src = static_cast<const uint8_t*>(pixels);
        if (rowBytes == origRowBytes) {
            buffer.SetSubData(0, size, src);
        } else {
            uint32_t offset = 0;
            for (int row = 0; row < h; row++) {
                buffer.SetSubData(offset, origRowBytes, src);
                offset += rowBytes;
                src += origRowBytes;
            }
        }
        dawn::BufferCopyView srcBuffer;
        srcBuffer.buffer = buffer;
        srcBuffer.offset = 0;
        srcBuffer.rowPitch = rowBytes;
        srcBuffer.imageHeight = h;
        dawn::TextureCopyView dstTexture;
        dstTexture.texture = tex;
        dstTexture.mipLevel = i;
        dstTexture.origin = {0, 0, 0};
        dawn::Extent3D copySize = {(uint32_t) w, (uint32_t) h, 1};
        copyEncoder.CopyBufferToTexture(&srcBuffer, &dstTexture, &copySize);
        w = SkTMax(1, w / 2);
        h = SkTMax(1, h / 2);
    }
    dawn::CommandBuffer cmdBuf = copyEncoder.Finish();
    fQueue.Submit(1, &cmdBuf);
    GrDawnImageInfo info;
    info.fTexture = tex;
    info.fFormat = desc.format;
    info.fLevelCount = desc.mipLevelCount;
    return GrBackendTexture(width, height, info);
}

void GrDawnGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    GrDawnImageInfo info;
    if (tex.getDawnImageInfo(&info)) {
        info.fTexture = nullptr;
    }
}

#if GR_TEST_UTILS
bool GrDawnGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    GrDawnImageInfo info;
    if (!tex.getDawnImageInfo(&info)) {
        return false;
    }

    return info.fTexture.Get();
}

GrBackendRenderTarget GrDawnGpu::createTestingOnlyBackendRenderTarget(int width, int height,
                                                                      GrColorType colorType) {
    GrPixelConfig config = GrColorTypeToPixelConfig(colorType);

    if (width > this->caps()->maxTextureSize() || height > this->caps()->maxTextureSize()) {
        return GrBackendRenderTarget();
    }

    dawn::TextureFormat format;
    if (!GrPixelConfigToDawnFormat(config, &format)) {
        return GrBackendRenderTarget();
    }

    dawn::TextureDescriptor desc;
    desc.usage =
        dawn::TextureUsageBit::CopySrc |
        dawn::TextureUsageBit::OutputAttachment;

    desc.size.width = width;
    desc.size.height = height;
    desc.size.depth = 1;
    desc.format = format;

    dawn::Texture tex = this->device().CreateTexture(&desc);

    GrDawnImageInfo info;
    info.fTexture = tex;
    info.fFormat = desc.format;
    info.fLevelCount = desc.mipLevelCount;
    return GrBackendRenderTarget(width, height, 1, 0, info);
}

void GrDawnGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    GrDawnImageInfo info;
    if (rt.getDawnImageInfo(&info)) {
        info.fTexture = nullptr;
    }
}

void GrDawnGpu::testingOnly_flushGpuAndSync() {
    SkASSERT(!"unimplemented");
}

#endif

void GrDawnGpu::onFinishFlush(GrSurfaceProxy*[], int n, SkSurface::BackendSurfaceAccess access,
                              const GrFlushInfo& info, const GrPrepareForExternalIORequests&) {
    SkASSERT(!"unimplemented");
}

bool GrDawnGpu::onCopySurface(GrSurface* dst,
                              GrSurface* src,
                              const SkIRect& srcRect,
                              const SkIPoint& dstPoint,
                              bool canDiscardOutsideDstRect) {
    SkASSERT(!"unimplemented");
    return false;
}

static void callback(DawnBufferMapAsyncStatus status, const void* data, uint64_t dataLength,
                     void* userdata) {
    (*reinterpret_cast<const void**>(userdata)) = data;
}

bool GrDawnGpu::onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                             GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                             size_t rowBytes) {
    dawn::Texture tex;
    if (auto rt = static_cast<GrDawnRenderTarget*>(surface->asRenderTarget())) {
        tex = rt->texture();
    } else if (auto t = static_cast<GrDawnTexture*>(surface->asTexture())) {
        tex = t->texture();
    } else {
        return false;
    }

    if (0 == rowBytes) {
        return false;
    }
    size_t origRowBytes = rowBytes;
    int origSizeInBytes = origRowBytes * height;
    rowBytes = GrDawnRoundRowBytes(rowBytes);
    int sizeInBytes = rowBytes * height;

    dawn::BufferDescriptor desc;
    desc.usage = dawn::BufferUsageBit::CopyDst | dawn::BufferUsageBit::MapRead;
    desc.size = sizeInBytes;

    dawn::Buffer buf = device().CreateBuffer(&desc);

    dawn::TextureCopyView srcTexture;
    srcTexture.texture = tex;
    srcTexture.origin = {(uint32_t) left, (uint32_t) top, 0};

    dawn::BufferCopyView dstBuffer;
    dstBuffer.buffer = buf;
    dstBuffer.offset = 0;
    dstBuffer.rowPitch = rowBytes;
    dstBuffer.imageHeight = height;

    dawn::Extent3D copySize = {(uint32_t) width, (uint32_t) height, 1};
    auto encoder = device().CreateCommandEncoder();
    encoder.CopyTextureToBuffer(&srcTexture, &dstBuffer, &copySize);
    auto commandBuffer = encoder.Finish();
    queue().Submit(1, &commandBuffer);

    const void *readPixelsPtr = nullptr;
    buf.MapReadAsync(callback, &readPixelsPtr);
    while (!readPixelsPtr) {
        device().Tick();
    }

    if (rowBytes == origRowBytes) {
        memcpy(buffer, readPixelsPtr, origSizeInBytes);
    } else {
        const char* src = static_cast<const char*>(readPixelsPtr);
        char* dst = static_cast<char*>(buffer);
        for (int row = 0; row < height; row++) {
            memcpy(dst, src, origRowBytes);
            dst += origRowBytes;
            src += rowBytes;
        }
    }
    buf.Unmap();
    return true;
}

bool GrDawnGpu::onRegenerateMipMapLevels(GrTexture*) {
    SkASSERT(!"unimplemented");
    return false;
}

void GrDawnGpu::submit(GrGpuCommandBuffer* buffer) {
    if (buffer->asRTCommandBuffer()) {
        SkASSERT(fRTCommandBuffer.get() == buffer);
        fRTCommandBuffer->submit();
    } else {
        SkASSERT(fTextureCommandBuffer.get() == buffer);
        fTextureCommandBuffer->submit();
    }
}

GrFence SK_WARN_UNUSED_RESULT GrDawnGpu::insertFence() {
    SkASSERT(!"unimplemented");
    return GrFence();
}

bool GrDawnGpu::waitFence(GrFence fence, uint64_t timeout) {
    SkASSERT(!"unimplemented");
    return false;
}

void GrDawnGpu::deleteFence(GrFence fence) const {
    SkASSERT(!"unimplemented");
}

sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT GrDawnGpu::makeSemaphore(bool isOwned) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrSemaphore> GrDawnGpu::wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                                   GrResourceProvider::SemaphoreWrapType wrapType,
                                                   GrWrapOwnership ownership) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

void GrDawnGpu::insertSemaphore(sk_sp<GrSemaphore> semaphore) {
    SkASSERT(!"unimplemented");
}

void GrDawnGpu::waitSemaphore(sk_sp<GrSemaphore> semaphore) {
    SkASSERT(!"unimplemented");
}

void GrDawnGpu::checkFinishProcs() {
    SkASSERT(!"unimplemented");
}

sk_sp<GrSemaphore> GrDawnGpu::prepareTextureForCrossContextUsage(GrTexture* texture) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

GrDawnRingBuffer::Slice GrDawnGpu::allocateUniformRingBufferSlice(int size) {
    return fUniformRingBuffer.allocate(size);
}
