/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrMockTexture_DEFINED
#define GrMockTexture_DEFINED

#include "include/gpu/GrTexture.h"
#include "include/gpu/mock/GrMockTypes.h"
#include "src/gpu/GrRenderTarget.h"
#include "src/gpu/GrRenderTargetPriv.h"
#include "src/gpu/GrStencilAttachment.h"
#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/mock/GrMockGpu.h"

class GrMockTexture : public GrTexture {
public:
    GrMockTexture(GrMockGpu* gpu, SkBudgeted budgeted, const GrSurfaceDesc& desc,
                  GrProtected isProtected, GrMipMapsStatus mipMapsStatus,
                  const GrMockTextureInfo& info)
            : GrMockTexture(gpu, desc, isProtected, mipMapsStatus, info) {
        this->registerWithCache(budgeted);
    }

    GrMockTexture(GrMockGpu* gpu, const GrSurfaceDesc& desc, GrProtected isProtected,
                  GrMipMapsStatus mipMapsStatus, const GrMockTextureInfo& info,
                  GrWrapCacheable cacheable, GrIOType ioType)
            : GrMockTexture(gpu, desc, isProtected, mipMapsStatus, info) {
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
        return fInfo.getBackendFormat();
    }

    void textureParamsModified() override {}

protected:
    // constructor for subclasses
    GrMockTexture(GrMockGpu* gpu, const GrSurfaceDesc& desc, GrProtected isProtected,
                  GrMipMapsStatus mipMapsStatus, const GrMockTextureInfo& info)
            : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, isProtected)
            , INHERITED(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, isProtected,
                        GrTextureType::k2D, mipMapsStatus)
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

private:
    GrMockTextureInfo fInfo;

    typedef GrTexture INHERITED;
};

class GrMockRenderTarget : public GrRenderTarget {
public:
    GrMockRenderTarget(GrMockGpu* gpu, SkBudgeted budgeted, const GrSurfaceDesc& desc,
                       int sampleCnt, GrProtected isProtected, const GrMockRenderTargetInfo& info)
            : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, isProtected)
            , INHERITED(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, sampleCnt, isProtected)
            , fInfo(info) {
        this->registerWithCache(budgeted);
    }

    enum Wrapped { kWrapped };
    GrMockRenderTarget(GrMockGpu* gpu, Wrapped, const GrSurfaceDesc& desc, int sampleCnt,
                       GrProtected isProtected, const GrMockRenderTargetInfo& info)
            : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, isProtected)
            , INHERITED(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, sampleCnt, isProtected)
            , fInfo(info) {
        this->registerWithCacheWrapped(GrWrapCacheable::kNo);
    }

    bool canAttemptStencilAttachment() const override { return true; }
    bool completeStencilAttachment() override { return true; }

    size_t onGpuMemorySize() const override {
        int numColorSamples = this->numSamples();
        if (numColorSamples > 1) {
            // Add one to account for the resolve buffer.
            ++numColorSamples;
        }
        const GrCaps& caps = *this->getGpu()->caps();
        return GrSurface::ComputeSize(caps, this->backendFormat(), this->dimensions(),
                                      numColorSamples, GrMipMapped::kNo);
    }

    GrBackendRenderTarget getBackendRenderTarget() const override {
        int numStencilBits = 0;
        if (GrStencilAttachment* stencil = this->renderTargetPriv().getStencilAttachment()) {
            numStencilBits = stencil->bits();
        }
        return {this->width(), this->height(), this->numSamples(), numStencilBits, fInfo};
    }

    GrBackendFormat backendFormat() const override {
        return fInfo.getBackendFormat();
    }

protected:
    // constructor for subclasses
    GrMockRenderTarget(GrMockGpu* gpu, const GrSurfaceDesc& desc, int sampleCnt,
                       GrProtected isProtected, const GrMockRenderTargetInfo& info)
            : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, isProtected)
            , INHERITED(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, sampleCnt, isProtected)
            , fInfo(info) {}

private:
    GrMockRenderTargetInfo fInfo;

    typedef GrRenderTarget INHERITED;
};

class GrMockTextureRenderTarget : public GrMockTexture, public GrMockRenderTarget {
public:
    // Internally created.
    GrMockTextureRenderTarget(GrMockGpu* gpu, SkBudgeted budgeted, const GrSurfaceDesc& desc,
                              int sampleCnt, GrProtected isProtected, GrMipMapsStatus mipMapsStatus,
                              const GrMockTextureInfo& texInfo,
                              const GrMockRenderTargetInfo& rtInfo)
            : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, isProtected)
            , GrMockTexture(gpu, desc, isProtected, mipMapsStatus, texInfo)
            , GrMockRenderTarget(gpu, desc, sampleCnt, isProtected, rtInfo) {
        this->registerWithCache(budgeted);
    }

    // Renderable wrapped backend texture.
    GrMockTextureRenderTarget(GrMockGpu* gpu, const GrSurfaceDesc& desc, int sampleCnt,
                              GrProtected isProtected, GrMipMapsStatus mipMapsStatus,
                              const GrMockTextureInfo& texInfo,
                              const GrMockRenderTargetInfo& rtInfo, GrWrapCacheable cacheble)
            : GrSurface(gpu, {desc.fWidth, desc.fHeight}, desc.fConfig, isProtected)
            , GrMockTexture(gpu, desc, isProtected, mipMapsStatus, texInfo)
            , GrMockRenderTarget(gpu, desc, sampleCnt, isProtected, rtInfo) {
        this->registerWithCacheWrapped(cacheble);
    }

    GrTexture* asTexture() override { return this; }
    GrRenderTarget* asRenderTarget() override { return this; }
    const GrTexture* asTexture() const override { return this; }
    const GrRenderTarget* asRenderTarget() const override { return this; }

    GrBackendFormat backendFormat() const override {
        return GrMockTexture::backendFormat();
    }

protected:
    // This avoids an inherits via dominance warning on MSVC.
    void willRemoveLastRef() override { GrTexture::willRemoveLastRef(); }

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
        int numColorSamples = this->numSamples();
        if (numColorSamples > 1) {
            // Add one to account for the resolve buffer.
            ++numColorSamples;
        }
        const GrCaps& caps = *this->getGpu()->caps();
        return GrSurface::ComputeSize(caps, this->backendFormat(), this->dimensions(),
                                      numColorSamples, this->texturePriv().mipMapped());
    }

    // This avoids an inherits via dominance warning on MSVC.
    void computeScratchKey(GrScratchKey* key) const override { GrTexture::computeScratchKey(key); }
};

#endif
