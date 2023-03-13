/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/dawn/GrDawnGpu.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkSLProgramKind.h"
#include "src/core/SkConvertPixels.h"
#include "src/gpu/ganesh/GrDataUtils.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGeometryProcessor.h"
#include "src/gpu/ganesh/GrGpuResourceCacheAccess.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrSemaphore.h"
#include "src/gpu/ganesh/GrStencilSettings.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/GrThreadSafePipelineBuilder.h"
#include "src/gpu/ganesh/dawn/GrDawnAsyncWait.h"
#include "src/gpu/ganesh/dawn/GrDawnAttachment.h"
#include "src/gpu/ganesh/dawn/GrDawnBuffer.h"
#include "src/gpu/ganesh/dawn/GrDawnCaps.h"
#include "src/gpu/ganesh/dawn/GrDawnOpsRenderPass.h"
#include "src/gpu/ganesh/dawn/GrDawnProgramBuilder.h"
#include "src/gpu/ganesh/dawn/GrDawnRenderTarget.h"
#include "src/gpu/ganesh/dawn/GrDawnTexture.h"
#include "src/gpu/ganesh/dawn/GrDawnUtil.h"
#include "src/sksl/SkSLProgramSettings.h"

#include "src/base/SkAutoMalloc.h"
#include "src/core/SkMipmap.h"
#include "src/sksl/SkSLCompiler.h"

#if !defined(SK_BUILD_FOR_WIN)
#include <unistd.h>
#endif // !defined(SK_BUILD_FOR_WIN)

static const int kMaxRenderPipelineEntries = 1024;

static wgpu::FilterMode to_dawn_filter_mode(GrSamplerState::Filter filter) {
    switch (filter) {
        case GrSamplerState::Filter::kNearest:
            return wgpu::FilterMode::Nearest;
        case GrSamplerState::Filter::kLinear:
            return wgpu::FilterMode::Linear;
        default:
            SkASSERT(!"unsupported filter mode");
            return wgpu::FilterMode::Nearest;
    }
}

static wgpu::FilterMode to_dawn_mipmap_mode(GrSamplerState::MipmapMode mode) {
    switch (mode) {
        case GrSamplerState::MipmapMode::kNone:
            // Fall-through (Dawn does not have an equivalent for "None")
        case GrSamplerState::MipmapMode::kNearest:
            return wgpu::FilterMode::Nearest;
        case GrSamplerState::MipmapMode::kLinear:
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
                             const GrContextOptions& options, GrDirectContext* direct) {
    if (!device) {
        return nullptr;
    }

    return sk_sp<GrGpu>(new GrDawnGpu(direct, options, device));
}

////////////////////////////////////////////////////////////////////////////////

GrDawnGpu::PendingMapAsyncRequests::PendingMapAsyncRequests(const wgpu::Device& device)
        : wait_(device) {}

void GrDawnGpu::PendingMapAsyncRequests::addOne() {
    if (fCount == 0) {
        wait_.reset();
    }
    fCount++;
}

void GrDawnGpu::PendingMapAsyncRequests::completeOne() {
    if (fCount == 1) {
        wait_.signal();
    }
    if (fCount > 0) {
        fCount--;
    }
}

void GrDawnGpu::PendingMapAsyncRequests::waitUntilDone() const {
    if (fCount == 0) {
        return;
    }
    wait_.busyWait();
    SkASSERT(fCount == 0);
}

GrDawnGpu::GrDawnGpu(GrDirectContext* direct,
                     const GrContextOptions& options,
                     const wgpu::Device& device)
        : INHERITED(direct)
        , fDevice(device)
        , fQueue(device.GetQueue())
        , fUniformRingBuffer(this, wgpu::BufferUsage::Uniform)
        , fStagingBufferManager(this)
        , fPendingMapAsyncRequests(device)
        , fRenderPipelineCache(kMaxRenderPipelineEntries)
        , fFinishCallbacks(this) {
    this->initCapsAndCompiler(sk_make_sp<GrDawnCaps>(options));
    device.SetUncapturedErrorCallback(
            [](WGPUErrorType type, char const* message, void*) {
                SkDebugf("GrDawnGpu: ERROR type %u, msg: %s", type, message);
            },
            nullptr);
}

GrDawnGpu::~GrDawnGpu() { this->finishOutstandingGpuWork(); }

void GrDawnGpu::disconnect(DisconnectType type) {
    if (DisconnectType::kCleanup == type) {
        this->finishOutstandingGpuWork();
    }
    fStagingBufferManager.reset();
    fQueue = nullptr;
    fDevice = nullptr;
    INHERITED::disconnect(type);
}

GrThreadSafePipelineBuilder* GrDawnGpu::pipelineBuilder() {
    return nullptr;
}

sk_sp<GrThreadSafePipelineBuilder> GrDawnGpu::refPipelineBuilder() {
    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

GrOpsRenderPass* GrDawnGpu::onGetOpsRenderPass(
        GrRenderTarget* rt,
        bool /*useMSAASurface*/,
        GrAttachment*,
        GrSurfaceOrigin origin,
        const SkIRect& bounds,
        const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
        const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
        const SkTArray<GrSurfaceProxy*, true>& sampledProxies,
        GrXferBarrierFlags renderPassXferBarriers) {
    fOpsRenderPass.reset(new GrDawnOpsRenderPass(this, rt, origin, colorInfo, stencilInfo));
    return fOpsRenderPass.get();
}

///////////////////////////////////////////////////////////////////////////////
sk_sp<GrGpuBuffer> GrDawnGpu::onCreateBuffer(size_t size,
                                             GrGpuBufferType type,
                                             GrAccessPattern accessPattern) {
    return GrDawnBuffer::Make(this, size, type, accessPattern,
                              /*label=*/"DawnGpu_GetOpsRenderPass");
}

////////////////////////////////////////////////////////////////////////////////
bool GrDawnGpu::onWritePixels(GrSurface* surface,
                              SkIRect rect,
                              GrColorType surfaceColorType,
                              GrColorType srcColorType,
                              const GrMipLevel texels[],
                              int mipLevelCount,
                              bool prepForTexSampling) {
    GrDawnTexture* texture = static_cast<GrDawnTexture*>(surface->asTexture());
    if (!texture) {
        return false;
    }
    this->uploadTextureData(srcColorType, texels, mipLevelCount, rect, texture->texture());
    if (mipLevelCount < texture->maxMipmapLevel() + 1) {
        texture->markMipmapsDirty();
    }
    return true;
}

bool GrDawnGpu::onTransferFromBufferToBuffer(sk_sp<GrGpuBuffer> src,
                                             size_t srcOffset,
                                             sk_sp<GrGpuBuffer> dst,
                                             size_t dstOffset,
                                             size_t size) {
    // skbug.com/13453
    SkASSERT(!"unimplemented");
    return false;
}

bool GrDawnGpu::onTransferPixelsTo(GrTexture* texture,
                                   SkIRect rect,
                                   GrColorType textureColorType,
                                   GrColorType bufferColorType,
                                   sk_sp<GrGpuBuffer> transferBuffer,
                                   size_t bufferOffset,
                                   size_t rowBytes) {
    // skbug.com/13453
    SkASSERT(!"unimplemented");
    return false;
}

bool GrDawnGpu::onTransferPixelsFrom(GrSurface* surface,
                                     SkIRect rect,
                                     GrColorType surfaceColorType,
                                     GrColorType bufferColorType,
                                     sk_sp<GrGpuBuffer> transferBuffer,
                                     size_t offset) {
    // skbug.com/13453
    SkASSERT(!"unimplemented");
    return false;
}

////////////////////////////////////////////////////////////////////////////////
sk_sp<GrTexture> GrDawnGpu::onCreateTexture(SkISize dimensions,
                                            const GrBackendFormat& backendFormat,
                                            GrRenderable renderable,
                                            int renderTargetSampleCnt,
                                            skgpu::Budgeted budgeted,
                                            GrProtected,
                                            int mipLevelCount,
                                            uint32_t levelClearMask,
                                            std::string_view label) {
    if (levelClearMask) {
        return nullptr;
    }

    wgpu::TextureFormat format;
    if (!backendFormat.asDawnFormat(&format)) {
        return nullptr;
    }

    GrMipmapStatus mipmapStatus =
        mipLevelCount > 1 ? GrMipmapStatus::kDirty : GrMipmapStatus::kNotAllocated;

    return GrDawnTexture::Make(this, dimensions, format, renderable, renderTargetSampleCnt,
                               budgeted, mipLevelCount, mipmapStatus, label);
}

sk_sp<GrTexture> GrDawnGpu::onCreateCompressedTexture(SkISize dimensions,
                                                      const GrBackendFormat&,
                                                      skgpu::Budgeted,
                                                      GrMipmapped,
                                                      GrProtected,
                                                      const void* data,
                                                      size_t dataSize) {
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
    return GrDawnTexture::MakeWrapped(this, dimensions, GrRenderable::kNo, 1, cacheable, ioType,
                                      info, backendTex.getLabel());
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

    sk_sp<GrTexture> result = GrDawnTexture::MakeWrapped(this, dimensions, GrRenderable::kYes,
                                                         sampleCnt, cacheable, kRW_GrIOType, info,
                                                         tex.getLabel());
    result->markMipmapsDirty();
    return result;
}

sk_sp<GrRenderTarget> GrDawnGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& rt) {
    GrDawnRenderTargetInfo info;
    if (!rt.getDawnRenderTargetInfo(&info) || !info.fTextureView) {
        return nullptr;
    }

    SkISize dimensions = { rt.width(), rt.height() };
    int sampleCnt = 1;
    return GrDawnRenderTarget::MakeWrapped(
            this, dimensions, sampleCnt, info, /*label=*/"DawnGpu_WrapBackendRenderTarget");
}

sk_sp<GrAttachment> GrDawnGpu::makeStencilAttachment(const GrBackendFormat& /*colorFormat*/,
                                                     SkISize dimensions, int numStencilSamples) {
    fStats.incStencilAttachmentCreates();
    return GrDawnAttachment::MakeStencil(this, dimensions, numStencilSamples);
}

GrBackendTexture GrDawnGpu::onCreateBackendTexture(SkISize dimensions,
                                                   const GrBackendFormat& backendFormat,
                                                   GrRenderable renderable,
                                                   GrMipmapped mipmapped,
                                                   GrProtected isProtected,
                                                   std::string_view label) {
    wgpu::TextureFormat format;
    if (!backendFormat.asDawnFormat(&format)) {
        return GrBackendTexture();
    }

    wgpu::TextureDescriptor desc;
    desc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                 wgpu::TextureUsage::CopyDst;

    if (GrRenderable::kYes == renderable) {
        desc.usage |= wgpu::TextureUsage::RenderAttachment;
    }

    int numMipLevels = 1;
    if (mipmapped == GrMipmapped::kYes) {
        numMipLevels = SkMipmap::ComputeLevelCount(dimensions.width(), dimensions.height()) + 1;
    }

    desc.size.width = dimensions.width();
    desc.size.height = dimensions.height();
    desc.size.depthOrArrayLayers = 1;
    desc.format = format;
    desc.mipLevelCount = numMipLevels;

    wgpu::Texture tex = this->device().CreateTexture(&desc);

    GrDawnTextureInfo info;
    info.fTexture = tex;
    info.fFormat = desc.format;
    info.fLevelCount = desc.mipLevelCount;
    return GrBackendTexture(dimensions.width(), dimensions.height(), info);
}

void GrDawnGpu::uploadTextureData(GrColorType srcColorType, const GrMipLevel texels[],
                                  int mipLevelCount, const SkIRect& rect,
                                  wgpu::Texture texture) {
    uint32_t x = rect.x();
    uint32_t y = rect.y();
    uint32_t width = rect.width();
    uint32_t height = rect.height();

    for (int i = 0; i < mipLevelCount; i++) {
        const void* src = texels[i].fPixels;
        size_t srcRowBytes = texels[i].fRowBytes;
        SkColorType colorType = GrColorTypeToSkColorType(srcColorType);
        size_t trimRowBytes = width * SkColorTypeBytesPerPixel(colorType);
        size_t dstRowBytes = GrDawnRoundRowBytes(trimRowBytes);
        size_t size = dstRowBytes * height;
        GrStagingBufferManager::Slice slice =
                this->stagingBufferManager()->allocateStagingBufferSlice(size);
        SkRectMemcpy(slice.fOffsetMapPtr, dstRowBytes, src, srcRowBytes, trimRowBytes, height);

        wgpu::ImageCopyBuffer srcBuffer = {};
        srcBuffer.buffer = static_cast<GrDawnBuffer*>(slice.fBuffer)->get();
        srcBuffer.layout.offset = slice.fOffset;
        srcBuffer.layout.bytesPerRow = dstRowBytes;
        srcBuffer.layout.rowsPerImage = height;

        wgpu::ImageCopyTexture dstTexture;
        dstTexture.texture = texture;
        dstTexture.mipLevel = i;
        dstTexture.origin = {x, y, 0};

        wgpu::Extent3D copySize = {width, height, 1};
        this->getCopyEncoder().CopyBufferToTexture(&srcBuffer, &dstTexture, &copySize);
        x /= 2;
        y /= 2;
        width = std::max(1u, width / 2);
        height = std::max(1u, height / 2);
    }
}

bool GrDawnGpu::onClearBackendTexture(const GrBackendTexture& backendTexture,
                                      sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                      std::array<float, 4> color) {
    GrDawnTextureInfo info;
    SkAssertResult(backendTexture.getDawnTextureInfo(&info));

    GrColorType colorType;
    if (!GrDawnFormatToGrColorType(info.fFormat, &colorType)) {
        return false;
    }

    size_t bpp = GrDawnBytesPerBlock(info.fFormat);
    size_t baseLayerSize = bpp * backendTexture.width() * backendTexture.height();
    SkAutoMalloc defaultStorage(baseLayerSize);
    GrImageInfo imageInfo(colorType, kUnpremul_SkAlphaType, nullptr, backendTexture.dimensions());
    GrClearImage(imageInfo, defaultStorage.get(), bpp * backendTexture.width(), color);

    wgpu::Device device = this->device();
    wgpu::CommandEncoder copyEncoder = this->getCopyEncoder();
    int w = backendTexture.width(), h = backendTexture.height();
    for (uint32_t i = 0; i < info.fLevelCount; i++) {
        size_t origRowBytes = bpp * w;
        size_t rowBytes = GrDawnRoundRowBytes(origRowBytes);
        size_t size = rowBytes * h;
        GrStagingBufferManager::Slice stagingBuffer =
                this->stagingBufferManager()->allocateStagingBufferSlice(size);
        if (rowBytes == origRowBytes) {
            memcpy(stagingBuffer.fOffsetMapPtr, defaultStorage.get(), size);
        } else {
            const char* src = static_cast<const char*>(defaultStorage.get());
            char* dst = static_cast<char*>(stagingBuffer.fOffsetMapPtr);
            for (int row = 0; row < h; row++) {
                memcpy(dst, src, origRowBytes);
                dst += rowBytes;
                src += origRowBytes;
            }
        }
        wgpu::ImageCopyBuffer srcBuffer = {};
        srcBuffer.buffer = static_cast<GrDawnBuffer*>(stagingBuffer.fBuffer)->get();
        srcBuffer.layout.offset = stagingBuffer.fOffset;
        srcBuffer.layout.bytesPerRow = rowBytes;
        srcBuffer.layout.rowsPerImage = h;
        wgpu::ImageCopyTexture dstTexture;
        dstTexture.texture = info.fTexture;
        dstTexture.mipLevel = i;
        dstTexture.origin = {0, 0, 0};
        wgpu::Extent3D copySize = {(uint32_t)w, (uint32_t)h, 1};
        copyEncoder.CopyBufferToTexture(&srcBuffer, &dstTexture, &copySize);
        w = std::max(1, w / 2);
        h = std::max(1, h / 2);
    }
    return true;
}

GrBackendTexture GrDawnGpu::onCreateCompressedBackendTexture(
        SkISize dimensions, const GrBackendFormat&, GrMipmapped, GrProtected) {
    return {};
}

bool GrDawnGpu::onUpdateCompressedBackendTexture(const GrBackendTexture&,
                                                 sk_sp<skgpu::RefCntedCallback> finishedCallback,
                                                 const void* data,
                                                 size_t size) {
    return false;
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

GrBackendRenderTarget GrDawnGpu::createTestingOnlyBackendRenderTarget(SkISize dimensions,
                                                                      GrColorType colorType,
                                                                      int sampleCnt,
                                                                      GrProtected isProtected) {
    if (dimensions.width()  > this->caps()->maxTextureSize() ||
        dimensions.height() > this->caps()->maxTextureSize()) {
        return {};
    }

    // We don't support MSAA in this backend yet.
    if (sampleCnt != 1) {
        return {};
    }

    if (isProtected == GrProtected::kYes) {
        return {};
    }

    wgpu::TextureFormat format;
    if (!GrColorTypeToDawnFormat(colorType, &format)) {
        return {};
    }

    wgpu::TextureDescriptor desc;
    desc.usage =
        wgpu::TextureUsage::CopySrc |
        wgpu::TextureUsage::RenderAttachment;

    desc.size.width = dimensions.width();
    desc.size.height = dimensions.height();
    desc.size.depthOrArrayLayers = 1;
    desc.format = format;

    wgpu::Texture tex = this->device().CreateTexture(&desc);

    GrDawnRenderTargetInfo info;
    info.fTextureView = tex.CreateView();
    info.fFormat = desc.format;
    info.fLevelCount = desc.mipLevelCount;

    return GrBackendRenderTarget(dimensions.width(), dimensions.height(), 1, 0, info);
}

void GrDawnGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    GrDawnRenderTargetInfo info;
    if (rt.getDawnRenderTargetInfo(&info)) {
        info.fTextureView = nullptr;
    }
}

#endif

void GrDawnGpu::addFinishedProc(GrGpuFinishedProc finishedProc,
                                GrGpuFinishedContext finishedContext) {
    fFinishCallbacks.add(finishedProc, finishedContext);
}

void GrDawnGpu::takeOwnershipOfBuffer(sk_sp<GrGpuBuffer> buffer) {
    fSubmittedStagingBuffers.push_back(std::move(buffer));
}

bool GrDawnGpu::onSubmitToGpu(bool syncCpu) {
    this->flushCopyEncoder();

    if (!fCommandBuffers.empty()) {
        fQueue.Submit(fCommandBuffers.size(), &fCommandBuffers.front());
        fCommandBuffers.clear();
    }

    // Schedule the queue done callback if it hasn't been scheduled already and if we just submitted
    // a new batch of recorded commands. If a callback was already registered in a prior call to
    // onSubmitToGpu then it will include the commands we just submitted.
    if (!fSubmittedWorkDoneCallbackPending) {
        auto callback = [](WGPUQueueWorkDoneStatus status, void* userData) {
            static_cast<GrDawnGpu*>(userData)->onSubmittedWorkDone(status);
        };
        fDevice.GetQueue().OnSubmittedWorkDone(0u, callback, this);
        fSubmittedWorkDoneCallbackPending = true;
    }

    this->mapPendingStagingBuffers();
    if (syncCpu) {
        // If no callback was scheduled then there is no pending work and we don't need to spin on a
        // fence.
        if (fSubmittedWorkDoneCallbackPending) {
            GrDawnAsyncWait* fence = this->createFence();
            fence->busyWait();
            this->destroyFence(fence);
        }
        fFinishCallbacks.callAll(true);
    }

    return true;
}

void GrDawnGpu::onSubmittedWorkDone(WGPUQueueWorkDoneStatus status) {
    fSubmittedWorkDoneCallbackPending = false;
    fQueueFences.foreach([](GrDawnAsyncWait* fence) {
        fence->signal();
    });
}

void GrDawnGpu::mapPendingStagingBuffers() {
    // Request to asynchronously map the submitted staging buffers. Dawn will ensure that these
    // buffers are not mapped until the pending submitted queue work is done at which point they
    // are free for re-use.
    for (unsigned i = 0; i < fSubmittedStagingBuffers.size(); i++) {
        fPendingMapAsyncRequests.addOne();
        sk_sp<GrGpuBuffer> buffer = std::move(fSubmittedStagingBuffers[i]);
        static_cast<GrDawnBuffer*>(buffer.get())
                ->mapAsync(
                        // We capture `buffer` into the callback which ensures that it stays alive
                        // until mapAsync completes.
                        [this, buffer = std::move(buffer)](bool success) {
                            fPendingMapAsyncRequests.completeOne();
                            if (!success) {
                                SkDebugf(
                                        "Failed to map staging buffer before making it available "
                                        "again");
                            }
                            // When this callback returns, the captured `buffer` will be dropped and
                            // returned back to its backing resource pool.
                        });
    }
    fSubmittedStagingBuffers.clear();
}

GrDawnAsyncWait* GrDawnGpu::createFence() {
    auto* fence = new GrDawnAsyncWait(fDevice);
    fQueueFences.add(fence);
    return fence;
}

void GrDawnGpu::destroyFence(GrDawnAsyncWait* fence) {
    fQueueFences.remove(fence);
    delete fence;
}

static wgpu::Texture get_dawn_texture_from_surface(GrSurface* src) {
    if (auto t = static_cast<GrDawnTexture*>(src->asTexture())) {
        return t->texture();
    } else {
        return nullptr;
    }
}

bool GrDawnGpu::onCopySurface(GrSurface* dst, const SkIRect& dstRect,
                              GrSurface* src, const SkIRect& srcRect,
                              GrSamplerState::Filter) {
    wgpu::Texture srcTexture = get_dawn_texture_from_surface(src);
    wgpu::Texture dstTexture = get_dawn_texture_from_surface(dst);
    if (!srcTexture || !dstTexture) {
        return false;
    }
    if (srcRect.size() != dstRect.size()) {
        return false;
    }

    uint32_t width = srcRect.width(), height = srcRect.height();

    wgpu::ImageCopyTexture srcTextureView, dstTextureView;
    srcTextureView.texture = srcTexture;
    srcTextureView.origin = {(uint32_t) srcRect.x(), (uint32_t) srcRect.y(), 0};
    dstTextureView.texture = dstTexture;
    dstTextureView.origin = {(uint32_t) dstRect.x(), (uint32_t) dstRect.y(), 0};

    wgpu::Extent3D copySize = {width, height, 1};
    this->getCopyEncoder().CopyTextureToTexture(&srcTextureView, &dstTextureView, &copySize);
    return true;
}

bool GrDawnGpu::onReadPixels(GrSurface* surface,
                             SkIRect rect,
                             GrColorType surfaceColorType,
                             GrColorType dstColorType,
                             void* buffer,
                             size_t rowBytes) {
    wgpu::Texture tex = get_dawn_texture_from_surface(surface);

    if (!tex || 0 == rowBytes) {
        return false;
    }
    size_t origRowBytes = rowBytes;
    int origSizeInBytes = origRowBytes*rect.height();
    rowBytes = GrDawnRoundRowBytes(rowBytes);
    int sizeInBytes = rowBytes*rect.height();

    sk_sp<GrDawnBuffer> dawnBuffer = GrDawnBuffer::Make(this,
                                                        sizeInBytes,
                                                        GrGpuBufferType::kXferGpuToCpu,
                                                        kStatic_GrAccessPattern,
                                                        "onReadPixels");
    if (!dawnBuffer) {
        SkDebugf("onReadPixels: failed to create GPU buffer");
        return false;
    }

    wgpu::ImageCopyTexture srcTexture;
    srcTexture.texture = tex;
    srcTexture.origin = {(uint32_t) rect.left(), (uint32_t) rect.top(), 0};

    wgpu::ImageCopyBuffer dstBuffer = {};
    dstBuffer.buffer = dawnBuffer->get();
    dstBuffer.layout.offset = 0;
    dstBuffer.layout.bytesPerRow = rowBytes;
    dstBuffer.layout.rowsPerImage = rect.height();

    wgpu::Extent3D copySize = {(uint32_t) rect.width(), (uint32_t) rect.height(), 1};
    this->getCopyEncoder().CopyTextureToBuffer(&srcTexture, &dstBuffer, &copySize);
    this->submitToGpu(true);

    const void* readPixelsPtr = dawnBuffer->map();
    if (!readPixelsPtr) {
        SkDebugf("onReadPixels: failed to map GPU buffer");
        return false;
    }

    if (rowBytes == origRowBytes) {
        memcpy(buffer, readPixelsPtr, origSizeInBytes);
    } else {
        const char* src = static_cast<const char*>(readPixelsPtr);
        char* dst = static_cast<char*>(buffer);
        for (int row = 0; row < rect.height(); row++) {
            memcpy(dst, src, origRowBytes);
            dst += origRowBytes;
            src += rowBytes;
        }
    }

    dawnBuffer->unmap();
    return true;
}

bool GrDawnGpu::onRegenerateMipMapLevels(GrTexture* tex) {
    this->flushCopyEncoder();
    GrDawnTexture* src = static_cast<GrDawnTexture*>(tex);
    int srcWidth = tex->width();
    int srcHeight = tex->height();

    // SkMipmap doesn't include the base level in the level count so we have to add 1
    uint32_t levelCount = SkMipmap::ComputeLevelCount(tex->width(), tex->height()) + 1;

    // Create a temporary texture for mipmap generation, then copy to source.
    // We have to do this even for renderable textures, since GrDawnRenderTarget currently only
    // contains a view, not a texture.
    wgpu::TextureDescriptor texDesc;
    texDesc.usage = wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
                    wgpu::TextureUsage::RenderAttachment;
    texDesc.size.width = (tex->width() + 1) / 2;
    texDesc.size.height = (tex->height() + 1) / 2;
    texDesc.size.depthOrArrayLayers = 1;
    texDesc.mipLevelCount = levelCount - 1;
    texDesc.format = src->format();
    wgpu::Texture dstTexture = fDevice.CreateTexture(&texDesc);

    const char* vs =
        "layout(spirv, location = 0) out float2 texCoord;"
        "float2 positions[4] = float2[4](float2(-1.0, 1.0),"
                                        "float2(1.0, 1.0),"
                                        "float2(-1.0, -1.0),"
                                        "float2(1.0, -1.0));"
        "float2 texCoords[4] = float2[4](float2(0.0, 0.0),"
                                        "float2(1.0, 0.0),"
                                        "float2(0.0, 1.0),"
                                        "float2(1.0, 1.0));"
        "void main() {"
            "sk_Position = float4(positions[sk_VertexID], 0.0, 1.0);"
            "texCoord = texCoords[sk_VertexID];"
        "}";
    std::string vsSPIRV = this->SkSLToSPIRV(vs,
                                            SkSL::ProgramKind::kVertex,
                                            /*rtFlipOffset*/ 0,
                                            nullptr);

    const char* fs =
        "layout(spirv, set = 0, binding = 0) uniform sampler samp;"
        "layout(spirv, set = 0, binding = 1) uniform texture2D tex;"
        "layout(location = 0) in float2 texCoord;"
        "void main() {"
            "sk_FragColor = sample(makeSampler2D(tex, samp), texCoord);"
        "}";
    std::string fsSPIRV = this->SkSLToSPIRV(fs,
                                            SkSL::ProgramKind::kFragment,
                                            /*rtFlipOffset=*/ 0,
                                            nullptr);

    wgpu::VertexState vertexState;
    vertexState.module = this->createShaderModule(vsSPIRV);
    vertexState.entryPoint = "main";
    vertexState.bufferCount = 0;

    wgpu::ColorTargetState colorTargetState;
    colorTargetState.format = static_cast<GrDawnTexture*>(tex)->format();

    wgpu::FragmentState fragmentState;
    fragmentState.module = this->createShaderModule(fsSPIRV);
    fragmentState.entryPoint = "main";
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTargetState;

    wgpu::RenderPipelineDescriptor renderPipelineDesc;
    renderPipelineDesc.vertex = vertexState;
    renderPipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleStrip;
    renderPipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Uint16;
    renderPipelineDesc.fragment = &fragmentState;
    wgpu::RenderPipeline pipeline = fDevice.CreateRenderPipeline(&renderPipelineDesc);

    wgpu::BindGroupLayout bgl = pipeline.GetBindGroupLayout(0);
    wgpu::TextureViewDescriptor srcViewDesc;
    srcViewDesc.mipLevelCount = 1;
    wgpu::TextureView srcView = src->texture().CreateView(&srcViewDesc);
    wgpu::SamplerDescriptor samplerDesc;
    samplerDesc.minFilter = wgpu::FilterMode::Linear;
    wgpu::Sampler sampler = fDevice.CreateSampler(&samplerDesc);
    wgpu::CommandEncoder commandEncoder = fDevice.CreateCommandEncoder();
    for (uint32_t mipLevel = 0; mipLevel < texDesc.mipLevelCount; mipLevel++) {
        int dstWidth = std::max(1, srcWidth / 2);
        int dstHeight = std::max(1, srcHeight / 2);
        wgpu::TextureViewDescriptor dstViewDesc;
        dstViewDesc.format = static_cast<GrDawnTexture*>(tex)->format();
        dstViewDesc.dimension = wgpu::TextureViewDimension::e2D;
        dstViewDesc.baseMipLevel = mipLevel;
        dstViewDesc.mipLevelCount = 1;
        wgpu::TextureView dstView = dstTexture.CreateView(&dstViewDesc);
        wgpu::BindGroupEntry bge[2];
        bge[0].binding = 0;
        bge[0].sampler = sampler;
        bge[1].binding = 1;
        bge[1].textureView = srcView;
        wgpu::BindGroupDescriptor bgDesc;
        bgDesc.layout = bgl;
        bgDesc.entryCount = 2;
        bgDesc.entries = bge;
        wgpu::BindGroup bindGroup = fDevice.CreateBindGroup(&bgDesc);
        wgpu::RenderPassColorAttachment colorAttachment;
        colorAttachment.view = dstView;
        colorAttachment.clearValue = {0.0f, 0.0f, 0.0f, 0.0f};
        colorAttachment.loadOp = wgpu::LoadOp::Load;
        colorAttachment.storeOp = wgpu::StoreOp::Store;
        wgpu::RenderPassColorAttachment* colorAttachments = { &colorAttachment };
        wgpu::RenderPassDescriptor renderPassDesc;
        renderPassDesc.colorAttachmentCount = 1;
        renderPassDesc.colorAttachments = colorAttachments;
        wgpu::RenderPassEncoder rpe = commandEncoder.BeginRenderPass(&renderPassDesc);
        rpe.SetPipeline(pipeline);
        rpe.SetBindGroup(0, bindGroup);
        rpe.Draw(4, 1, 0, 0);
        rpe.End();

        wgpu::Extent3D copySize = {(uint32_t)dstWidth, (uint32_t)dstHeight, 1};
        wgpu::ImageCopyTexture srcCopyView;
        srcCopyView.texture = dstTexture;
        srcCopyView.mipLevel = mipLevel;
        wgpu::ImageCopyTexture dstCopyView;
        dstCopyView.mipLevel = mipLevel + 1;
        dstCopyView.texture = src->texture();
        commandEncoder.CopyTextureToTexture(&srcCopyView, &dstCopyView, &copySize);

        srcHeight = dstHeight;
        srcWidth = dstWidth;
        srcView = dstView;
    }
    fCommandBuffers.push_back(commandEncoder.Finish());
    return true;
}

void GrDawnGpu::submit(GrOpsRenderPass* renderPass) {
    this->flushCopyEncoder();
    static_cast<GrDawnOpsRenderPass*>(renderPass)->submit();
}

GrFence SK_WARN_UNUSED_RESULT GrDawnGpu::insertFence() {
    return reinterpret_cast<GrFence>(this->createFence());
}

bool GrDawnGpu::waitFence(GrFence fence) {
    return reinterpret_cast<const GrDawnAsyncWait*>(fence)->yieldAndCheck();
}

void GrDawnGpu::deleteFence(GrFence fence) {
    this->destroyFence(reinterpret_cast<GrDawnAsyncWait*>(fence));
}

std::unique_ptr<GrSemaphore> SK_WARN_UNUSED_RESULT GrDawnGpu::makeSemaphore(bool isOwned) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

std::unique_ptr<GrSemaphore> GrDawnGpu::wrapBackendSemaphore(const GrBackendSemaphore& /* sema */,
                                                             GrSemaphoreWrapType /* wrapType */,
                                                             GrWrapOwnership /* ownership */) {
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
    fFinishCallbacks.check();
}

void GrDawnGpu::finishOutstandingGpuWork() {
    // If a callback is pending then any fence added here is guaranteed to get signaled when the
    // callback eventually runs.
    if (fSubmittedWorkDoneCallbackPending) {
        GrDawnAsyncWait* fence = this->createFence();
        fence->busyWait();
        this->destroyFence(fence);
    }

    // Make sure all pending mapAsync requests on staging buffers are complete before shutting down.
    fPendingMapAsyncRequests.waitUntilDone();
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
    bool hasDepthStencil = rt->getStencilAttachment() != nullptr;

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
    desc.maxAnisotropy = samplerState.maxAniso();
    if (samplerState.isAniso()) {
        // WebGPU requires these to be linear when maxAnisotropy is > 1.
        desc.magFilter = desc.minFilter = desc.mipmapFilter = wgpu::FilterMode::Linear;
    } else {
        desc.magFilter = desc.minFilter = to_dawn_filter_mode(samplerState.filter());
        desc.mipmapFilter = to_dawn_mipmap_mode(samplerState.mipmapMode());
    }
    wgpu::Sampler sampler = device().CreateSampler(&desc);
    fSamplers.insert(std::pair<GrSamplerState, wgpu::Sampler>(samplerState, sampler));
    return sampler;
}

GrDawnRingBuffer::Slice GrDawnGpu::allocateUniformRingBufferSlice(int size) {
    return fUniformRingBuffer.allocate(size);
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

std::string GrDawnGpu::SkSLToSPIRV(const char* shaderString,
                                   SkSL::ProgramKind kind,
                                   uint32_t rtFlipOffset,
                                   SkSL::Program::Inputs* inputs) {
    auto errorHandler = this->getContext()->priv().getShaderErrorHandler();
    SkSL::ProgramSettings settings;
    settings.fRTFlipOffset = rtFlipOffset;
    settings.fRTFlipBinding = 0;
    settings.fRTFlipSet = 0;
    std::unique_ptr<SkSL::Program> program = this->shaderCompiler()->convertProgram(
        kind,
        shaderString,
        settings);
    if (!program) {
        errorHandler->compileError(shaderString, this->shaderCompiler()->errorText().c_str());
        return "";
    }
    if (inputs) {
        *inputs = program->fInputs;
    }
    std::string code;
    if (!this->shaderCompiler()->toSPIRV(*program, &code)) {
        errorHandler->compileError(shaderString, this->shaderCompiler()->errorText().c_str());
        return "";
    }
    return code;
}

wgpu::ShaderModule GrDawnGpu::createShaderModule(const std::string& spirvSource) {
    wgpu::ShaderModuleSPIRVDescriptor desc;
    desc.codeSize = spirvSource.size() / 4;
    desc.code = reinterpret_cast<const uint32_t*>(spirvSource.c_str());

    wgpu::ShaderModuleDescriptor smDesc;
    smDesc.nextInChain = &desc;

    return fDevice.CreateShaderModule(&smDesc);
}
