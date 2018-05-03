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

    enum Wrapped { kWrapped };
    GrMockTexture(GrMockGpu* gpu, Wrapped, const GrSurfaceDesc& desc,
                  GrMipMapsStatus mipMapsStatus, const GrMockTextureInfo& info)
            : GrMockTexture(gpu, desc, mipMapsStatus, info) {
        this->registerWithCacheWrapped();
    }

    ~GrMockTexture() override {}

    GrBackendTexture getBackendTexture() const override {
        return GrBackendTexture(this->width(), this->height(), this->texturePriv().mipMapped(),
                                fInfo);
    }

    void textureParamsModified() override {}
    void setRelease(sk_sp<GrReleaseProcHelper> releaseHelper) override {
        fReleaseHelper = std::move(releaseHelper);
    }

protected:
    // constructor for subclasses
    GrMockTexture(GrMockGpu* gpu, const GrSurfaceDesc& desc, GrMipMapsStatus mipMapsStatus,
                  const GrMockTextureInfo& info)
            : GrSurface(gpu, desc)
            , INHERITED(gpu, desc, kTexture2DSampler_GrSLType, GrSamplerState::Filter::kMipMap,
                        mipMapsStatus)
            , fInfo(info) {}

    void onRelease() override {
        this->invokeReleaseProc();
        INHERITED::onRelease();
    }

    void onAbandon() override {
        this->invokeReleaseProc();
        INHERITED::onAbandon();
    }

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
        return false;
    }

private:
    void invokeReleaseProc() {
        if (fReleaseHelper) {
            // Depending on the ref count of fReleaseHelper this may or may not actually trigger the
            // ReleaseProc to be called.
            fReleaseHelper.reset();
        }
    }
    GrMockTextureInfo          fInfo;
    sk_sp<GrReleaseProcHelper> fReleaseHelper;

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
        this->registerWithCacheWrapped();
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

protected:
    // constructor for subclasses
    GrMockRenderTarget(GrMockGpu* gpu, const GrSurfaceDesc& desc,
                       const GrMockRenderTargetInfo& info)
            : GrSurface(gpu, desc), INHERITED(gpu, desc), fInfo(info) {}

private:
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
                              const GrMockRenderTargetInfo& rtInfo)
            : GrSurface(gpu, desc)
            , GrMockTexture(gpu, desc, mipMapsStatus, texInfo)
            , GrMockRenderTarget(gpu, desc, rtInfo) {
        this->registerWithCacheWrapped();
    }

    GrTexture* asTexture() override { return this; }
    GrRenderTarget* asRenderTarget() override { return this; }
    const GrTexture* asTexture() const override { return this; }
    const GrRenderTarget* asRenderTarget() const override { return this; }

private:
    void onAbandon() override {
        GrRenderTarget::onAbandon();
        GrMockTexture::onAbandon();
    }

    void onRelease() override {
        GrRenderTarget::onRelease();
        GrMockTexture::onRelease();
    }

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
