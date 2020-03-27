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
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/dawn/GrDawnBuffer.h"
#include "src/gpu/dawn/GrDawnCaps.h"
#include "src/gpu/dawn/GrDawnOpsRenderPass.h"
#include "src/gpu/dawn/GrDawnProgramBuilder.h"
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

const int kMaxRenderPipelineEntries = 1024;

static wgpu::FilterMode to_dawn_filter_mode(GrSamplerState::Filter filter) {
    switch (filter) {
        case GrSamplerState::Filter::kNearest:
            return wgpu::FilterMode::Nearest;
        case GrSamplerState::Filter::kBilerp:
        case GrSamplerState::Filter::kMipMap:
            return wgpu::FilterMode::Linear;
        default:
            SkASSERT(!"unsupported filter mode");
            return wgpu::FilterMode::Nearest;
    }
}

static wgpu::AddressMode to_dawn_address_mode(GrSamplerState::WrapMode wrapMode) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            return wgpu::AddressMode::ClampToEdge;
        case GrSamplerState::WrapMode::kRepeat:
            return wgpu::AddressMode::Repeat;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            return wgpu::AddressMode::MirrorRepeat;
        case GrSamplerState::WrapMode::kClampToBorder:
            SkASSERT(!"unsupported address mode");
    }
    SkASSERT(!"unsupported address mode");
    return wgpu::AddressMode::ClampToEdge;

}

sk_sp<GrGpu> GrDawnGpu::Make(const wgpu::Device& device,
                             const GrContextOptions& options, GrContext* context) {
    if (!device) {
        return nullptr;
    }

    return sk_sp<GrGpu>(new GrDawnGpu(context, options, device));
}

////////////////////////////////////////////////////////////////////////////////

GrDawnGpu::GrDawnGpu(GrContext* context, const GrContextOptions& options,
                     const wgpu::Device& device)
        : INHERITED(context)
        , fDevice(device)
        , fQueue(device.CreateQueue())
        , fCompiler(new SkSL::Compiler())
        , fUniformRingBuffer(this, wgpu::BufferUsage::Uniform)
        , fRenderPipelineCache(kMaxRenderPipelineEntries)
        , fStagingManager(fDevice) {
    fCaps.reset(new GrDawnCaps(options));
}

GrDawnGpu::~GrDawnGpu() {
}


void GrDawnGpu::disconnect(DisconnectType type) {
    SkASSERT(!"unimplemented");
}

///////////////////////////////////////////////////////////////////////////////

GrOpsRenderPass* GrDawnGpu::getOpsRenderPass(
            GrRenderTarget* rt, GrSurfaceOrigin origin, const SkIRect& bounds,
            const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
            const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
            const SkTArray<GrSurfaceProxy*, true>& sampledProxies) {
    fOpsRenderPass.reset(new GrDawnOpsRenderPass(this, rt, origin, colorInfo, stencilInfo));
    return fOpsRenderPass.get();
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
                              GrColorType surfaceColorType, GrColorType srcColorType,
                              const GrMipLevel texels[], int mipLevelCount,
                              bool prepForTexSampling) {
    GrDawnTexture* texture = static_cast<GrDawnTexture*>(surface->asTexture());
    if (!texture) {
        return false;
    }
    texture->upload(srcColorType, texels, mipLevelCount,
                    SkIRect::MakeXYWH(left, top, width, height), this->getCopyEncoder());
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
sk_sp<GrTexture> GrDawnGpu::onCreateTexture(SkISize dimensions,
                                            const GrBackendFormat& backendFormat,
                                            GrRenderable renderable,
                                            int renderTargetSampleCnt,
                                            SkBudgeted budgeted,
                                            GrProtected,
                                            int mipLevelCount,
                                            uint32_t levelClearMask) {
    SkASSERT(!levelClearMask);
    wgpu::TextureFormat format;
    if (!backendFormat.asDawnFormat(&format)) {
        return nullptr;
    }

    GrMipMapsStatus mipMapsStatus =
        mipLevelCount > 1 ? GrMipMapsStatus::kDirty : GrMipMapsStatus::kNotAllocated;

    return GrDawnTexture::Make(this, dimensions, format, renderable, renderTargetSampleCnt,
                               budgeted, mipLevelCount, mipMapsStatus);
}

sk_sp<GrTexture> GrDawnGpu::onCreateCompressedTexture(SkISize dimensions, const GrBackendFormat&,
                                                      SkBudgeted, GrMipMapped, GrProtected,
                                                      const void* data, size_t dataSize) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrTexture> GrDawnGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                                 GrWrapOwnership ownership,
                                                 GrWrapCacheable cacheable,
                                                 GrIOType ioType) {
    GrDawnTextureInfo info;
    if (!backendTex.getDawnTextureInfo(&info)) {
        return nullptr;
    }

    SkISize dimensions = { backendTex.width(), backendTex.height() };
    GrMipMapsStatus status = GrMipMapsStatus::kNotAllocated;
    return GrDawnTexture::MakeWrapped(this, dimensions, GrRenderable::kNo, 1, status, cacheable,
                                      ioType, info);
}

sk_sp<GrTexture> GrDawnGpu::onWrapCompressedBackendTexture(const GrBackendTexture& backendTex,
                                                           GrWrapOwnership ownership,
                                                           GrWrapCacheable cacheable) {
    return nullptr;
}

sk_sp<GrTexture> GrDawnGpu::onWrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                           int sampleCnt,
                                                           GrWrapOwnership,
                                                           GrWrapCacheable cacheable) {
    GrDawnTextureInfo info;
    if (!tex.getDawnTextureInfo(&info) || !info.fTexture) {
        return nullptr;
    }

    SkISize dimensions = { tex.width(), tex.height() };
    sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, tex.getBackendFormat());
    if (sampleCnt < 1) {
        return nullptr;
    }

    GrMipMapsStatus status = GrMipMapsStatus::kNotAllocated;
    return GrDawnTexture::MakeWrapped(this, dimensions, GrRenderable::kYes, sampleCnt, status,
                                      cacheable, kRW_GrIOType, info);
}

sk_sp<GrRenderTarget> GrDawnGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& rt) {
    GrDawnRenderTargetInfo info;
    if (!rt.getDawnRenderTargetInfo(&info) || !info.fTextureView) {
        return nullptr;
    }

    SkISize dimensions = { rt.width(), rt.height() };
    int sampleCnt = 1;
    return GrDawnRenderTarget::MakeWrapped(this, dimensions, sampleCnt, info);
}

sk_sp<GrRenderTarget> GrDawnGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                    int sampleCnt) {
    GrDawnTextureInfo textureInfo;
    if (!tex.getDawnTextureInfo(&textureInfo) || !textureInfo.fTexture) {
        return nullptr;
    }

    SkISize dimensions = { tex.width(), tex.height() };
    sampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, tex.getBackendFormat());
    if (sampleCnt < 1) {
        return nullptr;
    }

    GrDawnRenderTargetInfo info(textureInfo);
    return GrDawnRenderTarget::MakeWrapped(this, dimensions, sampleCnt, info);
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

GrBackendTexture GrDawnGpu::onCreateBackendTexture(SkISize dimensions,
                                                   const GrBackendFormat& backendFormat,
                                                   GrRenderable renderable,
                                                   GrMipMapped mipMapped,
                                                   GrProtected isProtected,
                                                   const BackendTextureData* data) {
    wgpu::TextureFormat format;
    if (!backendFormat.asDawnFormat(&format)) {
        return GrBackendTexture();
    }

    // FIXME: Dawn doesn't support mipmapped render targets (yet).
    if (mipMapped == GrMipMapped::kYes && GrRenderable::kYes == renderable) {
        return GrBackendTexture();
    }

    wgpu::TextureDescriptor desc;
    desc.usage =
        wgpu::TextureUsage::Sampled |
        wgpu::TextureUsage::CopySrc |
        wgpu::TextureUsage::CopyDst;

    if (GrRenderable::kYes == renderable) {
        desc.usage |= wgpu::TextureUsage::OutputAttachment;
    }

    int numMipLevels = 1;
    if (mipMapped == GrMipMapped::kYes) {
        numMipLevels = SkMipMap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    desc.size.width = dimensions.width();
    desc.size.height = dimensions.height();
    desc.size.depth = 1;
    desc.format = format;
    desc.mipLevelCount = numMipLevels;

    wgpu::Texture tex = this->device().CreateTexture(&desc);

    size_t bpp = GrDawnBytesPerPixel(format);
    size_t baseLayerSize = bpp * dimensions.width() * dimensions.height();
    const void* pixels;
    SkAutoMalloc defaultStorage(baseLayerSize);
    if (data && data->type() == BackendTextureData::Type::kPixmaps) {
        pixels = data->pixmap(0).addr();
    } else {
        pixels = defaultStorage.get();
        memset(defaultStorage.get(), 0, baseLayerSize);
    }
    wgpu::Device device = this->device();
    wgpu::CommandEncoder copyEncoder = fDevice.CreateCommandEncoder();
    int w = dimensions.width(), h = dimensions.height();
    for (uint32_t i = 0; i < desc.mipLevelCount; i++) {
        size_t origRowBytes = bpp * w;
        size_t rowBytes = GrDawnRoundRowBytes(origRowBytes);
        size_t size = rowBytes * h;
        GrDawnStagingBuffer* stagingBuffer = this->getStagingBuffer(size);
        if (rowBytes == origRowBytes) {
            memcpy(stagingBuffer->fData, pixels, size);
        } else {
            const char* src = static_cast<const char*>(pixels);
            char* dst = static_cast<char*>(stagingBuffer->fData);
            for (int row = 0; row < h; row++) {
                memcpy(dst, src, origRowBytes);
                dst += rowBytes;
                src += origRowBytes;
            }
        }
        wgpu::Buffer buffer = stagingBuffer->fBuffer;
        buffer.Unmap();
        stagingBuffer->fData = nullptr;
        wgpu::BufferCopyView srcBuffer;
        srcBuffer.buffer = buffer;
        srcBuffer.offset = 0;
        srcBuffer.rowPitch = rowBytes;
        srcBuffer.imageHeight = h;
        wgpu::TextureCopyView dstTexture;
        dstTexture.texture = tex;
        dstTexture.mipLevel = i;
        dstTexture.origin = {0, 0, 0};
        wgpu::Extent3D copySize = {(uint32_t) w, (uint32_t) h, 1};
        copyEncoder.CopyBufferToTexture(&srcBuffer, &dstTexture, &copySize);
        w = std::max(1, w / 2);
        h = std::max(1, h / 2);
    }
    wgpu::CommandBuffer cmdBuf = copyEncoder.Finish();
    fQueue.Submit(1, &cmdBuf);
    GrDawnTextureInfo info;
    info.fTexture = tex;
    info.fFormat = desc.format;
    info.fLevelCount = desc.mipLevelCount;
    return GrBackendTexture(dimensions.width(), dimensions.height(), info);
}

GrBackendTexture GrDawnGpu::onCreateCompressedBackendTexture(SkISize dimensions,
                                                             const GrBackendFormat&,
                                                             GrMipMapped,
                                                             GrProtected,
                                                             const BackendTextureData*) {
    return {};
}

void GrDawnGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    GrDawnTextureInfo info;
    if (tex.getDawnTextureInfo(&info)) {
        info.fTexture = nullptr;
    }
}

bool GrDawnGpu::compile(const GrProgramDesc&, const GrProgramInfo&) {
    return false;
}

#if GR_TEST_UTILS
bool GrDawnGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    GrDawnTextureInfo info;
    if (!tex.getDawnTextureInfo(&info)) {
        return false;
    }

    return info.fTexture.Get();
}

GrBackendRenderTarget GrDawnGpu::createTestingOnlyBackendRenderTarget(int width, int height,
                                                                      GrColorType colorType) {

    if (width > this->caps()->maxTextureSize() || height > this->caps()->maxTextureSize()) {
        return GrBackendRenderTarget();
    }

    wgpu::TextureFormat format;
    if (!GrColorTypeToDawnFormat(colorType, &format)) {
        return GrBackendRenderTarget();
    }

    wgpu::TextureDescriptor desc;
    desc.usage =
        wgpu::TextureUsage::CopySrc |
        wgpu::TextureUsage::OutputAttachment;

    desc.size.width = width;
    desc.size.height = height;
    desc.size.depth = 1;
    desc.format = format;

    wgpu::Texture tex = this->device().CreateTexture(&desc);

    GrDawnRenderTargetInfo info;
    info.fTextureView = tex.CreateView();
    info.fFormat = desc.format;
    info.fLevelCount = desc.mipLevelCount;
    return GrBackendRenderTarget(width, height, 1, 0, info);
}

void GrDawnGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    GrDawnRenderTargetInfo info;
    if (rt.getDawnRenderTargetInfo(&info)) {
        info.fTextureView = nullptr;
    }
}

void GrDawnGpu::testingOnly_flushGpuAndSync() {
    this->flush();
}

#endif

void GrDawnGpu::flush() {
    fUniformRingBuffer.flush();
    this->flushCopyEncoder();
    if (!fCommandBuffers.empty()) {
        fQueue.Submit(fCommandBuffers.size(), &fCommandBuffers.front());
        fCommandBuffers.clear();
    }
    fStagingManager.mapBusyList();
    fDevice.Tick();
}

bool GrDawnGpu::onFinishFlush(GrSurfaceProxy*[], int n, SkSurface::BackendSurfaceAccess access,
                              const GrFlushInfo& info, const GrPrepareForExternalIORequests&) {
    this->flush();
    return true;
}

static wgpu::Texture get_dawn_texture_from_surface(GrSurface* src) {
    if (auto t = static_cast<GrDawnTexture*>(src->asTexture())) {
        return t->texture();
    } else {
        return nullptr;
    }
}

bool GrDawnGpu::onCopySurface(GrSurface* dst,
                              GrSurface* src,
                              const SkIRect& srcRect,
                              const SkIPoint& dstPoint) {
    wgpu::Texture srcTexture = get_dawn_texture_from_surface(src);
    wgpu::Texture dstTexture = get_dawn_texture_from_surface(dst);
    if (!srcTexture || !dstTexture) {
        return false;
    }

    uint32_t width = srcRect.width(), height = srcRect.height();

    wgpu::TextureCopyView srcTextureView, dstTextureView;
    srcTextureView.texture = srcTexture;
    srcTextureView.origin = {(uint32_t) srcRect.x(), (uint32_t) srcRect.y(), 0};
    dstTextureView.texture = dstTexture;
    dstTextureView.origin = {(uint32_t) dstPoint.x(), (uint32_t) dstPoint.y(), 0};

    wgpu::Extent3D copySize = {width, height, 1};
    this->getCopyEncoder().CopyTextureToTexture(&srcTextureView, &dstTextureView, &copySize);
    return true;
}

static void callback(WGPUBufferMapAsyncStatus status, const void* data, uint64_t dataLength,
                     void* userdata) {
    (*reinterpret_cast<const void**>(userdata)) = data;
}

bool GrDawnGpu::onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                             GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                             size_t rowBytes) {
    wgpu::Texture tex = get_dawn_texture_from_surface(surface);
    SkASSERT(tex);

    if (0 == rowBytes) {
        return false;
    }
    size_t origRowBytes = rowBytes;
    int origSizeInBytes = origRowBytes * height;
    rowBytes = GrDawnRoundRowBytes(rowBytes);
    int sizeInBytes = rowBytes * height;

    wgpu::BufferDescriptor desc;
    desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    desc.size = sizeInBytes;

    wgpu::Buffer buf = device().CreateBuffer(&desc);

    wgpu::TextureCopyView srcTexture;
    srcTexture.texture = tex;
    srcTexture.origin = {(uint32_t) left, (uint32_t) top, 0};

    wgpu::BufferCopyView dstBuffer;
    dstBuffer.buffer = buf;
    dstBuffer.offset = 0;
    dstBuffer.rowPitch = rowBytes;
    dstBuffer.imageHeight = height;

    wgpu::Extent3D copySize = {(uint32_t) width, (uint32_t) height, 1};
    this->getCopyEncoder().CopyTextureToBuffer(&srcTexture, &dstBuffer, &copySize);
    flush();

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

void GrDawnGpu::submit(GrOpsRenderPass* renderPass) {
    this->flushCopyEncoder();
    static_cast<GrDawnOpsRenderPass*>(renderPass)->submit();
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

std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT GrDawnGpu::makeSemaphore(bool isOwned) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

std::unique_ptr<GrSemaphore> GrDawnGpu::wrapBackendSemaphore(
        const GrBackendSemaphore& semaphore,
        GrResourceProvider::SemaphoreWrapType wrapType,
        GrWrapOwnership ownership) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

void GrDawnGpu::insertSemaphore(GrSemaphore* semaphore) {
    SkASSERT(!"unimplemented");
}

void GrDawnGpu::waitSemaphore(GrSemaphore* semaphore) {
    SkASSERT(!"unimplemented");
}

void GrDawnGpu::checkFinishProcs() {
    SkASSERT(!"unimplemented");
}

std::unique_ptr<GrSemaphore> GrDawnGpu::prepareTextureForCrossContextUsage(GrTexture* texture) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrDawnProgram> GrDawnGpu::getOrCreateRenderPipeline(
        GrRenderTarget* rt,
        const GrProgramInfo& programInfo) {

    GrProgramDesc desc = this->caps()->makeDesc(rt, programInfo);
    if (!desc.isValid()) {
        return nullptr;
    }

    if (sk_sp<GrDawnProgram>* program = fRenderPipelineCache.find(desc)) {
        return *program;
    }

    wgpu::TextureFormat colorFormat;
    SkAssertResult(programInfo.backendFormat().asDawnFormat(&colorFormat));

    wgpu::TextureFormat stencilFormat = wgpu::TextureFormat::Depth24PlusStencil8;
    bool hasDepthStencil = rt->renderTargetPriv().getStencilAttachment() != nullptr;

    sk_sp<GrDawnProgram> program = GrDawnProgramBuilder::Build(
        this, rt, programInfo, colorFormat,
        hasDepthStencil, stencilFormat, &desc);
    fRenderPipelineCache.insert(desc, program);
    return program;
}

wgpu::Sampler GrDawnGpu::getOrCreateSampler(GrSamplerState samplerState) {
    auto i = fSamplers.find(samplerState);
    if (i != fSamplers.end()) {
        return i->second;
    }
    wgpu::SamplerDescriptor desc;
    desc.addressModeU = to_dawn_address_mode(samplerState.wrapModeX());
    desc.addressModeV = to_dawn_address_mode(samplerState.wrapModeY());
    desc.addressModeW = wgpu::AddressMode::ClampToEdge;
    desc.magFilter = desc.minFilter = to_dawn_filter_mode(samplerState.filter());
    desc.mipmapFilter = wgpu::FilterMode::Linear;
    desc.lodMinClamp = 0.0f;
    desc.lodMaxClamp = 1000.0f;
    desc.compare = wgpu::CompareFunction::Never;
    wgpu::Sampler sampler = device().CreateSampler(&desc);
    fSamplers.insert(std::pair<GrSamplerState, wgpu::Sampler>(samplerState, sampler));
    return sampler;
}

GrDawnRingBuffer::Slice GrDawnGpu::allocateUniformRingBufferSlice(int size) {
    return fUniformRingBuffer.allocate(size);
}

GrDawnStagingBuffer* GrDawnGpu::getStagingBuffer(size_t size) {
    return fStagingManager.findOrCreateStagingBuffer(size);
}

void GrDawnGpu::appendCommandBuffer(wgpu::CommandBuffer commandBuffer) {
    if (commandBuffer) {
        fCommandBuffers.push_back(commandBuffer);
    }
}

wgpu::CommandEncoder GrDawnGpu::getCopyEncoder() {
    if (!fCopyEncoder) {
        fCopyEncoder = fDevice.CreateCommandEncoder();
    }
    return fCopyEncoder;
}

void GrDawnGpu::flushCopyEncoder() {
    if (fCopyEncoder) {
        fCommandBuffers.push_back(fCopyEncoder.Finish());
        fCopyEncoder = nullptr;
    }
}
