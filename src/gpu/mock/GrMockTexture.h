/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrMockTexture_DEFINED
#define GrMockTexture_DEFINED

#include "GrMockGpu.h"
#include "GrTexture.h"
#include "GrTexturePriv.h"

class GrMockTexture : public GrTexture {
public:
    GrMockTexture(GrMockGpu* gpu, SkBudgeted budgeted, const GrSurfaceDesc& desc, bool hasMipLevels)
            : GrMockTexture(gpu, desc, hasMipLevels) {
        this->registerWithCache(budgeted);
    }
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
    GrMockTexture(GrMockGpu* gpu, const GrSurfaceDesc& desc, bool hasMipLevels)
            : GrSurface(gpu, desc)
            , INHERITED(gpu, desc, kITexture2DSampler_GrSLType, GrSamplerParams::kMipMap_FilterMode,
                        hasMipLevels)
            , fReleaseProc(nullptr)
            , fReleaseCtx(nullptr) {}

private:
    ReleaseProc fReleaseProc;
    ReleaseCtx fReleaseCtx;

    typedef GrTexture INHERITED;
};

class GrMockTextureRenderTarget : public GrMockTexture, public GrRenderTarget {
public:
    GrMockTextureRenderTarget(GrMockGpu* gpu, SkBudgeted budgeted, const GrSurfaceDesc& desc,
                              bool hasMipLevels)
            : GrSurface(gpu, desc)
            , GrMockTexture(gpu, desc, hasMipLevels)
            , GrRenderTarget(gpu, desc) {
        this->registerWithCache(budgeted);
    }
    ResolveType getResolveType() const override { return kCanResolve_ResolveType; }
    GrBackendObject getRenderTargetHandle() const override { return 0; }
    bool canAttemptStencilAttachment() const override { return true; }
    bool completeStencilAttachment() override { return true; }
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
        return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                      this->numStencilSamples(),
                                      this->texturePriv().hasMipMaps());
    }

    void computeScratchKey(GrScratchKey* key) const override {
        GrTexturePriv::ComputeScratchKey(this->config(), this->width(), this->height(),
                                         this->origin(), true, this->numStencilSamples(),
                                         this->texturePriv().hasMipMaps(), key);
    }
};

#endif
