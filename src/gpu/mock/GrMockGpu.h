/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGpu.h"
#include "GrSemaphore.h"

class GrPipeline;

struct GrMockOptions {
    GrMockOptions() {
        // By default RGBA_8888 is textureable and renderable and A8 is texturable.
        fConfigOptions[kRGBA_8888_GrPixelConfig].fRenderable[0] = true;
        fConfigOptions[kRGBA_8888_GrPixelConfig].fTexturable = true;
        fConfigOptions[kAlpha_8_GrPixelConfig].fTexturable = true;
    }


    struct ConfigOptions {
        bool fRenderable[2] = {false, false};
        bool fTexturable = false;
    };

    int fMaxTextureSize = 16384;
    int fMaxRenderTargetSize = 16384;
    int fMaxVertexAttributes = 16;
    ConfigOptions fConfigOptions[kGrPixelConfigCnt];
};

class GrMockCaps : public GrCaps {
public:
    GrMockCaps(const GrContextOptions& contextOptions, const GrMockOptions& options) : INHERITED(contextOptions), fOptions(options) {
        fBufferMapThreshold = SK_MaxS32;
        fMaxTextureSize = options.fMaxTextureSize;
        fMaxRenderTargetSize = SkTMin(options.fMaxRenderTargetSize, fMaxTextureSize);
        fMaxVertexAttributes = options.fMaxVertexAttributes;
        fShaderCaps.reset(new GrShaderCaps(contextOptions));
    }
    bool isConfigTexturable(GrPixelConfig config) const override {
        return fOptions.fConfigOptions[config].fTexturable;
    }
    bool isConfigRenderable(GrPixelConfig config, bool withMSAA) const override {
        return fOptions.fConfigOptions[config].fTexturable;
    }
    bool canConfigBeImageStorage(GrPixelConfig) const override { return false; }
    bool initDescForDstCopy(const GrRenderTargetProxy* src, GrSurfaceDesc* desc,
                            bool* rectsMustMatch, bool* disallowSubrect) const override {
        return false;
    }

private:
    GrMockOptions fOptions;
    typedef GrCaps INHERITED;
};

class GrMockGpu;

class GrMockTexture : public GrTexture {
public:
    GrMockTexture(GrMockGpu* gpu, SkBudgeted, const GrSurfaceDesc&, bool hasMipLevels);
    ~GrMockTexture() override {
        if (fReleaseProc) {
            fReleaseProc(fReleaseCtx);
        }
    }
    GrBackendObject getTextureHandle() const override { return 0; }
    void textureParamsModified() override {}
    void setRelease(ReleaseProc proc, ReleaseCtx ctx) override {
        fReleaseProc = proc;
        fReleaseCtx = ctx;
    }

protected:
    // constructor for subclasses
    GrMockTexture(GrMockGpu* gpu, const GrSurfaceDesc&, bool hasMipLevels);

private:
    ReleaseProc fReleaseProc;
    ReleaseCtx fReleaseCtx;
};

class GrMockTextureRenderTarget : public GrRenderTarget, public GrMockTexture {
public:
    GrMockTextureRenderTarget(GrMockGpu*, SkBudgeted, const GrSurfaceDesc&, bool hasMipLevels);
    ResolveType getResolveType() const override { return kCanResolve_ResolveType; }
    GrBackendObject getRenderTargetHandle() const override { return 0; }
    bool canAttemptStencilAttachment() const override { return true; }
    bool completeStencilAttachment() override { return true; }
};

class GrMockStencilAttachment : public GrStencilAttachment {
public:
    GrMockStencilAttachment(GrMockGpu*, int width, int height, int bits, int sampleCnt);
private:
    size_t onGpuMemorySize() const override {
        return SkTMax(1, (int)(this->bits() / sizeof(char))) * this->width() * this->height();
    }
};

class GrMockBuffer : public GrBuffer {
public:
    GrMockBuffer(GrMockGpu*, size_t sizeInBytes, GrBufferType, GrAccessPattern);
private:
    void onMap() override {}
    void onUnmap() override {}
    bool onUpdateData(const void* src, size_t srcSizeInBytes) override { return true; }
};

class GrMockGpuCommandBuffer : public GrGpuCommandBuffer {
public:
    GrMockGpuCommandBuffer(GrMockGpu*);

    GrGpu* gpu() override;
    void inlineUpload(GrOpFlushState*, GrDrawOp::DeferredUploadFn&, GrRenderTarget*) override {}
    void discard(GrRenderTarget*) override {}
    void end() override {}

    int numDraws() const { return fNumDraws; }

private:
    void onSubmit() override;
    void onDraw(const GrPipeline&, const GrPrimitiveProcessor&,
                            const GrMesh[],
                            const GrPipeline::DynamicState[],
                            int meshCount,
                            const SkRect& bounds) override { ++fNumDraws; }
    void onClear(GrRenderTarget*, const GrFixedClip&, GrColor) override {}
    void onClearStencilClip(GrRenderTarget*, const GrFixedClip&, bool insideStencilMask) override {}
    GrRenderTarget* renderTarget() override { return nullptr; }

    GrMockGpu* fGpu;
    int fNumDraws = 0;
};


class GrMockGpu : public GrGpu {
public:
    static GrGpu* Create(GrBackendContext backendContext, const GrContextOptions& contextOptions,
                         GrContext* context) {
        SkASSERT((void*)backendContext == nullptr);
        static const GrMockOptions kOptions = GrMockOptions();
        return new GrMockGpu(context, kOptions, contextOptions);
    }

    ~GrMockGpu() override {}

    bool onGetReadPixelsInfo(GrSurface* srcSurface, int readWidth, int readHeight, size_t rowBytes,
                             GrPixelConfig readConfig, DrawPreference*,
                             ReadPixelTempDrawInfo*) override { return true; }

    bool onGetWritePixelsInfo(GrSurface* dstSurface, int width, int height,
                              GrPixelConfig srcConfig, DrawPreference*,
                              WritePixelTempDrawInfo*) override { return true; }

    bool onCopySurface(GrSurface* dst,
                       GrSurface* src,
                       const SkIRect& srcRect,
                       const SkIPoint& dstPoint) override { return true; }

    void onQueryMultisampleSpecs(GrRenderTarget* rt, const GrStencilSettings&,
                                 int* effectiveSampleCnt, SamplePattern*) override {
        *effectiveSampleCnt = rt->numStencilSamples();
    }

    GrGpuCommandBuffer* createCommandBuffer(const GrGpuCommandBuffer::LoadAndStoreInfo&,
                                            const GrGpuCommandBuffer::LoadAndStoreInfo&) override {
        return new GrMockGpuCommandBuffer(this);
    }

    GrFence SK_WARN_UNUSED_RESULT insertFence() override { return 0; }
    bool waitFence(GrFence, uint64_t) override { return true; }
    void deleteFence(GrFence) const override {}

    sk_sp<GrSemaphore> SK_WARN_UNUSED_RESULT makeSemaphore(bool isOwned) override {
        return nullptr;
    }
    sk_sp<GrSemaphore> wrapBackendSemaphore(const GrBackendSemaphore& semaphore,
                                            GrWrapOwnership ownership) override { return nullptr; }
    void insertSemaphore(sk_sp<GrSemaphore> semaphore, bool flush) override {}
    void waitSemaphore(sk_sp<GrSemaphore> semaphore) override {}
    sk_sp<GrSemaphore> prepareTextureForCrossContextUsage(GrTexture*) override { return nullptr; }

    void submitCommandBuffer(const GrMockGpuCommandBuffer* cmdBuffer) {
        for (int i = 0; i < cmdBuffer->numDraws(); ++i) {
            fStats.incNumDraws();
        }
    }

private:
    GrMockGpu(GrContext* context, const GrMockOptions& options, const GrContextOptions& contextOptions) : INHERITED(context) {
        fCaps.reset(new GrMockCaps(contextOptions, options));
    }

    void onResetContext(uint32_t resetBits) override {}

    void xferBarrier(GrRenderTarget*, GrXferBarrierType) override {}

    sk_sp<GrTexture> onCreateTexture(const GrSurfaceDesc& desc, SkBudgeted budgeted,
                                     const SkTArray<GrMipLevel>& texels) override {
        bool hasMipLevels = texels.count() > 1;
        if (desc.fFlags & kRenderTarget_GrSurfaceFlag) {
            return sk_sp<GrTexture>(new GrMockTextureRenderTarget(this, budgeted, desc, hasMipLevels));
        }
        return sk_sp<GrTexture>(new GrMockTexture(this, budgeted, desc, hasMipLevels));
    }

    sk_sp<GrTexture> onWrapBackendTexture(const GrBackendTexture&,
                                          GrSurfaceOrigin,
                                          GrBackendTextureFlags,
                                          int sampleCnt,
                                          GrWrapOwnership) override {
        return nullptr;
    }

    sk_sp<GrRenderTarget> onWrapBackendRenderTarget(const GrBackendRenderTarget&,
                                                    GrSurfaceOrigin) override {
        return nullptr;
    }

    sk_sp<GrRenderTarget> onWrapBackendTextureAsRenderTarget(const GrBackendTexture&,
                                                             GrSurfaceOrigin,
                                                             int sampleCnt) override {
        return nullptr;
    }

    GrBuffer* onCreateBuffer(size_t sizeInBytes, GrBufferType type, GrAccessPattern accessPattern, const void*) override {
        return new GrMockBuffer(this, sizeInBytes, type, accessPattern);
    }

    gr_instanced::InstancedRendering* onCreateInstancedRendering() override { return nullptr; }

    bool onReadPixels(GrSurface* surface,
                      int left, int top, int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override {
        return true;
    }

    bool onWritePixels(GrSurface* surface,
                       int left, int top, int width, int height,
                       GrPixelConfig config, const SkTArray<GrMipLevel>& texels) override {
        return true;
    }

    bool onTransferPixels(GrTexture* texture,
                          int left, int top, int width, int height,
                          GrPixelConfig config, GrBuffer* transferBuffer,
                          size_t offset, size_t rowBytes) override {
        return true;
    }

    void onResolveRenderTarget(GrRenderTarget* target) override { return; }

    GrStencilAttachment* createStencilAttachmentForRenderTarget(const GrRenderTarget* rt,
                                                                int width,
                                                                int height) override {
        static constexpr int kBits = 8;
        fStats.incStencilAttachmentCreates();
        return new GrMockStencilAttachment(this, width, height, kBits, rt->numColorSamples());
    }

    void clearStencil(GrRenderTarget* target) override  {}

    GrBackendObject createTestingOnlyBackendTexture(void* pixels, int w, int h,
                                                    GrPixelConfig config, bool isRT) override {
        return 0;
    }
    bool isTestingOnlyBackendTexture(GrBackendObject ) const override { return false; }
    void deleteTestingOnlyBackendTexture(GrBackendObject, bool abandonTexture) override {}

    typedef GrGpu INHERITED;
};

GrMockTexture::GrMockTexture(GrMockGpu* gpu, const GrSurfaceDesc& desc, bool hasMipLevels) : GrSurface(gpu, desc), GrTexture(gpu, desc, kITexture2DSampler_GrSLType, GrSamplerParams::kMipMap_FilterMode, hasMipLevels), fReleaseProc(nullptr), fReleaseCtx(nullptr) {}

GrMockTexture::GrMockTexture(GrMockGpu* gpu, SkBudgeted budgeted, const GrSurfaceDesc& desc, bool hasMipLevels) : GrMockTexture(gpu, desc, hasMipLevels) {
    this->registerWithCache(budgeted);
}

GrMockTextureRenderTarget::GrMockTextureRenderTarget(GrMockGpu* gpu, SkBudgeted budgeted, const GrSurfaceDesc& desc, bool hasMipLevels) : GrSurface(gpu, desc), GrRenderTarget(gpu, desc), GrMockTexture(gpu, desc, hasMipLevels) {
    this->registerWithCache(budgeted);
}

GrMockStencilAttachment::GrMockStencilAttachment(GrMockGpu* gpu, int width, int height, int bits, int sampleCnt) : GrStencilAttachment(gpu, width, height, bits, sampleCnt) {
    this->registerWithCache(SkBudgeted::kYes);
}

GrMockBuffer::GrMockBuffer(GrMockGpu* gpu, size_t sizeInBytes, GrBufferType type, GrAccessPattern accessPattern) : GrBuffer(gpu, sizeInBytes, type, accessPattern) {
    this->registerWithCache(SkBudgeted::kYes);
}

GrMockGpuCommandBuffer::GrMockGpuCommandBuffer(GrMockGpu* gpu) : fGpu(gpu) {}

GrGpu* GrMockGpuCommandBuffer::gpu() { return fGpu; }

void GrMockGpuCommandBuffer::onSubmit() { fGpu->submitCommandBuffer(this); }
