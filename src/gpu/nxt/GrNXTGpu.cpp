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

#include "SkSLCompiler.h"

#if !defined(SK_BUILD_FOR_WIN)
#include <unistd.h>
#endif // !defined(SK_BUILD_FOR_WIN)

const int kMaxRenderPipelineEntries = 1024;

namespace {

nxt::FilterMode to_nxt_filter_mode(GrSamplerState::Filter filter) {
    switch (filter) {
        case GrSamplerState::Filter::kNearest:
            return nxt::FilterMode::Nearest;
        case GrSamplerState::Filter::kBilerp:
        case GrSamplerState::Filter::kMipMap:
            return nxt::FilterMode::Linear;
        default:
            SkASSERT(!"unsupported filter mode");
            return nxt::FilterMode::Nearest;
    }
}

nxt::AddressMode to_nxt_address_mode(GrSamplerState::WrapMode wrapMode) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            return nxt::AddressMode::ClampToEdge;
        case GrSamplerState::WrapMode::kRepeat:
            return nxt::AddressMode::Repeat;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            return nxt::AddressMode::MirroredRepeat;
    }
    SkASSERT(!"unsupported address mode");
    return nxt::AddressMode::ClampToEdge;

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
        , fDevice(nxt::Device::Acquire(backendContext->fDevice))
        , fQueue(nxt::Queue::Acquire(backendContext->fQueue))
        , fUniformRingBuffer(this, nxt::BufferUsageBit::Uniform)
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
    SkASSERT(!"unimplemented");
    return nullptr;
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

sk_sp<GrTexture> GrNXTGpu::onWrapBackendTexture(const GrBackendTexture& backendTex,
                                                GrWrapOwnership ownership) {
    SkASSERT(!"unimplemented");
    return nullptr;
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
GrBackendTexture GrNXTGpu::createTestingOnlyBackendTexture(const void* srcData, int w, int h,
                                                           GrPixelConfig config,
                                                           bool isRenderTarget,
                                                           GrMipMapped mipMapped) {
    SkASSERT(!"unimplemented");
    return GrBackendTexture();
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
        .SetDepthStencilAttachment(attachment->view(), nxt::LoadOp::Clear, nxt::LoadOp::Clear)
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

static void callback(nxtBufferMapAsyncStatus status, const void* data, nxt::CallbackUserdata userdata) {
    auto gpu = reinterpret_cast<GrNXTGpu*>(userdata);
    gpu->setReadPixelsPtr(data);
}

bool GrNXTGpu::onReadPixels(GrSurface* surface,
                            int left, int top, int width, int height,
                            GrColorType colorType,
                            void* buffer,
                            size_t rowBytes) {
    nxt::Texture tex = static_cast<GrNXTTexture*>(surface->asTexture())->texture();

    int sizeInBytes = rowBytes * height;
    nxt::Buffer buf = device().CreateBufferBuilder()
        .SetAllowedUsage(nxt::BufferUsageBit::TransferDst | nxt::BufferUsageBit::MapRead)
        .SetSize(sizeInBytes)
        .GetResult();
    auto commandBuffer = device().CreateCommandBufferBuilder()
        .CopyTextureToBuffer(tex, left, top, 0, width, height, 1, 0, buf, 0, rowBytes)
        .GetResult();
    queue().Submit(1, &commandBuffer);
    buf.MapReadAsync(0, sizeInBytes, callback, reinterpret_cast<nxt::CallbackUserdata>(this));
    while (!fReadPixelsPtr) {
        device().Tick();
    }
    memcpy(buffer, fReadPixelsPtr, sizeInBytes);
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

    nxt::TextureFormat colorFormat = GrPixelConfigToNXTFormat(rt->config());
    nxt::TextureFormat stencilFormat = nxt::TextureFormat::D32FloatS8Uint;

    sk_sp<GrNXTProgram> program = GrNXTProgramBuilder::Build(
        this, pipeline, primProc, primitiveType, colorFormat, hasDepthStencil, stencilFormat, &desc);
    fRenderPipelineCache.insert(desc, program);
    return program;
}

nxt::Sampler GrNXTGpu::getOrCreateSampler(const GrSamplerState& samplerState) {
    auto i = fSamplers.find(samplerState);
    if (i != fSamplers.end()) {
        return i->second.Clone();
    }
    nxt::SamplerDescriptor desc;
    desc.minFilter = desc.magFilter = to_nxt_filter_mode(samplerState.filter());
    desc.mipmapFilter = nxt::FilterMode::Linear;
    desc.addressModeU = to_nxt_address_mode(samplerState.wrapModeX());
    desc.addressModeV = to_nxt_address_mode(samplerState.wrapModeY());
    desc.addressModeW = nxt::AddressMode::ClampToEdge;
    nxt::Sampler sampler = device().CreateSampler(&desc);
    fSamplers.insert(std::pair<GrSamplerState, nxt::Sampler>(samplerState, sampler.Clone()));
    return sampler;
}

GrNXTRingBuffer::Slice GrNXTGpu::allocateUniformRingBufferSlice(int size) {
    return fUniformRingBuffer.allocate(size);
}

GrNXTStagingBuffer* GrNXTGpu::getStagingBuffer(size_t size) {
    return fStagingManager.findOrCreateStagingBuffer(size, nxt::BufferUsageBit::TransferSrc);
}

void GrNXTGpu::appendCommandBuffer(nxt::CommandBuffer commandBuffer) {
    if (commandBuffer) {
        fCommandBuffers.push_back(commandBuffer.Clone());
    }
}
