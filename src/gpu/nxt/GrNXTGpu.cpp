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

const int kMaxRenderPassEntries = 1024;
const int kMaxRenderPipelineEntries = 1024;

namespace {

nxt::LoadOp to_nxt_load_op(GrLoadOp loadOp) {
    switch (loadOp) {
        case GrLoadOp::kLoad:
            return nxt::LoadOp::Load;
        case GrLoadOp::kDiscard:
            // FIXME: NXT doesn't support discard (yet), so fall-through to clear
        case GrLoadOp::kClear:
            return nxt::LoadOp::Clear;
        default:
            SK_ABORT("Invalid LoadOp");
            return nxt::LoadOp::Load;
    }
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

sk_sp<GrGpu> GrNXTGpu::Make(GrBackendContext backendContext, const GrContextOptions& options,
                            GrContext* context) {
    const auto* backend = reinterpret_cast<const GrNXTBackendContext*>(backendContext);
    return Make(sk_ref_sp(backend), options, context);
}

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
        , fDevice(backendContext->fDevice.Clone())
        , fQueue(backendContext->fQueue.Clone())
        , fRenderPassCache(kMaxRenderPassEntries)
        , fRenderPipelineCache(kMaxRenderPipelineEntries)
        , fUniformRingBuffer(this, nxt::BufferUsageBit::Uniform)
        , fCopyBuilder(fDevice.CreateCommandBufferBuilder()) {
    fCompiler = new SkSL::Compiler();
    fCaps.reset(new GrNXTCaps(options));
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
    if (fCopyBuilder) {
        nxt::CommandBuffer cmdBuf = fCopyBuilder.GetResult();
        this->queue().Submit(1, &cmdBuf);
        fCopyBuilder = this->device().CreateCommandBufferBuilder();
    }
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
bool GrNXTGpu::onGetWritePixelsInfo(GrSurface* dstSurface, GrSurfaceOrigin dstOrigin,
                                    int width, int height,
                                    GrColorType colorType, DrawPreference* drawPreference,
                                    WritePixelTempDrawInfo* tempDrawInfo) {
    // We don't want to introduce an sRGB conversion if we trigger a draw.
    auto srcConfigSRGBEncoded = GrPixelConfigIsSRGBEncoded(dstSurface->config());
    GrRenderTarget* renderTarget = dstSurface->asRenderTarget();

    // Start off assuming no swizzling
    tempDrawInfo->fSwizzle = GrSwizzle::RGBA();
    tempDrawInfo->fWriteColorType = colorType;

    // These settings we will always want if a temp draw is performed. Initially set the config
    // to srcConfig, though that may be modified if we decide to do a R/B swap
    tempDrawInfo->fTempSurfaceDesc.fFlags = kNone_GrSurfaceFlags;
    tempDrawInfo->fTempSurfaceDesc.fWidth = width;
    tempDrawInfo->fTempSurfaceDesc.fHeight = height;
    tempDrawInfo->fTempSurfaceDesc.fSampleCnt = 0;

    if (GrPixelConfigToColorType(dstSurface->config()) == colorType) {
        // We only support writing pixels to textures. Forcing a draw lets us write to pure RTs.
        if (!dstSurface->asTexture()) {
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        }
        // If the dst is MSAA, we have to draw, or we'll just be writing to the resolve target.
        if (renderTarget && renderTarget->numColorSamples() > 1) {
            ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);
        }
        return true;
    }

    // Any config change requires a draw
    ElevateDrawPreference(drawPreference, kRequireDraw_DrawPreference);

    auto srcAsConfig = GrColorTypeToPixelConfig(colorType, srcConfigSRGBEncoded);
    bool configsAreRBSwaps = GrPixelConfigSwapRAndB(srcAsConfig) == dstSurface->config();

    if (!this->caps()->isConfigTexturable(srcAsConfig) && configsAreRBSwaps) {
        tempDrawInfo->fTempSurfaceDesc.fConfig = dstSurface->config();
        tempDrawInfo->fSwizzle = GrSwizzle::BGRA();
        tempDrawInfo->fWriteColorType = GrPixelConfigToColorType(dstSurface->config());
    }
    return true;
}

bool GrNXTGpu::onWritePixels(GrSurface* surface, GrSurfaceOrigin origin,
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
                                           GrSurfaceOrigin texelsOrigin, const GrMipLevel texels[],
                                           int mipLevels) {
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
    const GrNXTImageInfo* info = tex.getNXTImageInfo();
    if (!info) {
        return nullptr;
    }
    if (!info->fTexture) {
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

GrBackendTexture GrNXTGpu::createTestingOnlyBackendTexture(void* srcData, int w, int h,
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

void GrNXTGpu::deleteTestingOnlyBackendTexture(GrBackendTexture* tex, bool abandon) {
    SkASSERT(!"unimplemented");
}

void GrNXTGpu::onFinishFlush(bool insertedSemaphore) {
}

void GrNXTGpu::clearStencil(GrRenderTarget* target, int clearValue) {
    // FIXME: this clear could be integrated into the next draw's subpass
    auto attachment = static_cast<GrNXTStencilAttachment*>(target->renderTargetPriv().getStencilAttachment());
    auto renderPass = device().CreateRenderPassBuilder()
        .SetAttachmentCount(1)
        .AttachmentSetFormat(0, nxt::TextureFormat::D32FloatS8Uint)
        .AttachmentSetDepthStencilLoadOps(0, nxt::LoadOp::Clear, nxt::LoadOp::Clear)
        .SetSubpassCount(1)
        .SubpassSetDepthStencilAttachment(0, 0)
        .GetResult();
    auto framebuffer = device().CreateFramebufferBuilder()
        .SetRenderPass(renderPass)
        .SetDimensions(target->width(), target->height())
        .SetAttachment(0, attachment->view())
        .GetResult();
    framebuffer.AttachmentSetClearDepthStencil(0, 0, clearValue);
    auto commandBuffer = device().CreateCommandBufferBuilder()
            .BeginRenderPass(renderPass, framebuffer)
            .BeginRenderSubpass()
            .EndRenderSubpass()
            .EndRenderPass()
            .GetResult();
    queue().Submit(1, &commandBuffer);
}

bool GrNXTGpu::onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                             GrSurface* src, GrSurfaceOrigin srcOrigin,
                             const SkIRect& srcRect,
                             const SkIPoint& dstPoint) {
    SkASSERT(!"unimplemented");
    return false;
}

bool GrNXTGpu::onGetReadPixelsInfo(GrSurface* srcSurface, GrSurfaceOrigin srcOrigin,
                                   int width, int height, size_t rowBytes,
                                   GrColorType colorType, DrawPreference* drawPreference,
                                   ReadPixelTempDrawInfo* tempDrawInfo) {
    SkASSERT(!"unimplemented");
    return false;
}

bool GrNXTGpu::onReadPixels(GrSurface* surface, GrSurfaceOrigin origin,
                            int left, int top, int width, int height,
                            GrColorType colorType,
                            void* buffer,
                            size_t rowBytes) {
    SkASSERT(!"unimplemented");
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

struct RenderPassValue {
    RenderPassValue(bool hasStencil, GrPixelConfig rtConfig, GrLoadOp colorLoadOp,
                    GrLoadOp stencilLoadOp) {
        memset(this, 0, sizeof(*this));
        fHasStencil = hasStencil;
        fRTConfig = rtConfig;
        fColorLoadOp = colorLoadOp;
        fStencilLoadOp = stencilLoadOp;
    }
    uint32_t hash() {
        return SkOpts::hash_fn(this, sizeof(*this), 0);
    }
    bool          fHasStencil;
    GrPixelConfig fRTConfig;
    GrLoadOp      fColorLoadOp;
    GrLoadOp      fStencilLoadOp;
};

nxt::RenderPass GrNXTGpu::getOrCreateRenderPass(GrRenderTarget* rt,
                                                GrLoadOp colorLoadOp,
                                                GrLoadOp stencilLoadOp,
                                                uint32_t* hashOut) {
    GrStencilAttachment* stencilAttachment = rt->renderTargetPriv().getStencilAttachment();
    RenderPassValue value(nullptr != stencilAttachment, rt->config(), colorLoadOp, stencilLoadOp);
    uint32_t hash = value.hash();
    if (hashOut) {
        *hashOut = hash;
    }
    if (nxt::RenderPass* renderPass = fRenderPassCache.find(hash)) {
        return renderPass->Clone();
    }

    auto renderPassBuilder = fDevice.CreateRenderPassBuilder()
        .SetAttachmentCount(stencilAttachment ? 2 : 1)
        .AttachmentSetFormat(0, GrPixelConfigToNXTFormat(rt->config()))
        .AttachmentSetColorLoadOp(0, to_nxt_load_op(colorLoadOp))
        .SetSubpassCount(1)
        .SubpassSetColorAttachment(0, 0, 0)
        .Clone();
    if (stencilAttachment) {
        renderPassBuilder
            .AttachmentSetFormat(1, nxt::TextureFormat::D32FloatS8Uint)
            .AttachmentSetDepthStencilLoadOps(1, to_nxt_load_op(stencilLoadOp),
                                                 to_nxt_load_op(stencilLoadOp))
            .SubpassSetDepthStencilAttachment(0, 1);
    }
    nxt::RenderPass renderPass = renderPassBuilder.GetResult();
    fRenderPassCache.insert(hash, renderPass.Clone());
    return renderPass;
}

sk_sp<GrNXTProgram> GrNXTGpu::getOrCreateRenderPipeline(nxt::RenderPass renderPass,
                                                        uint32_t renderPassHash,
                                                        const GrPipeline& pipeline,
                                                        const GrPrimitiveProcessor& primProc,
                                                        bool hasPoints,
                                                        GrPrimitiveType primitiveType) {
    GrProgramDesc desc;
    GrProgramDesc::Build(&desc, primProc, hasPoints, pipeline, *this->caps()->shaderCaps());
    desc.finalize();
    SkTArray<uint8_t, true> keyData;
    keyData.push_back_n(desc.keyLength(), (const unsigned char*) desc.asKey());
    GrProcessorKeyBuilder b(&keyData);
    b.add32(renderPassHash);
    GrStencilSettings stencil;
    stencil.reset(*pipeline.getUserStencil(), pipeline.hasStencilClip(), 8);
    stencil.genKey(&b);
    b.add32(get_blend_info_key(pipeline));
    b.add32(static_cast<uint32_t>(primitiveType));
    uint32_t hash = SkOpts::hash_fn(keyData.begin(), keyData.count(), 0);

    if (sk_sp<GrNXTProgram>* program = fRenderPipelineCache.find(hash)) {
        return *program;
    }

    sk_sp<GrNXTProgram> program = GrNXTProgramBuilder::Build(
        this, pipeline, primProc, primitiveType, renderPass.Clone(), &desc);
    fRenderPipelineCache.insert(hash, program);
    return program;
}

GrNXTRingBuffer::Slice GrNXTGpu::allocateUniformRingBufferSlice(int size) {
    return fUniformRingBuffer.allocate(size);
}
