/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrDawnGpu.h"

#include "GrBackendSemaphore.h"
#include "GrBackendSurface.h"
#include "GrContextOptions.h"
#include "GrGeometryProcessor.h"
#include "GrGpuResourceCacheAccess.h"
#include "GrMesh.h"
#include "GrDawnBuffer.h"
#include "GrDawnCaps.h"
#include "GrDawnGpuCommandBuffer.h"
#include "GrDawnProgramBuilder.h"
#include "GrDawnRenderTarget.h"
#include "GrDawnStencilAttachment.h"
#include "GrDawnTexture.h"
#include "GrDawnUtil.h"
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

sk_sp<GrGpu> GrDawnGpu::Make(sk_sp<const GrDawnBackendContext> backendContext,
                            const GrContextOptions& options, GrContext* context) {
    if (!backendContext) {
        return nullptr;
    }

    return sk_sp<GrGpu>(new GrDawnGpu(context, options, backendContext));
}

////////////////////////////////////////////////////////////////////////////////

GrDawnGpu::GrDawnGpu(GrContext* context, const GrContextOptions& options,
                   sk_sp<const GrDawnBackendContext> backendContext)
        : INHERITED(context)
        , fDevice(dawn::Device::Acquire(backendContext->fDevice))
        , fQueue(dawn::Queue::Acquire(backendContext->fQueue))
        , fUniformRingBuffer(this, dawn::BufferUsageBit::Uniform)
        , fCopyBuilder(fDevice.CreateCommandBufferBuilder())
        , fRenderPipelineCache(kMaxRenderPipelineEntries)
        , fStagingManager(fDevice.Clone()) {
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
            GrRenderTarget* rt, GrSurfaceOrigin origin,
            const GrGpuRTCommandBuffer::LoadAndStoreInfo& colorInfo,
            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo& stencilInfo) {
    return new GrDawnGpuRTCommandBuffer(this, rt, origin, colorInfo, stencilInfo);
}

GrGpuTextureCommandBuffer* GrDawnGpu::getCommandBuffer(GrTexture* texture,
                                                       GrSurfaceOrigin origin) {
    return new GrDawnGpuTextureCommandBuffer(this, texture, origin);
}

///////////////////////////////////////////////////////////////////////////////
GrBuffer* GrDawnGpu::onCreateBuffer(size_t size, GrBufferType type, GrAccessPattern accessPattern,
                                   const void* data) {
    GrBuffer* b = new GrDawnBuffer(this, size, type, accessPattern);
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
                    fCopyBuilder.Clone());
    return true;
}

bool GrDawnGpu::onTransferPixels(GrTexture* texture,
                                int left, int top, int width, int height,
                                GrColorType colorType, GrBuffer* transferBuffer,
                                size_t bufferOffset, size_t rowBytes) {
    SkASSERT(!"unimplemented");
    return false;
}

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
    tex->upload(texels, mipLevels, fCopyBuilder.Clone());
    return tex;
}

sk_sp<GrTexture> GrDawnGpu::onWrapBackendTexture(const GrBackendTexture& tex,
                                                GrWrapOwnership ownership) {
    GrDawnImageInfo info;
    if (!tex.getDawnImageInfo(&info)) {
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
    sk_sp<GrDawnTexture> tgt = GrDawnTexture::MakeWrapped(this, desc, status, info);
    return tgt;
}

sk_sp<GrTexture> GrDawnGpu::onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                          int sampleCnt,
                                                          GrWrapOwnership) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrRenderTarget> GrDawnGpu::onWrapBackendRenderTarget(const GrBackendRenderTarget&) {
    SkASSERT(!"unimplemented");
    return nullptr;
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

    sk_sp<GrDawnRenderTarget> tgt = GrDawnRenderTarget::MakeWrapped(this, desc, info);
    return tgt;
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

#if GR_TEST_UTILS
GrBackendTexture GrDawnGpu::createTestingOnlyBackendTexture(const void* pixels,
                                                           int width, int height,
                                                           GrColorType colorType,
                                                           bool isRenderTarget,
                                                           GrMipMapped mipMapped,
                                                           size_t rowBytes) {
    GrPixelConfig config = GrColorTypeToPixelConfig(colorType, GrSRGBEncoded::kNo);
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
    desc.size.width = width;
    desc.size.height = height;
    desc.size.depth = 1;
    desc.arrayLayer = 1;
    desc.format = GrPixelConfigToDawnFormat(config);

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
        dawn::Buffer buffer = stagingBuffer->fBuffer.Clone();
        buffer.Unmap();
        stagingBuffer->fData = nullptr;
        copyBuilder.CopyBufferToTexture(buffer, 0, rowBytes, tex, 0, 0, 0, w, h, 1, i, 0);
        w = SkTMax(1, w / 2);
        h = SkTMax(1, h / 2);
    }
    dawn::CommandBuffer cmdBuf = copyBuilder.GetResult();
    fQueue.Submit(1, &cmdBuf);
    GrDawnImageInfo info;
    info.fTexture = tex.Clone();
    info.fFormat = desc.format;
    info.fLevelCount = desc.mipLevel;
    GrBackendTexture beTex = GrBackendTexture(width, height, info);
    beTex.setPixelConfig(config);
    return beTex;
}

bool GrDawnGpu::isTestingOnlyBackendTexture(const GrBackendTexture& tex) const {
    SkASSERT(!"unimplemented");
    return false;
}

void GrDawnGpu::deleteTestingOnlyBackendTexture(const GrBackendTexture& tex) {
    SkASSERT(!"unimplemented");
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

void GrDawnGpu::onFinishFlush(bool insertedSemaphore) {

    fCommandBuffers[0] = fCopyBuilder.GetResult();
    fQueue.Submit(fCommandBuffers.size(), &fCommandBuffers.front());
    fCopyBuilder = fDevice.CreateCommandBufferBuilder();
    fCommandBuffers.clear();
    fCommandBuffers.push_back(nullptr);
    fStagingManager.mapBusyList();
    fDevice.Tick();
}

bool GrDawnGpu::onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                             GrSurface* src, GrSurfaceOrigin srcOrigin,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint,
                             bool canDiscardOutsideDstRect) {
    SkASSERT(!"unimplemented");
    return false;
}

static void callback(dawnBufferMapAsyncStatus status, const void* data, dawn::CallbackUserdata userdata) {
    auto gpu = reinterpret_cast<GrDawnGpu*>(userdata);
    gpu->setReadPixelsPtr(data);
}

bool GrDawnGpu::onReadPixels(GrSurface* surface,
                            int left, int top, int width, int height,
                            GrColorType colorType,
                            void* buffer,
                            size_t rowBytes) {
    dawn::Texture tex = static_cast<GrDawnTexture*>(surface->asTexture())->texture();

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

void GrDawnGpu::insertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) {
    SkASSERT(!"unimplemented");
}

void GrDawnGpu::waitSemaphore(sk_sp<GrSemaphore> semaphore) {
    SkASSERT(!"unimplemented");
}

sk_sp<GrSemaphore> GrDawnGpu::prepareTextureForCrossContextUsage(GrTexture* texture) {
    SkASSERT(!"unimplemented");
    return nullptr;
}

sk_sp<GrDawnProgram> GrDawnGpu::getOrCreateRenderPipeline(GrRenderTarget* rt,
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

    if (sk_sp<GrDawnProgram>* program = fRenderPipelineCache.find(desc)) {
        return *program;
    }

    dawn::TextureFormat colorFormat = GrPixelConfigToDawnFormat(rt->config());
    dawn::TextureFormat stencilFormat = dawn::TextureFormat::D32FloatS8Uint;

    sk_sp<GrDawnProgram> program = GrDawnProgramBuilder::Build(
        this, pipeline, primProc, primitiveType, colorFormat, hasDepthStencil, stencilFormat, &desc);
    fRenderPipelineCache.insert(desc, program);
    return program;
}

dawn::Sampler GrDawnGpu::getOrCreateSampler(const GrSamplerState& samplerState) {
    auto i = fSamplers.find(samplerState);
    if (i != fSamplers.end()) {
        return i->second.Clone();
    }
    dawn::SamplerDescriptor desc;
    desc.minFilter = desc.magFilter = to_dawn_filter_mode(samplerState.filter());
    desc.mipmapFilter = dawn::FilterMode::Linear;
    desc.addressModeU = to_dawn_address_mode(samplerState.wrapModeX());
    desc.addressModeV = to_dawn_address_mode(samplerState.wrapModeY());
    desc.addressModeW = dawn::AddressMode::ClampToEdge;
    dawn::Sampler sampler = device().CreateSampler(&desc);
    fSamplers.insert(std::pair<GrSamplerState, dawn::Sampler>(samplerState, sampler.Clone()));
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
        fCommandBuffers.push_back(commandBuffer.Clone());
    }
}
