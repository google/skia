/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrMockTexture_DEFINED
#define GrMockTexture_DEFINED

#include "GrMockGpu.h"
#include "GrRenderTarget.h"
#include "GrRenderTargetPriv.h"
#include "GrTexture.h"
#include "GrTexturePriv.h"
#include "mock/GrMockTypes.h"

class GrMockTexture : public GrTexture {
public:
    GrMockTexture(GrMockGpu* gpu, SkBudgeted budgeted, const GrSurfaceDesc& desc,
                  GrMipMapsStatus mipMapsStatus, const GrMockTextureInfo& info)
            : GrMockTexture(gpu, desc, mipMapsStatus, info) {
        this->registerWithCache(budgeted);
    }

    GrMockTexture(GrMockGpu* gpu, const GrSurfaceDesc& desc, GrMipMapsStatus mipMapsStatus,
                  const GrMockTextureInfo& info, GrWrapCacheable cacheable, GrIOType ioType)
            : GrMockTexture(gpu, desc, mipMapsStatus, info) {
        if (ioType == kRead_GrIOType) {
            this->setReadOnly();
        }
        this->registerWithCacheWrapped(cacheable);
    }

    ~GrMockTexture() override {}

    GrBackendTexture getBackendTexture() const override {
        return GrBackendTexture(this->width(), this->height(), this->texturePriv().mipMapped(),
                                fInfo);
    }

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMock(fInfo.fConfig);
    }

    void textureParamsModified() override {}

    void setIdleProc(IdleProc proc, void* context) override {
        fIdleProc = proc;
        fIdleProcContext = context;
    }
    void* idleContext() const override { return fIdleProcContext; }

protected:
    // constructor for subclasses
    GrMockTexture(GrMockGpu* gpu, const GrSurfaceDesc& desc, GrMipMapsStatus mipMapsStatus,
                  const GrMockTextureInfo& info)
            : GrSurface(gpu, desc)
            , INHERITED(gpu, desc, GrTextureType::k2D, mipMapsStatus)
            , fInfo(info) {}

    void onRelease() override {
        INHERITED::onRelease();
    }

    void onAbandon() override {
        INHERITED::onAbandon();
    }

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
        return false;
    }

    // protected so that GrMockTextureRenderTarget can call this to avoid "inheritance via
    // dominance" warning.
    void willRemoveLastRefOrPendingIO() override {
        if (fIdleProc) {
            fIdleProc(fIdleProcContext);
            fIdleProc = nullptr;
            fIdleProcContext = nullptr;
        }
    }

private:
    void onSetRelease(sk_sp<GrReleaseProcHelper> releaseHelper) override {}

    GrMockTextureInfo fInfo;
    sk_sp<GrReleaseProcHelper> fReleaseHelper;
    IdleProc* fIdleProc = nullptr;
    void* fIdleProcContext = nullptr;

    typedef GrTexture INHERITED;
};

class GrMockRenderTarget : public GrRenderTarget {
public:
    GrMockRenderTarget(GrMockGpu* gpu, SkBudgeted budgeted, const GrSurfaceDesc& desc,
                       const GrMockRenderTargetInfo& info)
            : GrSurface(gpu, desc), INHERITED(gpu, desc), fInfo(info) {
        this->registerWithCache(budgeted);
    }

    enum Wrapped { kWrapped };
    GrMockRenderTarget(GrMockGpu* gpu, Wrapped, const GrSurfaceDesc& desc,
                       const GrMockRenderTargetInfo& info)
            : GrSurface(gpu, desc), INHERITED(gpu, desc), fInfo(info) {
        this->registerWithCacheWrapped(GrWrapCacheable::kNo);
    }

    ResolveType getResolveType() const override { return kCanResolve_ResolveType; }
    bool canAttemptStencilAttachment() const override { return true; }
    bool completeStencilAttachment() override { return true; }

    size_t onGpuMemorySize() const override {
        int numColorSamples = this->numColorSamples();
        if (numColorSamples > 1) {
            // Add one to account for the resolve buffer.
            ++numColorSamples;
        }
        return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                      numColorSamples, GrMipMapped::kNo);
    }

    GrBackendRenderTarget getBackendRenderTarget() const override {
        int numStencilBits = 0;
        if (GrStencilAttachment* stencil = this->renderTargetPriv().getStencilAttachment()) {
            numStencilBits = stencil->bits();
        }
        return {this->width(), this->height(), this->numColorSamples(), numStencilBits, fInfo};
    }

    GrBackendFormat backendFormat() const override {
        return GrBackendFormat::MakeMock(fInfo.fConfig);
    }

protected:
    // constructor for subclasses
    GrMockRenderTarget(GrMockGpu* gpu, const GrSurfaceDesc& desc,
                       const GrMockRenderTargetInfo& info)
            : GrSurface(gpu, desc), INHERITED(gpu, desc), fInfo(info) {}

private:
    void onSetRelease(sk_sp<GrReleaseProcHelper> releaseHelper) override {}

    GrMockRenderTargetInfo fInfo;

    typedef GrRenderTarget INHERITED;
};

class GrMockTextureRenderTarget : public GrMockTexture, public GrMockRenderTarget {
public:
    // Internally created.
    GrMockTextureRenderTarget(GrMockGpu* gpu, SkBudgeted budgeted, const GrSurfaceDesc& desc,
                              GrMipMapsStatus mipMapsStatus, const GrMockTextureInfo& texInfo,
                              const GrMockRenderTargetInfo& rtInfo)
            : GrSurface(gpu, desc)
            , GrMockTexture(gpu, desc, mipMapsStatus, texInfo)
            , GrMockRenderTarget(gpu, desc, rtInfo) {
        this->registerWithCache(budgeted);
    }

    // Renderable wrapped backend texture.
    GrMockTextureRenderTarget(GrMockGpu* gpu, const GrSurfaceDesc& desc,
                              GrMipMapsStatus mipMapsStatus, const GrMockTextureInfo& texInfo,
                              const GrMockRenderTargetInfo& rtInfo, GrWrapCacheable cacheble)
            : GrSurface(gpu, desc)
            , GrMockTexture(gpu, desc, mipMapsStatus, texInfo)
            , GrMockRenderTarget(gpu, desc, rtInfo) {
        this->registerWithCacheWrapped(cacheble);
    }

    GrTexture* asTexture() override { return this; }
    GrRenderTarget* asRenderTarget() override { return this; }
    const GrTexture* asTexture() const override { return this; }
    const GrRenderTarget* asRenderTarget() const override { return this; }

    GrBackendFormat backendFormat() const override {
        return GrMockTexture::backendFormat();
    }

private:
    void onSetRelease(sk_sp<GrReleaseProcHelper> releaseHelper) override {}

    void onAbandon() override {
        GrRenderTarget::onAbandon();
        GrMockTexture::onAbandon();
    }

    void onRelease() override {
        GrRenderTarget::onRelease();
        GrMockTexture::onRelease();
    }

    // We implement this to avoid the inheritance via dominance warning.
    void willRemoveLastRefOrPendingIO() override { GrMockTexture::willRemoveLastRefOrPendingIO(); }

    size_t onGpuMemorySize() const override {
        int numColorSamples = this->numColorSamples();
        if (numColorSamples > 1) {
            // Add one to account for the resolve buffer.
            ++numColorSamples;
        }
        return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                      numColorSamples,
                                      this->texturePriv().mipMapped());
    }

    void computeScratchKey(GrScratchKey* key) const override {
        GrTexturePriv::ComputeScratchKey(this->config(), this->width(), this->height(),
                                         true, this->numStencilSamples(),
                                         this->texturePriv().mipMapped(), key);
    }
};

#endif
