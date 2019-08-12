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
        , fCompiler(new SkSL::Compiler()) {
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
    fCachedRTCommandBuffer.reset(
        new GrDawnGpuRTCommandBuffer(this, rt, origin, colorInfo, stencilInfo));
    return fCachedRTCommandBuffer.get();
}

GrGpuTextureCommandBuffer* GrDawnGpu::getCommandBuffer(GrTexture* texture,
                                                       GrSurfaceOrigin origin) {
    fCachedTextureCommandBuffer.reset(
        new GrDawnGpuTextureCommandBuffer(this, texture, origin));
    return fCachedTextureCommandBuffer.get();
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
                                            const GrBackendFormat& format,
                                            GrRenderable renderable,
                                            int renderTargetSampleCnt,
                                            SkBudgeted budgeted,
                                            GrProtected,
                                            const GrMipLevel texels[],
                                            int mipLevelCount) {
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

    sk_sp<GrDawnTexture> tex = GrDawnTexture::Make(this, desc, renderable, renderTargetSampleCnt,
                                                   budgeted, mipLevelCount, mipMapsStatus);
    if (!tex) {
        return nullptr;
    }
    tex->upload(texels, mipLevelCount);
    return tex;
}

sk_sp<GrTexture> GrDawnGpu::onCreateCompressedTexture(int width, int height,
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
    if (!info.fTexture) {
        return nullptr;
    }

    GrPixelConfig config = this->caps()->getConfigFromBackendFormat(backendTex.getBackendFormat(),
                                                                    colorType);
    GrSurfaceDesc desc;
    desc.fWidth = backendTex.width();
    desc.fHeight = backendTex.height();
    desc.fConfig = config;

    GrMipMapsStatus status = GrMipMapsStatus::kNotAllocated;
    return GrDawnTexture::MakeWrapped(this, desc, status, cacheable, info);
}

sk_sp<GrTexture> GrDawnGpu::onWrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                           int sampleCnt, GrColorType,
                                                           GrWrapOwnership,
                                                           GrWrapCacheable cacheable) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrRenderTarget> GrDawnGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget&,
                                                           GrColorType colorType) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrRenderTarget> GrDawnGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                    int sampleCnt,
                                                                    GrColorType colorType) {
    GrDawnImageInfo info;
    if (!tex.getDawnImageInfo(&info)) {
        return nullptr;
    }
    if (!info.fTexture) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = this->caps()->getConfigFromBackendFormat(tex.getBackendFormat(), colorType);
    sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, tex.getBackendFormat());
    if (sampleCnt < 1) {
        return nullptr;
    }

    return GrDawnRenderTarget::MakeWrapped(this, desc, sampleCnt, info);
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
                                                 const GrBackendFormat& format,
                                                 GrMipMapped mipMapped,
                                                 GrRenderable renderable,
                                                 const void* pixels,
                                                 size_t rowBytes,
                                                 const SkColor4f* color,
                                                 GrProtected isProtected) {
    SkASSERT(!"unimplemented");
    return GrBackendTexture();
}

void GrDawnGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(!"unimplemented");
}

#if GR_TEST_UTILS
bool GrDawnGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(!"unimplemented");
    return false;
}

GrBackendRenderTarget GrDawnGpu::createTestingOnlyBackendRenderTarget(int w, int h, GrColorType) {
    SkASSERT(!"unimplemented");
    return GrBackendRenderTarget();
}

void GrDawnGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) {
    SkASSERT(!"unimplemented");
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
    auto gpu = reinterpret_cast<GrDawnGpu*>(userdata);
    gpu->setReadPixelsPtr(data);
}

bool GrDawnGpu::onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                             GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                             size_t rowBytes) {
    dawn::Texture tex = static_cast<GrDawnTexture*>(surface->asTexture())->texture();

    size_t origRowBytes = rowBytes;
    int origSizeInBytes = origRowBytes * height;
    rowBytes = (rowBytes + 0xFF) & ~0xFF;
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
    buf.MapReadAsync(callback, this);
    while (!fReadPixelsPtr) {
        device().Tick();
    }
    if (rowBytes == origRowBytes) {
        memcpy(buffer, fReadPixelsPtr, origSizeInBytes);
    } else {
        const char* src = static_cast<const char*>(fReadPixelsPtr);
        char* dst = static_cast<char*>(buffer);
        for (int row = 0; row < height; row++) {
            memcpy(dst, src, origRowBytes);
            dst += origRowBytes;
            src += rowBytes;
        }
    }
    fReadPixelsPtr = nullptr;
    buf.Unmap();
    return true;
}

bool GrDawnGpu::onRegenerateMipMapLevels(GrTexture*) {
    SkASSERT(!"unimplemented");
    return false;
}

void GrDawnGpu::submit(GrGpuCommandBuffer* buffer) {
    if (buffer->asRTCommandBuffer()) {
        SkASSERT(fCachedRTCommandBuffer.get() == buffer);
        fCachedRTCommandBuffer->submit();
    } else {
        SkASSERT(fCachedTextureCommandBuffer.get() == buffer);
        fCachedTextureCommandBuffer->submit();
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
