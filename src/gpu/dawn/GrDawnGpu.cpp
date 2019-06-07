/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnGpu.h"

#include "include/gpu/GrBackendSemaphore.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrGeometryProcessor.h"
#include "src/gpu/GrGpuResourceCacheAccess.h"
#include "src/gpu/GrMesh.h"
#include "src/gpu/dawn/GrDawnBuffer.h"
#include "src/gpu/dawn/GrDawnCaps.h"
#include "src/gpu/dawn/GrDawnGpuCommandBuffer.h"
#include "src/gpu/dawn/GrDawnProgramBuilder.h"
#include "src/gpu/dawn/GrDawnRenderTarget.h"
#include "src/gpu/dawn/GrDawnStencilAttachment.h"
#include "src/gpu/dawn/GrDawnTexture.h"
#include "src/gpu/GrPipeline.h"
#include "src/gpu/GrStencilSettings.h"
#include "src/gpu/dawn/GrDawnUtil.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrSemaphore.h"
#include "src/gpu/GrTexturePriv.h"

#include "src/core/SkAutoMalloc.h"
#include "src/core/SkMipMap.h"
#include "src/sksl/SkSLCompiler.h"

#if !defined(SK_BUILD_FOR_WIN)
#include <unistd.h>
#endif // !defined(SK_BUILD_FOR_WIN)

const int kMaxRenderPipelineEntries = 1024;

namespace {

dawn::FilterMode to_dawn_filter_mode(GrSamplerState::Filter filter) {
    switch (filter) {
        case GrSamplerState::Filter::kNearest:
            return dawn::FilterMode::Nearest;
        case GrSamplerState::Filter::kBilerp:
        case GrSamplerState::Filter::kMipMap:
            return dawn::FilterMode::Linear;
        default:
            SkASSERT(!"unsupported filter mode");
            return dawn::FilterMode::Nearest;
    }
}

dawn::AddressMode to_dawn_address_mode(GrSamplerState::WrapMode wrapMode) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            return dawn::AddressMode::ClampToEdge;
        case GrSamplerState::WrapMode::kRepeat:
            return dawn::AddressMode::Repeat;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            return dawn::AddressMode::MirroredRepeat;
        case GrSamplerState::WrapMode::kClampToBorder:
            SkASSERT(!"unsupported address mode");
    }
    SkASSERT(!"unsupported address mode");
    return dawn::AddressMode::ClampToEdge;

}

// FIXME: taken from GrVkPipelineState; refactor.
uint32_t get_blend_info_key(const GrPipeline& pipeline) {
    GrXferProcessor::BlendInfo blendInfo;
    pipeline.getXferProcessor().getBlendInfo(&blendInfo);

    static const uint32_t kBlendWriteShift = 1;
    static const uint32_t kBlendCoeffShift = 5;
    GR_STATIC_ASSERT(kLast_GrBlendCoeff < (1 << kBlendCoeffShift));
    GR_STATIC_ASSERT(kFirstAdvancedGrBlendEquation - 1 < 4);

    uint32_t key = blendInfo.fWriteColor;
    key |= (blendInfo.fSrcBlend << kBlendWriteShift);
    key |= (blendInfo.fDstBlend << (kBlendWriteShift + kBlendCoeffShift));
    key |= (blendInfo.fEquation << (kBlendWriteShift + 2 * kBlendCoeffShift));

    return key;
}

};

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
        , fUniformRingBuffer(this, dawn::BufferUsageBit::Uniform)
        , fCopyEncoder(fDevice.CreateCommandEncoder())
        , fRenderPipelineCache(kMaxRenderPipelineEntries)
        , fStagingManager(fDevice) {
    fCompiler = new SkSL::Compiler();
    fCaps.reset(new GrDawnCaps(options));
    // This will be filled by the copy command buffer.
    fCommandBuffers.push_back(nullptr);
}

GrDawnGpu::~GrDawnGpu() {
    delete fCompiler;
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
    return new GrDawnGpuTextureCommandBuffer(this, texture, origin);
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
bool GrDawnGpu::onWritePixels(GrSurface* surface,
                              int left, int top, int width, int height,
                              GrColorType colorType,
                              const GrMipLevel texels[], int mipLevelCount) {
    GrDawnTexture* texture = (GrDawnTexture*) surface->asTexture();
    if (!texture) {
        SkASSERT(!"uploading to non-texture unimplemented");
        return false;
    }
    texture->upload(texels, mipLevelCount, SkIRect::MakeXYWH(left, top, width, height),
                    fCopyEncoder);
    return true;
}

bool GrDawnGpu::onTransferPixelsTo(GrTexture* texture,
                                  int left, int top, int width, int height,
                                  GrColorType colorType, GrGpuBuffer* transferBuffer,
                                  size_t bufferOffset, size_t rowBytes) {
    SkASSERT(!"unimplemented");
    return false;
}

bool GrDawnGpu::onTransferPixelsFrom(GrSurface* surface, int left, int top, int width, int height,
                                     GrColorType, GrGpuBuffer* transferBuffer, size_t offset) {
    SkASSERT(!"unimplemented");
    return false;
}

////////////////////////////////////////////////////////////////////////////////

sk_sp<GrTexture> GrDawnGpu::onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                            const GrMipLevel texels[], int mipLevels) {
    GrMipMapsStatus mipMapsStatus = GrMipMapsStatus::kNotAllocated;
    if (mipLevels > 1) {
        mipMapsStatus = GrMipMapsStatus::kValid;
        for (int i = 0; i < mipLevels; ++i) {
            if (!texels[i].fPixels) {
                mipMapsStatus = GrMipMapsStatus::kDirty;
                break;
            }
        }
    }

    sk_sp<GrDawnTexture> tex = GrDawnTexture::Make(this, desc, budgeted, mipMapsStatus);
    if (!tex) {
        return nullptr;
    }
    tex->upload(texels, mipLevels, fCopyEncoder);
    return tex;
}

sk_sp<GrTexture> GrDawnGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
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

    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = backendTex.width();
    desc.fHeight = backendTex.height();
    desc.fConfig = backendTex.config();
    desc.fSampleCnt = 1;

    GrMipMapsStatus status = GrMipMapsStatus::kNotAllocated;
    sk_sp<GrDawnTexture> tgt = GrDawnTexture::MakeWrapped(this, desc, status, cacheable, info);
    return tgt;
}

sk_sp<GrTexture> GrDawnGpu::onWrapRenderableBackendTexture(const GrBackendTexture& tex,
                                                           int sampleCnt,
                                                           GrWrapOwnership,
                                                           GrWrapCacheable cacheable) {
    GrDawnImageInfo info;
    if (!tex.getDawnImageInfo(&info)) {
        return nullptr;
    }
    if (!info.fTexture) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = tex.config();
    desc.fSampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, tex.config());
    if (desc.fSampleCnt < 1) {
        return nullptr;
    }

    GrMipMapsStatus status = GrMipMapsStatus::kNotAllocated;
    return GrDawnTexture::MakeWrapped(this, desc, status, cacheable, info);
}

sk_sp<GrRenderTarget> GrDawnGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget& rt) {
    GrDawnImageInfo info;
    if (!rt.getDawnImageInfo(&info)) {
        return nullptr;
    }
    if (!info.fTexture) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = rt.width();
    desc.fHeight = rt.height();
    desc.fConfig = rt.config();
    desc.fSampleCnt = 1;

    sk_sp<GrDawnRenderTarget> tgt = GrDawnRenderTarget::MakeWrapped(this, desc, info);
    return tgt;
}

sk_sp<GrRenderTarget> GrDawnGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                    int sampleCnt) {
    GrDawnImageInfo info;
    if (!tex.getDawnImageInfo(&info)) {
        return nullptr;
    }
    if (!info.fTexture) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = tex.config();
    desc.fSampleCnt = this->caps()->getRenderTargetSampleCount(sampleCnt, tex.config());
    if (desc.fSampleCnt < 1) {
        return nullptr;
    }

    return GrDawnRenderTarget::MakeWrapped(this, desc, info);
}

GrStencilAttachment* GrDawnGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                       int width,
                                                                       int height) {
    SkASSERT(width >= rt->width());
    SkASSERT(height >= rt->height());

    int samples = rt->numStencilSamples();

    GrDawnStencilAttachment* stencil(GrDawnStencilAttachment::Create(this,
                                                                   width,
                                                                   height,
                                                                   samples));
    fStats.incStencilAttachmentCreates();
    return stencil;
}

GrBackendTexture GrDawnGpu::createBackendTexture(int width, int height,
                                                 const GrBackendFormat& backendFormat,
                                                 GrMipMapped mipMapped,
                                                 GrRenderable renderable,
                                                 const void* pixels,
                                                 size_t rowBytes,
                                                 const SkColor4f& color) {
    const dawn::TextureFormat* format = backendFormat.getDawnFormat();

    if (!format) {
        return GrBackendTexture();
    }

    GrPixelConfig config = GrDawnFormatToPixelConfig(*format);

    if (!this->caps()->isConfigTexturable(config)) {
        return GrBackendTexture();
    }

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
        dawn::TextureUsageBit::TransferSrc |
        dawn::TextureUsageBit::TransferDst;

    if (GrRenderable::kYes == renderable) {
        desc.usage |= dawn::TextureUsageBit::OutputAttachment;
    }

    desc.dimension = dawn::TextureDimension::e2D;
    desc.size.width = width;
    desc.size.height = height;
    desc.size.depth = 1;
    desc.arrayLayerCount = 1;
    desc.format = *format;
    desc.sampleCount = 1;

    // Figure out the number of mip levels.
    if (GrMipMapped::kYes == mipMapped) {
        desc.mipLevelCount = SkMipMap::ComputeLevelCount(width, height) + 1;
    } else {
        desc.mipLevelCount = 1;
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
        size_t rowBytes = origRowBytes;
        if ((rowBytes & 0xFF) != 0) {
            rowBytes = (rowBytes + 0xFF) & ~0xFF;
        }
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
        dawn::Buffer buffer = stagingBuffer->fBuffer;
        buffer.Unmap();
        stagingBuffer->fData = nullptr;
        dawn::BufferCopyView srcBuffer;
        srcBuffer.buffer = buffer;
        srcBuffer.offset = 0;
        srcBuffer.rowPitch = rowBytes;
        srcBuffer.imageHeight = h;
        dawn::TextureCopyView dstTexture;
        dstTexture.texture = tex;
        dstTexture.level = i;
        dstTexture.slice = 0;
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
    GrBackendTexture beTex = GrBackendTexture(width, height, info);
    beTex.setPixelConfig(config);
    return beTex;
}

void GrDawnGpu::deleteBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(GrBackendApi::kDawn == tex.fBackend);

    GrDawnImageInfo info;
    if (tex.getDawnImageInfo(&info)) {
        info.fTexture = nullptr;
    }
}

#if GR_TEST_UTILS
bool GrDawnGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(GrBackendApi::kDawn == tex.fBackend);

    GrDawnImageInfo info;
    if (!tex.getDawnImageInfo(&info)) {
        return false;
    }

    return info.fTexture.Get();
}

GrBackendRenderTarget GrDawnGpu::createTestingOnlyBackendRenderTarget(int width, int height,
                                                                      GrColorType colorType) {
    GrPixelConfig config = GrColorTypeToPixelConfig(colorType, GrSRGBEncoded::kNo);
    if (!this->caps()->isConfigTexturable(config)) {
        return GrBackendRenderTarget();
    }

    if (width > this->caps()->maxTextureSize() || height > this->caps()->maxTextureSize()) {
        return GrBackendRenderTarget();
    }

    dawn::TextureFormat format;
    if (!GrPixelConfigToDawnFormat(config, &format)) {
        return GrBackendRenderTarget();
    }

    dawn::TextureDescriptor desc;
    desc.usage =
        dawn::TextureUsageBit::TransferSrc |
        dawn::TextureUsageBit::OutputAttachment;

    desc.dimension = dawn::TextureDimension::e2D;
    desc.size.width = width;
    desc.size.height = height;
    desc.size.depth = 1;
    desc.arrayLayerCount = 1;
    desc.format = format;
    desc.sampleCount = 1;
    desc.mipLevelCount = 1;

    dawn::Texture tex = this->device().CreateTexture(&desc);

    GrDawnImageInfo info;
    info.fTexture = tex;
    info.fFormat = desc.format;
    info.fLevelCount = desc.mipLevelCount;
    GrBackendRenderTarget result = GrBackendRenderTarget(width, height, 1, 0, info);
    result.setPixelConfig(config);
    return result;
}

void GrDawnGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget& rt) {
    SkASSERT(GrBackendApi::kDawn == rt.fBackend);

    GrDawnImageInfo info;
    if (rt.getDawnImageInfo(&info)) {
        info.fTexture = nullptr;
    }
}

void GrDawnGpu::testingOnly_flushGpuAndSync() {
    flush();
}

#endif

void GrDawnGpu::flush() {
    fCommandBuffers[0] = fCopyEncoder.Finish();
    fQueue.Submit(fCommandBuffers.size(), &fCommandBuffers.front());
    fCopyEncoder = fDevice.CreateCommandEncoder();
    fCommandBuffers.clear();
    fCommandBuffers.push_back(nullptr);
    fStagingManager.mapBusyList();
    fDevice.Tick();
}

void GrDawnGpu::onFinishFlush(GrSurfaceProxy*[], int n, SkSurface::BackendSurfaceAccess access,
                              const GrFlushInfo& info, const GrPrepareForExternalIORequests&) {
    flush();
}

bool GrDawnGpu::onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                              GrSurface* src, GrSurfaceOrigin srcOrigin,
                              const SkIRect& srcRect,
                              const SkIPoint& dstPoint,
                              bool canDiscardOutsideDstRect) {
    SkASSERT(!"unimplemented");
    return false;
}

static void callback(DawnBufferMapAsyncStatus status, const void* data, uint64_t dataLength, void* userdata) {
    auto gpu = reinterpret_cast<GrDawnGpu*>(userdata);
    gpu->setReadPixelsPtr(data);
}

bool GrDawnGpu::onReadPixels(GrSurface* surface,
                             int left, int top, int width, int height,
                             GrColorType colorType,
                             void* buffer,
                             size_t rowBytes) {
    dawn::Texture tex;
    if (auto rt = static_cast<GrDawnRenderTarget*>(surface->asRenderTarget())) {
        tex = rt->texture();
    } else if (auto t = static_cast<GrDawnTexture*>(surface->asTexture())) {
        tex = t->texture();
    } else {
        return false;
    }

    size_t origRowBytes = rowBytes;
    int origSizeInBytes = origRowBytes * height;
    rowBytes = (rowBytes + 0xFF) & ~0xFF;
    int sizeInBytes = rowBytes * height;
    dawn::BufferDescriptor desc;
    desc.usage = dawn::BufferUsageBit::TransferDst | dawn::BufferUsageBit::MapRead;
    desc.size = sizeInBytes;
    dawn::Buffer buf = device().CreateBuffer(&desc);
    dawn::TextureCopyView srcTexture;
    srcTexture.texture = tex;
    srcTexture.level = 0;
    srcTexture.slice = 0;
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
    return false;
}

void GrDawnGpu::submit(GrGpuCommandBuffer* buffer) {
    if (auto buf = static_cast<GrDawnGpuRTCommandBuffer*>(buffer->asRTCommandBuffer())) {
        buf->submit();
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

sk_sp<GrDawnProgram> GrDawnGpu::getOrCreateRenderPipeline(
        GrRenderTarget* rt,
        GrSurfaceOrigin origin,
        const GrPipeline& pipeline,
        const GrPrimitiveProcessor& primProc,
        const GrTextureProxy* const* primProcProxies,
        bool hasPoints,
        GrPrimitiveType primitiveType) {
    bool hasDepthStencil = rt->renderTargetPriv().getStencilAttachment() != nullptr;
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, rt, primProc, hasPoints, pipeline, this);
    GrProcessorKeyBuilder b(&desc.key());
    GrStencilSettings stencil;
    stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(), 8);
    stencil.genKey(&b);
    b.add32(rt->config());
    b.add32(static_cast<int32_t>(hasDepthStencil));
    b.add32(get_blend_info_key(pipeline));
    b.add32(static_cast<uint32_t>(primitiveType));

    if (sk_sp<GrDawnProgram>* program = fRenderPipelineCache.find(desc)) {
        return *program;
    }

    dawn::TextureFormat colorFormat;
    SkAssertResult(GrPixelConfigToDawnFormat(rt->config(), &colorFormat));
    dawn::TextureFormat stencilFormat = dawn::TextureFormat::D32FloatS8Uint;

    sk_sp<GrDawnProgram> program = GrDawnProgramBuilder::Build(
        this, rt, origin, pipeline, primProc, primProcProxies, primitiveType, colorFormat, hasDepthStencil, stencilFormat, &desc);
    fRenderPipelineCache.insert(desc, program);
    return program;
}

dawn::Sampler GrDawnGpu::getOrCreateSampler(const GrSamplerState& samplerState) {
    auto i = fSamplers.find(samplerState);
    if (i != fSamplers.end()) {
        return i->second;
    }
    dawn::SamplerDescriptor desc;
    desc.addressModeU = to_dawn_address_mode(samplerState.wrapModeX());
    desc.addressModeV = to_dawn_address_mode(samplerState.wrapModeY());
    desc.addressModeW = dawn::AddressMode::ClampToEdge;
    desc.magFilter = desc.minFilter = to_dawn_filter_mode(samplerState.filter());
    desc.mipmapFilter = dawn::FilterMode::Linear;
    desc.lodMinClamp = 0.0f;
    desc.lodMaxClamp = 1000.0f;
    desc.compareFunction = dawn::CompareFunction::Never;
    dawn::Sampler sampler = device().CreateSampler(&desc);
    fSamplers.insert(std::pair<GrSamplerState, dawn::Sampler>(samplerState, sampler));
    return sampler;
}

GrDawnRingBuffer::Slice GrDawnGpu::allocateUniformRingBufferSlice(int size) {
    return fUniformRingBuffer.allocate(size);
}

GrDawnStagingBuffer* GrDawnGpu::getStagingBuffer(size_t size) {
    return fStagingManager.findOrCreateStagingBuffer(size, dawn::BufferUsageBit::TransferSrc);
}

void GrDawnGpu::appendCommandBuffer(dawn::CommandBuffer commandBuffer) {
    if (commandBuffer) {
        fCommandBuffers.push_back(commandBuffer);
    }
}
