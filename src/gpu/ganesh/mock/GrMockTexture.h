/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrMockTexture_DEFINED
#define GrMockTexture_DEFINED

#include "include/gpu/mock/GrMockTypes.h"
#include "src/gpu/ganesh/GrAttachment.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/mock/GrMockGpu.h"

class GrMockTexture : public GrTexture {
public:
    GrMockTexture(GrMockGpu* gpu,
                  skgpu::Budgeted budgeted,
                  SkISize dimensions,
                  GrProtected isProtected,
                  GrMipmapStatus mipmapStatus,
                  const GrMockTextureInfo& info,
                  std::string_view label)
            : GrMockTexture(gpu, dimensions, isProtected, mipmapStatus, info, label) {
        this->registerWithCache(budgeted);
    }

    GrMockTexture(GrMockGpu* gpu,
                  SkISize dimensions,
                  GrProtected isProtected,
                  GrMipmapStatus mipmapStatus,
                  const GrMockTextureInfo& info,
                  GrWrapCacheable cacheable,
                  GrIOType ioType,
                  std::string_view label)
            : GrMockTexture(gpu, dimensions, isProtected, mipmapStatus, info, label) {
        if (ioType == kRead_GrIOType) {
            this->setReadOnly();
        }
        this->registerWithCacheWrapped(cacheable);
    }

    ~GrMockTexture() override {}

    GrBackendTexture getBackendTexture() const override {
        return GrBackendTexture(this->width(), this->height(), this->mipmapped(), fInfo);
    }

    GrBackendFormat backendFormat() const override {
        return fInfo.getBackendFormat();
    }

    void textureParamsModified() override {}

protected:
    // constructor for subclasses
    GrMockTexture(GrMockGpu* gpu, const SkISize& dimensions, GrProtected isProtected,
                  GrMipmapStatus mipmapStatus,
                  const GrMockTextureInfo& info,
                  std::string_view label)
            : GrSurface(gpu, dimensions, isProtected, label)
            , INHERITED(gpu, dimensions, isProtected, GrTextureType::k2D, mipmapStatus, label)
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
    void onSetLabel() override{}

    GrMockTextureInfo fInfo;

    using INHERITED = GrTexture;
};

class GrMockRenderTarget : public GrRenderTarget {
public:
    GrMockRenderTarget(GrMockGpu* gpu,
                       skgpu::Budgeted budgeted,
                       SkISize dimensions,
                       int sampleCnt,
                       GrProtected isProtected,
                       const GrMockRenderTargetInfo& info,
                       std::string_view label)
            : GrSurface(gpu, dimensions, isProtected, label)
            , INHERITED(gpu, dimensions, sampleCnt, isProtected, label)
            , fInfo(info) {
        this->registerWithCache(budgeted);
    }

    enum Wrapped { kWrapped };
    GrMockRenderTarget(GrMockGpu* gpu, Wrapped, SkISize dimensions, int sampleCnt,
                       GrProtected isProtected,
                       const GrMockRenderTargetInfo& info,
                       std::string_view label)
            : GrSurface(gpu, dimensions, isProtected, label)
            , INHERITED(gpu, dimensions, sampleCnt, isProtected, label)
            , fInfo(info) {
        this->registerWithCacheWrapped(GrWrapCacheable::kNo);
    }

    bool canAttemptStencilAttachment(bool useMSAASurface) const override {
        SkASSERT(useMSAASurface == (this->numSamples() > 1));
        return true;
    }

    bool completeStencilAttachment(GrAttachment*, bool useMSAASurface) override {
        SkASSERT(useMSAASurface == (this->numSamples() > 1));
        return true;
    }

    size_t onGpuMemorySize() const override {
        int numColorSamples = this->numSamples();
        if (numColorSamples > 1) {
            // Add one to account for the resolve buffer.
            ++numColorSamples;
        }
        return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                      numColorSamples, GrMipmapped::kNo);
    }

    GrBackendRenderTarget getBackendRenderTarget() const override {
        int numStencilBits = 0;
        if (GrAttachment* stencil = this->getStencilAttachment()) {
            numStencilBits = GrBackendFormatStencilBits(stencil->backendFormat());
        }
        return {this->width(), this->height(), this->numSamples(), numStencilBits, fInfo};
    }

    GrBackendFormat backendFormat() const override {
        return fInfo.getBackendFormat();
    }

protected:
    // constructor for subclasses
    GrMockRenderTarget(GrMockGpu* gpu,
                       SkISize dimensions,
                       int sampleCnt,
                       GrProtected isProtected,
                       const GrMockRenderTargetInfo& info,
                       std::string_view label)
            : GrSurface(gpu, dimensions, isProtected, label)
            , INHERITED(gpu, dimensions, sampleCnt, isProtected, label)
            , fInfo(info) {}

private:
    void onSetLabel() override{}

    GrMockRenderTargetInfo fInfo;

    using INHERITED = GrRenderTarget;
};

class GrMockTextureRenderTarget : public GrMockTexture, public GrMockRenderTarget {
public:
    // Internally created.
    GrMockTextureRenderTarget(GrMockGpu* gpu,
                              skgpu::Budgeted budgeted,
                              SkISize dimensions,
                              int sampleCnt,
                              GrProtected isProtected,
                              GrMipmapStatus mipmapStatus,
                              const GrMockTextureInfo& texInfo,
                              const GrMockRenderTargetInfo& rtInfo,
                              std::string_view label)
            : GrSurface(gpu, dimensions, isProtected, label)
            , GrMockTexture(gpu, dimensions, isProtected, mipmapStatus, texInfo, label)
            , GrMockRenderTarget(gpu, dimensions, sampleCnt, isProtected, rtInfo, label) {
        this->registerWithCache(budgeted);
    }

    // Renderable wrapped backend texture.
    GrMockTextureRenderTarget(GrMockGpu* gpu,
                              SkISize dimensions,
                              int sampleCnt,
                              GrProtected isProtected,
                              GrMipmapStatus mipmapStatus,
                              const GrMockTextureInfo& texInfo,
                              const GrMockRenderTargetInfo& rtInfo,
                              GrWrapCacheable cacheable,
                              std::string_view label)
            : GrSurface(gpu, dimensions, isProtected, label)
            , GrMockTexture(gpu, dimensions, isProtected, mipmapStatus, texInfo, label)
            , GrMockRenderTarget(gpu, dimensions, sampleCnt, isProtected, rtInfo, label) {
        this->registerWithCacheWrapped(cacheable);
    }

    GrTexture* asTexture() override { return this; }
    GrRenderTarget* asRenderTarget() override { return this; }
    const GrTexture* asTexture() const override { return this; }
    const GrRenderTarget* asRenderTarget() const override { return this; }

    GrBackendFormat backendFormat() const override {
        return GrMockTexture::backendFormat();
    }

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
        return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                      numColorSamples, this->mipmapped());
    }

    void onSetLabel() override{}

    // This avoids an inherits via dominance warning on MSVC.
    void computeScratchKey(skgpu::ScratchKey* key) const override {
        GrTexture::computeScratchKey(key);
    }
};

#endif
