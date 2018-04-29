/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTGpu_DEFINED
#define GrNXTGpu_DEFINED

#include "GrGpu.h"
#include "nxt/nxtcpp.h"
#include "nxt/GrNXTBackendContext.h"
#include "nxt/GrNXTRingBuffer.h"
#include "nxt/GrNXTStagingManager.h"
#include "SkLRUCache.h"

#include <unordered_map>

class GrPipeline;
struct GrNXTProgram;

namespace SkSL {
    class Compiler;
}

class GrNXTGpu : public GrGpu {
public:
    static sk_sp<GrGpu> Make(sk_sp<const GrNXTBackendContext>, const GrContextOptions&, GrContext*);
    GrNXTGpu(GrContext* context, const GrContextOptions& options, sk_sp<const GrNXTBackendContext>);

    ~GrNXTGpu() override;

    void disconnect(DisconnectType) override;

    nxt::Device device() const { return fDevice.Clone(); }
    nxt::Queue  queue() const { return fQueue.Clone(); }

    bool onGetReadPixelsInfo(GrSurface*, GrSurfaceOrigin, int width, int height,
                             size_t rowBytes, GrColorType, DrawPreference*,
                             ReadPixelTempDrawInfo*) override;

    bool onGetWritePixelsInfo(GrSurface*, GrSurfaceOrigin, int width, int height,
                              GrColorType, DrawPreference*, WritePixelTempDrawInfo*) override;

    bool onCopySurface(GrSurface* dst, GrSurfaceOrigin dstOrigin,
                       GrSurface* src, GrSurfaceOrigin srcOrigin,
                       const SkIRect& srcRect, const SkIPoint& dstPoint,
                       bool canDiscardOutsideDstRect) override;

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

#if GR_TEST_UTILS
    GrBackendTexture createTestingOnlyBackendTexture(const void* pixels, int w, int h,
                                                     GrPixelConfig config,
                                                     bool isRenderTarget,
                                                     GrMipMapped) override;
    bool isTestingOnlyBackendTexture(const GrBackendTexture&) const override;
    void deleteTestingOnlyBackendTexture(const GrBackendTexture&) override;

    GrBackendRenderTarget createTestingOnlyBackendRenderTarget(int w, int h, GrColorType,
                                                               GrSRGBEncoded) override;
    void deleteTestingOnlyBackendRenderTarget(const GrBackendRenderTarget&) override;

    void testingOnly_flushGpuAndSync() override;
#endif

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget*,
                                                                int width,
                                                                int height) override;

    void clearStencil(GrRenderTarget* target, int clearValue) override;

    GrGpuRTCommandBuffer* createCommandBuffer(
            GrRenderTarget*, GrSurfaceOrigin,
            const GrGpuRTCommandBuffer::LoadAndStoreInfo&,
            const GrGpuRTCommandBuffer::StencilLoadAndStoreInfo&) override;

    GrGpuTextureCommandBuffer* createCommandBuffer(GrTexture*, GrSurfaceOrigin) override;

    SkSL::Compiler* shaderCompiler() const {
        return fCompiler;
    }

    void onResolveRenderTarget(GrRenderTarget* target) override {
    }

    GrFence SK_WARN_UNUSED_RESULT insertFence() override;
    bool waitFence(GrFence, uint64_t timeout) override;
    void deleteFence(GrFence) const override;

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned = true) override;
    sk_sp<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                            GrResourceProvider::SemaphoreWrapType wrapType,
                                            GrWrapOwnership ownership) override;
    void insertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush = false) override;
    void waitSemaphore(sk_sp<GrSemaphore> semaphore) override;

    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override;

    nxt::RenderPass getOrCreateRenderPass(GrRenderTarget*,
                                          GrLoadOp colorLoadOp,
                                          GrLoadOp stencilLoadOp);

    nxt::Framebuffer getOrCreateFramebuffer(const nxt::RenderPass&,
                                            GrRenderTarget*,
                                            GrColor clearColor);

    sk_sp<GrNXTProgram> getOrCreateRenderPipeline(nxt::RenderPass renderPass,
                                                  const GrPipeline&,
                                                  const GrPrimitiveProcessor&,
                                                  bool hasPoints,
                                                  GrPrimitiveType primitiveType);

    nxt::Sampler getOrCreateSampler(const GrSamplerState& samplerState);

    GrNXTRingBuffer::Slice allocateUniformRingBufferSlice(int size);
    GrNXTStagingBuffer* getStagingBuffer(size_t size, nxt::BufferUsageBit usage);
    void setReadPixelsPtr(const void* ptr) { fReadPixelsPtr = ptr; }
    GrNXTStagingManager* getStagingManager() { return &fStagingManager; }
    nxt::CommandBufferBuilder getCopyBuilder() { return fCopyBuilder.Clone(); }
    void appendCommandBuffer(nxt::CommandBuffer commandBuffer);

private:
    void onResetContext(uint32_t resetBits) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                     const GrMipLevel texels[], int mipLevelCount) override;

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&, GrWrapOwnership) override;
    sk_sp<GrTexture> onWrapRenderableBackendTexture(const GrBackendTexture&,
                                                    int sampleCnt,
                                                    GrWrapOwnership) override;
    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&) override;

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             int sampleCnt) override;

    GrBuffer* onCreateBuffer(size_t size, GrBufferType type, GrAccessPattern,
                             const void* data) override;

    bool onReadPixels(GrSurface* surface, GrSurfaceOrigin,
                      int left, int top, int width, int height,
                      GrColorType, void* buffer, size_t rowBytes) override;

    bool onWritePixels(GrSurface* surface, GrSurfaceOrigin,
                       int left, int top, int width, int height,
                       GrColorType, const GrMipLevel texels[], int mipLevelCount) override;

    bool onTransferPixels(GrTexture*, int left, int top, int width, int height,
                          GrColorType colorType, GrBuffer* transferBuffer,
                          size_t offset, size_t rowBytes) override;

    void onFinishFlush(bool insertedSemaphores) override;

    nxt::Device                                  fDevice;
    nxt::Queue                                   fQueue;

    SkSL::Compiler* fCompiler;

    GrNXTRingBuffer                              fUniformRingBuffer;
    nxt::CommandBufferBuilder                    fCopyBuilder;
    const void*                                  fReadPixelsPtr = nullptr;

    struct RenderPassValue {
        RenderPassValue(bool hasStencil, GrPixelConfig rtConfig, GrLoadOp colorLoadOp,
                        GrLoadOp stencilLoadOp) {
            memset(this, 0, sizeof(*this));
            fHasStencil = hasStencil;
            fRTConfig = rtConfig;
            fColorLoadOp = colorLoadOp;
            fStencilLoadOp = stencilLoadOp;
        }
        bool operator==(const RenderPassValue& other) const {
            return fHasStencil == other.fHasStencil
                && fRTConfig == other.fRTConfig
                && fColorLoadOp == other.fColorLoadOp
                && fStencilLoadOp == other.fStencilLoadOp;
        }
        struct Hash {
            uint32_t operator()(const RenderPassValue& value) {
                return SkOpts::hash_fn(&value, sizeof(value), 0);
            }
        };
        bool          fHasStencil;
        GrPixelConfig fRTConfig;
        GrLoadOp      fColorLoadOp;
        GrLoadOp      fStencilLoadOp;
    };

    struct FramebufferValue {
        FramebufferValue(nxtRenderPass renderPass, nxtTexture texture, GrColor clearColor) {
            memset(this, 0, sizeof(*this));
            fRenderPass = renderPass;
            fTexture = texture;
            fClearColor = clearColor;
        }
        bool operator==(const FramebufferValue& other) const {
            return fRenderPass == other.fRenderPass &&
                   fTexture == other.fTexture &&
                   fClearColor == other.fClearColor;
        }
        struct Hash {
            uint32_t operator()(const FramebufferValue& value) {
                return SkOpts::hash_fn(&value, sizeof(value), 0);
            }
        };
        nxtRenderPass     fRenderPass;
        nxtTexture        fTexture;
        GrColor           fClearColor;
    };

    struct ProgramDescHash {
        uint32_t operator()(const GrProgramDesc& desc) const {
            return SkOpts::hash_fn(desc.asKey(), desc.keyLength(), 0);
        }
    };

    struct SamplerHash {
        size_t operator()(const GrSamplerState& samplerState) const {
            return SkOpts::hash_fn(&samplerState, sizeof(samplerState), 0);
        }
    };

    SkLRUCache<RenderPassValue, nxt::RenderPass, RenderPassValue::Hash>  fRenderPassCache;
    SkLRUCache<GrProgramDesc, sk_sp<GrNXTProgram>, ProgramDescHash>    fRenderPipelineCache;
    SkLRUCache<FramebufferValue, nxt::Framebuffer, FramebufferValue::Hash> fFramebufferCache;
    std::unordered_map<GrSamplerState, nxt::Sampler, SamplerHash> fSamplers;
    GrNXTStagingManager fStagingManager;
    std::vector<nxt::CommandBuffer> fCommandBuffers;

    typedef GrGpu INHERITED;
};

#endif
