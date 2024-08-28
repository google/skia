/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef GrMockTexture_DEFINED
#define GrMockTexture_DEFINED

#include "include/core/SkSize.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "include/gpu/ganesh/mock/GrMockTypes.h"
#include "include/private/base/SkAssert.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/ganesh/GrAttachment.h"
#include "src/gpu/ganesh/GrBackendUtils.h"
#include "src/gpu/ganesh/GrRenderTarget.h"
#include "src/gpu/ganesh/GrSurface.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/mock/GrMockGpu.h"

#include <cstddef>
#include <string_view>

namespace skgpu { class ScratchKey; }

class GrMockTexture : public GrTexture {
public:
    GrMockTexture(GrMockGpu* gpu,
                  skgpu::Budgeted budgeted,
                  SkISize dimensions,
                  GrMipmapStatus mipmapStatus,
                  const GrMockTextureInfo& info,
                  std::string_view label)
            : GrMockTexture(gpu, dimensions, mipmapStatus, info, label) {
        this->registerWithCache(budgeted);
    }

    GrMockTexture(GrMockGpu* gpu,
                  SkISize dimensions,
                  GrMipmapStatus mipmapStatus,
                  const GrMockTextureInfo& info,
                  GrWrapCacheable cacheable,
                  GrIOType ioType,
                  std::string_view label)
            : GrMockTexture(gpu, dimensions, mipmapStatus, info, label) {
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
    GrMockTexture(GrMockGpu* gpu,
                  const SkISize& dims,
                  GrMipmapStatus mipmapStatus,
                  const GrMockTextureInfo& info,
                  std::string_view label)
            : GrSurface(gpu, dims, info.getProtected(), label)
            , GrTexture(gpu, dims, info.getProtected(), GrTextureType::k2D, mipmapStatus, label)
            , fInfo(info) {}

    void onRelease() override {
        INHERITED::onRelease();
    }

    void onAbandon() override {
        INHERITED::onAbandon();
    }

    bool onStealBackendTexture(GrBackendTexture*, SkImages::BackendTextureReleaseProc*) override {
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
                       const GrMockRenderTargetInfo& info,
                       std::string_view label)
            : GrSurface(gpu, dimensions, info.getProtected(), label)
            , GrRenderTarget(gpu, dimensions, sampleCnt, info.getProtected(), label)
            , fInfo(info) {
        this->registerWithCache(budgeted);
    }

    enum Wrapped { kWrapped };
    GrMockRenderTarget(GrMockGpu* gpu, Wrapped, SkISize dimensions, int sampleCnt,
                       const GrMockRenderTargetInfo& info,
                       std::string_view label)
            : GrSurface(gpu, dimensions, info.getProtected(), label)
            , GrRenderTarget(gpu, dimensions, sampleCnt, info.getProtected(), label)
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
        return GrSurface::ComputeSize(
                this->backendFormat(), this->dimensions(), numColorSamples, skgpu::Mipmapped::kNo);
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
                       const GrMockRenderTargetInfo& info,
                       std::string_view label)
            : GrSurface(gpu, dimensions, info.getProtected(), label)
            , GrRenderTarget(gpu, dimensions, sampleCnt, info.getProtected(), label)
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
                              GrMipmapStatus mipmapStatus,
                              const GrMockTextureInfo& texInfo,
                              const GrMockRenderTargetInfo& rtInfo,
                              std::string_view label)
            : GrSurface(gpu, dimensions, texInfo.getProtected(), label)
            , GrMockTexture(gpu, dimensions, mipmapStatus, texInfo, label)
            , GrMockRenderTarget(gpu, dimensions, sampleCnt, rtInfo, label) {
        SkASSERT(texInfo.getProtected() == rtInfo.getProtected());
        this->registerWithCache(budgeted);
    }

    // Renderable wrapped backend texture.
    GrMockTextureRenderTarget(GrMockGpu* gpu,
                              SkISize dimensions,
                              int sampleCnt,
                              GrMipmapStatus mipmapStatus,
                              const GrMockTextureInfo& texInfo,
                              const GrMockRenderTargetInfo& rtInfo,
                              GrWrapCacheable cacheable,
                              std::string_view label)
            : GrSurface(gpu, dimensions, texInfo.getProtected(), label)
            , GrMockTexture(gpu, dimensions, mipmapStatus, texInfo, label)
            , GrMockRenderTarget(gpu, dimensions, sampleCnt, rtInfo, label) {
        SkASSERT(texInfo.getProtected() == rtInfo.getProtected());
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
