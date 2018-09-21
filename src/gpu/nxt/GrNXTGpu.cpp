/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrNXTGpu.h"

#include "GrBackendSemaphore.h"
#include "GrBackendSurface.h"
#include "GrContextOptions.h"
#include "GrGeometryProcessor.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrMesh.h"
#include "GrNXTBuffer.h"
#include "GrNXTCaps.h"
#include "GrNXTGpuCommandBuffer.h"
#include "GrNXTProgramBuilder.h"
#include "GrNXTRenderTarget.h"
#include "GrNXTStencilAttachment.h"
#include "GrNXTTexture.h"
#include "GrNXTUtil.h"
#include "GrPipeline.h"
#include "GrRenderTargetPriv.h"
#include "GrSemaphore.h"
#include "GrStencilSettings.h"
#include "GrTexturePriv.h"

#include "SkAutoMalloc.h"
#include "SkMipMap.h"
#include "SkSLCompiler.h"

#if !defined(SK_BUILD_FOR_WIN)
#include <unistd.h>
#endif // !defined(SK_BUILD_FOR_WIN)

const int kMaxRenderPipelineEntries = 1024;

namespace {

dawn::FilterMode to_nxt_filter_mode(GrSamplerState::Filter filter) {
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

dawn::AddressMode to_nxt_address_mode(GrSamplerState::WrapMode wrapMode) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            return dawn::AddressMode::ClampToEdge;
        case GrSamplerState::WrapMode::kRepeat:
            return dawn::AddressMode::Repeat;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            return dawn::AddressMode::MirroredRepeat;
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

sk_sp<GrGpu> GrNXTGpu::Make(sk_sp<const GrNXTBackendContext> backendContext,
                            const GrContextOptions& options, GrContext* context) {
    if (!backendContext) {
        return nullptr;
    }

    return sk_sp<GrGpu>(new GrNXTGpu(context, options, backendContext));
}

////////////////////////////////////////////////////////////////////////////////

GrNXTGpu::GrNXTGpu(GrContext* context, const GrContextOptions& options,
                   sk_sp<const GrNXTBackendContext> backendContext)
        : INHERITED(context)
        , fDevice(dawn::Device::Acquire(backendContext->fDevice))
        , fQueue(dawn::Queue::Acquire(backendContext->fQueue))
        , fUniformRingBuffer(this, dawn::BufferUsageBit::Uniform)
        , fCopyBuilder(fDevice.CreateCommandBufferBuilder())
        , fRenderPipelineCache(kMaxRenderPipelineEntries)
        , fStagingManager(fDevice.Clone()) {
    fCompiler = new SkSL::Compiler();
    fCaps.reset(new GrNXTCaps(options));
    // This will be filled by the copy command buffer.
    fCommandBuffers.push_back(nullptr);
}

GrNXTGpu::~GrNXTGpu() {
    delete fCompiler;
}


void GrNXTGpu::disconnect(DisconnectType type) {
    SkASSERT(!"unimplemented");
}

///////////////////////////////////////////////////////////////////////////////

GrGpuRTCommandBuffer* GrNXTGpu::createCommandBuffer(
            GrRenderTarget* rt, GrSurfaceOrigin origin,
            const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo) {
    return new GrNXTGpuRTCommandBuffer(this, rt, origin, colorInfo, stencilInfo);
}

GrGpuTextureCommandBuffer* GrNXTGpu::createCommandBuffer(GrTexture* texture,
                                                         GrSurfaceOrigin origin) {
    return new GrNXTGpuTextureCommandBuffer(this, texture, origin);
}

///////////////////////////////////////////////////////////////////////////////
GrBuffer* GrNXTGpu::onCreateBuffer(size_t size, GrBufferType type, GrAccessPattern accessPattern,
                                   const void* data) {
    GrBuffer* b = new GrNXTBuffer(this, size, type, accessPattern);
    if (data && b) {
        b->updateData(data, size);
    }
    return b;
}

////////////////////////////////////////////////////////////////////////////////
bool GrNXTGpu::onWritePixels(GrSurface* surface,
                             int left, int top, int width, int height,
                             GrColorType colorType,
                             const GrMipLevel texels[], int mipLevelCount) {
    GrNXTTexture* texture = (GrNXTTexture*) surface->asTexture();
    if (!texture) {
        SkASSERT(!"uploading to non-texture unimplemented");
        return false;
    }
    texture->upload(texels, mipLevelCount, SkIRect::MakeXYWH(left, top, width, height),
                    fCopyBuilder.Clone());
    return true;
}

bool GrNXTGpu::onTransferPixels(GrTexture* texture,
                                int left, int top, int width, int height,
                                GrColorType colorType, GrBuffer* transferBuffer,
                                size_t bufferOffset, size_t rowBytes) {
    SkASSERT(!"unimplemented");
    return false;
}

sk_sp<GrTexture> GrNXTGpu::onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
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

    sk_sp<GrNXTTexture> tex = GrNXTTexture::Make(this, desc, budgeted, mipMapsStatus);
    if (!tex) {
        return nullptr;
    }
    tex->upload(texels, mipLevels, fCopyBuilder.Clone());
    return tex;
}

sk_sp<GrTexture> GrNXTGpu::onWrapBackendTexture(const GrBackendTexture& tex,
                                                GrWrapOwnership ownership) {
    GrNXTImageInfo info;
    if (!tex.getNXTImageInfo(&info)) {
        return nullptr;
    }
    if (!info.fTexture) {
        return nullptr;
    }

    GrSurfaceDesc desc;
    desc.fFlags = kNone_GrSurfaceFlags;
    desc.fWidth = tex.width();
    desc.fHeight = tex.height();
    desc.fConfig = tex.config();
    desc.fSampleCnt = 1;

    GrMipMapsStatus status = GrMipMapsStatus::kNotAllocated;
    sk_sp<GrNXTTexture> tgt = GrNXTTexture::MakeWrapped(this, desc, status, info);
    return tgt;
}

sk_sp<GrTexture> GrNXTGpu::onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                          int sampleCnt,
                                                          GrWrapOwnership) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrRenderTarget> GrNXTGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget&) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrRenderTarget> GrNXTGpu::onWrapBackendTextureAsRenderTarget(const GrBackendTexture& tex,
                                                                   int sampleCnt) {
    GrNXTImageInfo info;
    if (!tex.getNXTImageInfo(&info)) {
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

    sk_sp<GrNXTRenderTarget> tgt = GrNXTRenderTarget::MakeWrapped(this, desc, info);
    return tgt;
}

GrStencilAttachment* GrNXTGpu::createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                      int width,
                                                                      int height) {
    SkASSERT(width >= rt->width());
    SkASSERT(height >= rt->height());

    int samples = rt->numStencilSamples();

    GrNXTStencilAttachment* stencil(GrNXTStencilAttachment::Create(this,
                                                                   width,
                                                                   height,
                                                                   samples));
    fStats.incStencilAttachmentCreates();
    return stencil;
}

#if GR_TEST_UTILS
GrBackendTexture GrNXTGpu::createTestingOnlyBackendTexture(const void* pixels,
                                                           int width, int height,
                                                           GrPixelConfig config,
                                                           bool isRenderTarget,
                                                           GrMipMapped mipMapped) {
    if (!this->caps()->isConfigTexturable(config)) {
        return GrBackendTexture();
    }

    if (width > this->caps()->maxTextureSize() || height > this->caps()->maxTextureSize()) {
        return GrBackendTexture();
    }

    // Currently we don't support uploading pixel data when mipped.
    if (pixels && GrMipMapped::kYes == mipMapped) {
        return GrBackendTexture();
    }

    dawn::TextureDescriptor desc;
    desc.usage =
        dawn::TextureUsageBit::Sampled |
        dawn::TextureUsageBit::TransferSrc |
        dawn::TextureUsageBit::TransferDst;

    if (isRenderTarget) {
        desc.usage |= dawn::TextureUsageBit::OutputAttachment;
    }

    desc.dimension = dawn::TextureDimension::e2D;
    desc.width = width;
    desc.height = height;
    desc.depth = 1;
    desc.arrayLayer = 1;
    desc.format = GrPixelConfigToNXTFormat(config);

    // Figure out the number of mip levels.
    if (GrMipMapped::kYes == mipMapped) {
        desc.mipLevel = SkMipMap::ComputeLevelCount(width, height) + 1;
    } else {
        desc.mipLevel = 1;
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
    dawn::CommandBufferBuilder copyBuilder = fDevice.CreateCommandBufferBuilder();
    int w = width, h = height;
    for (uint32_t i = 0; i < desc.mipLevel; i++) {
        size_t origRowBytes = bpp * w;
        size_t rowBytes = origRowBytes;
        if ((rowBytes & 0xFF) != 0) {
            rowBytes = (rowBytes + 0xFF) & ~0xFF;
        }
        size_t size = rowBytes * h;
        GrNXTStagingBuffer* stagingBuffer = this->getStagingBuffer(size);
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
        dawn::Buffer buffer = stagingBuffer->fBuffer.Clone();
        buffer.Unmap();
        stagingBuffer->fData = nullptr;
        copyBuilder.CopyBufferToTexture(buffer, 0, rowBytes, tex, 0, 0, 0, w, h, 1, i, 0);
        w = SkTMax(1, w / 2);
        h = SkTMax(1, h / 2);
    }
    dawn::CommandBuffer cmdBuf = copyBuilder.GetResult();
    fQueue.Submit(1, &cmdBuf);
    GrNXTImageInfo info;
    info.fTexture = tex.Clone();
    info.fFormat = desc.format;
    info.fLevelCount = desc.mipLevel;
    GrBackendTexture beTex = GrBackendTexture(width, height, info);
    beTex.setPixelConfig(config);
    return beTex;

}

bool GrNXTGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(!"unimplemented");
    return false;
}

void GrNXTGpu::deleteTestingOnlyBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(!"unimplemented");
}

GrBackendRenderTarget GrNXTGpu::createTestingOnlyBackendRenderTarget(int w, int h, GrColorType,
                                                                     GrSRGBEncoded) {
    SkASSERT(!"unimplemented");
    return GrBackendRenderTarget();
}

void GrNXTGpu::deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) {
    SkASSERT(!"unimplemented");
}

void GrNXTGpu::testingOnly_flushGpuAndSync() {
    SkASSERT(!"unimplemented");
}

#endif

void GrNXTGpu::onFinishFlush(bool insertedSemaphore) {

    fCommandBuffers[0] = fCopyBuilder.GetResult();
    fQueue.Submit(fCommandBuffers.size(), &fCommandBuffers.front());
    fCopyBuilder = fDevice.CreateCommandBufferBuilder();
    fCommandBuffers.clear();
    fCommandBuffers.push_back(nullptr);
    fStagingManager.mapBusyList();
    fDevice.Tick();
}

void GrNXTGpu::clearStencil(GrRenderTarget* target, int clearValue) {
    // FIXME: this clear could be integrated into the next draw's subpass
    auto attachment = static_cast<GrNXTStencilAttachment*>(target->renderTargetPriv().getStencilAttachment());
    auto renderPassDescriptor = device().CreateRenderPassDescriptorBuilder()
        .SetDepthStencilAttachment(attachment->view(), dawn::LoadOp::Clear, dawn::LoadOp::Clear)
        .SetDepthStencilAttachmentClearValue(0.0f, clearValue)
        .GetResult();
    appendCommandBuffer(device().CreateCommandBufferBuilder()
            .BeginRenderPass(renderPassDescriptor)
            .EndRenderPass()
            .GetResult());
}

bool GrNXTGpu::onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                             GrSurface* src, GrSurfaceOrigin srcOrigin,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint,
                             bool canDiscardOutsideDstRect) {
    SkASSERT(!"unimplemented");
    return false;
}

static void callback(dawnBufferMapAsyncStatus status, const void* data, dawn::CallbackUserdata userdata) {
    auto gpu = reinterpret_cast<GrNXTGpu*>(userdata);
    gpu->setReadPixelsPtr(data);
}

bool GrNXTGpu::onReadPixels(GrSurface* surface,
                            int left, int top, int width, int height,
                            GrColorType colorType,
                            void* buffer,
                            size_t rowBytes) {
    dawn::Texture tex = static_cast<GrNXTTexture*>(surface->asTexture())->texture();

    size_t origRowBytes = rowBytes;
    int origSizeInBytes = origRowBytes * height;
    rowBytes = (rowBytes + 0xFF) & ~0xFF;
    int sizeInBytes = rowBytes * height;
    dawn::BufferDescriptor desc;
    desc.usage = dawn::BufferUsageBit::TransferDst | dawn::BufferUsageBit::MapRead;
    desc.size = sizeInBytes;
    dawn::Buffer buf = device().CreateBuffer(&desc);
    auto commandBuffer = device().CreateCommandBufferBuilder()
        .CopyTextureToBuffer(tex, left, top, 0, width, height, 1, 0, 0, buf, 0, rowBytes)
        .GetResult();
    queue().Submit(1, &commandBuffer);
    buf.MapReadAsync(0, origSizeInBytes, callback, reinterpret_cast<dawn::CallbackUserdata>(this));
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

bool GrNXTGpu::onRegenerateMipMapLevels(GrTexture*) {
    return false;
}

GrFence SK_WARN_UNUSED_RESULT GrNXTGpu::insertFence() {
    SkASSERT(!"unimplemented");
    return GrFence();
}

bool GrNXTGpu::waitFence(GrFence fence, uint64_t timeout) {
    SkASSERT(!"unimplemented");
    return false;
}

void GrNXTGpu::deleteFence(GrFence fence) const {
    SkASSERT(!"unimplemented");
}

sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT GrNXTGpu::makeSemaphore(bool isOwned) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrSemaphore> GrNXTGpu::wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                                  GrResourceProvider::SemaphoreWrapType wrapType,
                                                  GrWrapOwnership ownership) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

void GrNXTGpu::insertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) {
    SkASSERT(!"unimplemented");
}

void GrNXTGpu::waitSemaphore(sk_sp<GrSemaphore> semaphore) {
    SkASSERT(!"unimplemented");
}

sk_sp<GrSemaphore> GrNXTGpu::prepareTextureForCrossContextUsage(GrTexture* texture) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrNXTProgram> GrNXTGpu::getOrCreateRenderPipeline(GrRenderTarget* rt,
                                                        const GrPipeline& pipeline,
                                                        const GrPrimitiveProcessor& primProc,
                                                        bool hasPoints,
                                                        GrPrimitiveType primitiveType) {
    bool hasDepthStencil = rt->renderTargetPriv().getStencilAttachment() != nullptr;
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, primProc, hasPoints, pipeline, *this->caps()->shaderCaps());
    GrProcessorKeyBuilder b(&desc.key());
    GrStencilSettings stencil;
    stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(), 8);
    stencil.genKey(&b);
    b.add32(rt->config());
    b.add32(static_cast<int32_t>(hasDepthStencil));
    b.add32(get_blend_info_key(pipeline));
    b.add32(static_cast<uint32_t>(primitiveType));
    desc.finalize();

    if (sk_sp<GrNXTProgram>* program = fRenderPipelineCache.find(desc)) {
        return *program;
    }

    dawn::TextureFormat colorFormat = GrPixelConfigToNXTFormat(rt->config());
    dawn::TextureFormat stencilFormat = dawn::TextureFormat::D32FloatS8Uint;

    sk_sp<GrNXTProgram> program = GrNXTProgramBuilder::Build(
        this, pipeline, primProc, primitiveType, colorFormat, hasDepthStencil, stencilFormat, &desc);
    fRenderPipelineCache.insert(desc, program);
    return program;
}

dawn::Sampler GrNXTGpu::getOrCreateSampler(const GrSamplerState& samplerState) {
    auto i = fSamplers.find(samplerState);
    if (i != fSamplers.end()) {
        return i->second.Clone();
    }
    dawn::SamplerDescriptor desc;
    desc.minFilter = desc.magFilter = to_nxt_filter_mode(samplerState.filter());
    desc.mipmapFilter = dawn::FilterMode::Linear;
    desc.addressModeU = to_nxt_address_mode(samplerState.wrapModeX());
    desc.addressModeV = to_nxt_address_mode(samplerState.wrapModeY());
    desc.addressModeW = dawn::AddressMode::ClampToEdge;
    dawn::Sampler sampler = device().CreateSampler(&desc);
    fSamplers.insert(std::pair<GrSamplerState, dawn::Sampler>(samplerState, sampler.Clone()));
    return sampler;
}

GrNXTRingBuffer::Slice GrNXTGpu::allocateUniformRingBufferSlice(int size) {
    return fUniformRingBuffer.allocate(size);
}

GrNXTStagingBuffer* GrNXTGpu::getStagingBuffer(size_t size) {
    return fStagingManager.findOrCreateStagingBuffer(size, dawn::BufferUsageBit::TransferSrc);
}

void GrNXTGpu::appendCommandBuffer(dawn::CommandBuffer commandBuffer) {
    if (commandBuffer) {
        fCommandBuffers.push_back(commandBuffer.Clone());
    }
}
