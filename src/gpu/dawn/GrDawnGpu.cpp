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

static dawn::FilterMode to_dawn_filter_mode(GrSamplerState::Filter filter) {
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

static dawn::AddressMode to_dawn_address_mode(GrSamplerState::WrapMode wrapMode) {
    switch (wrapMode) {
        case GrSamplerState::WrapMode::kClamp:
            return dawn::AddressMode::ClampToEdge;
        case GrSamplerState::WrapMode::kRepeat:
            return dawn::AddressMode::Repeat;
        case GrSamplerState::WrapMode::kMirrorRepeat:
            return dawn::AddressMode::MirrorRepeat;
        case GrSamplerState::WrapMode::kClampToBorder:
            SkASSERT(!"unsupported address mode");
    }
    SkASSERT(!"unsupported address mode");
    return dawn::AddressMode::ClampToEdge;

}

// FIXME: taken from GrVkPipelineState; refactor.
static uint32_t get_blend_info_key(const GrPipeline& pipeline) {
    GrXferProcessor::BlendInfo blendInfo = pipeline.getXferProcessor().getBlendInfo();

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

class Desc : public GrProgramDesc {
public:
    static bool Build(Desc* desc,
                      GrRenderTarget* rt,
                      const GrPipeline& pipeline,
                      const GrPrimitiveProcessor& primProc,
                      GrPrimitiveType primitiveType,
                      bool hasPoints,
                      bool hasDepthStencil,
                      GrGpu* gpu) {
        if (!GrProgramDesc::Build(desc, rt, primProc, hasPoints, pipeline, gpu)) {
            return false;
        }
        GrProcessorKeyBuilder b(&desc->key());

        GrStencilSettings stencil;
        stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(), 8);
        stencil.genKey(&b);
        b.add32(rt->config());
        b.add32(static_cast<int32_t>(hasDepthStencil));
        b.add32(get_blend_info_key(pipeline));
        b.add32(static_cast<uint32_t>(primitiveType));
        return true;
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
        , fCompiler(new SkSL::Compiler())
        , fUniformRingBuffer(this, dawn::BufferUsageBit::Uniform)
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
            GrRenderTarget* rt, GrSurfaceOrigin origin, const SkRect& bounds,
            const GrOpsRenderPass::LoadAndStoreInfo& colorInfo,
            const GrOpsRenderPass::StencilLoadAndStoreInfo& stencilInfo,
            const SkTArray<GrTextureProxy*, true>& sampledProxies) {
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
        SkASSERT(!"uploading to non-texture unimplemented");
        return false;
    }
    texture->upload(texels, mipLevelCount, SkIRect::MakeXYWH(left, top, width, height),
                    this->getCopyEncoder());
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
                                            int mipLevelCount,
                                            uint32_t levelClearMask) {
    SkASSERT(!levelClearMask);
    dawn::TextureFormat format;
    if (!backendFormat.asDawnFormat(&format)) {
        return nullptr;
    }

    GrMipMapsStatus mipMapsStatus =
        mipLevelCount > 1 ? GrMipMapsStatus::kDirty : GrMipMapsStatus::kNotAllocated;

    return GrDawnTexture::Make(this, { desc.fWidth, desc.fHeight },
                                       desc.fConfig, format, renderable,
                                       renderTargetSampleCnt, budgeted, mipLevelCount,
                                       mipMapsStatus);
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
    this->flush();
}

#endif

void GrDawnGpu::flush() {
    this->flushCopyEncoder();
    fQueue.Submit(fCommandBuffers.size(), &fCommandBuffers.front());
    fCommandBuffers.clear();
    fStagingManager.mapBusyList();
    fDevice.Tick();
}

void GrDawnGpu::onFinishFlush(GrSurfaceProxy*[], int n, SkSurface::BackendSurfaceAccess access,
                              const GrFlushInfo& info, const GrPrepareForExternalIORequests&) {
    this->flush();
}

static dawn::Texture get_dawn_texture_from_surface(GrSurface* src) {
    if (auto rt = static_cast<GrDawnRenderTarget*>(src->asRenderTarget())) {
        return rt->texture();
    } else if (auto t = static_cast<GrDawnTexture*>(src->asTexture())) {
        return t->texture();
    } else {
        return nullptr;
    }
}

bool GrDawnGpu::onCopySurface(GrSurface* dst,
                              GrSurface* src,
                              const SkIRect& srcRect,
                              const SkIPoint& dstPoint) {
    dawn::Texture srcTexture = get_dawn_texture_from_surface(src);
    dawn::Texture dstTexture = get_dawn_texture_from_surface(dst);
    if (!srcTexture || !dstTexture) {
        return false;
    }

    uint32_t width = srcRect.width(), height = srcRect.height();

    dawn::TextureCopyView srcTextureView, dstTextureView;
    srcTextureView.texture = srcTexture;
    srcTextureView.origin = {(uint32_t) srcRect.x(), (uint32_t) srcRect.y(), 0};
    dstTextureView.texture = dstTexture;
    dstTextureView.origin = {(uint32_t) dstPoint.x(), (uint32_t) dstPoint.y(), 0};

    dawn::Extent3D copySize = {width, height, 1};
    this->getCopyEncoder().CopyTextureToTexture(&srcTextureView, &dstTextureView, &copySize);
    return true;
}

static void callback(DawnBufferMapAsyncStatus status, const void* data, uint64_t dataLength,
                     void* userdata) {
    (*reinterpret_cast<const void**>(userdata)) = data;
}

bool GrDawnGpu::onReadPixels(GrSurface* surface, int left, int top, int width, int height,
                             GrColorType surfaceColorType, GrColorType dstColorType, void* buffer,
                             size_t rowBytes) {
    dawn::Texture tex = get_dawn_texture_from_surface(surface);

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
    Desc desc;
    if (!Desc::Build(&desc, rt, pipeline, primProc, primitiveType, hasPoints, hasDepthStencil,
                     this)) {
        return nullptr;
    }

    if (sk_sp<GrDawnProgram>* program = fRenderPipelineCache.find(desc)) {
        return *program;
    }

    dawn::TextureFormat colorFormat;
    SkAssertResult(GrPixelConfigToDawnFormat(rt->config(), &colorFormat));
    dawn::TextureFormat stencilFormat = dawn::TextureFormat::Depth24PlusStencil8;

    sk_sp<GrDawnProgram> program = GrDawnProgramBuilder::Build(
        this, rt, origin, pipeline, primProc, primProcProxies, primitiveType, colorFormat,
        hasDepthStencil, stencilFormat, &desc);
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
    desc.compare = dawn::CompareFunction::Never;
    dawn::Sampler sampler = device().CreateSampler(&desc);
    fSamplers.insert(std::pair<GrSamplerState, dawn::Sampler>(samplerState, sampler));
    return sampler;
}

GrDawnRingBuffer::Slice GrDawnGpu::allocateUniformRingBufferSlice(int size) {
    return fUniformRingBuffer.allocate(size);
}

GrDawnStagingBuffer* GrDawnGpu::getStagingBuffer(size_t size) {
    return fStagingManager.findOrCreateStagingBuffer(size);
}

void GrDawnGpu::appendCommandBuffer(dawn::CommandBuffer commandBuffer) {
    if (commandBuffer) {
        fCommandBuffers.push_back(commandBuffer);
    }
}

dawn::CommandEncoder GrDawnGpu::getCopyEncoder() {
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
