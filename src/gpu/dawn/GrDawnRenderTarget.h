/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnRenderTarget_DEFINED
#define GrDawnRenderTarget_DEFINED

#include "include/gpu/GrRenderTarget.h"
#include "include/gpu/dawn/GrDawnTypes.h"

class GrDawnGpu;

class GrDawnRenderTarget: public GrRenderTarget {
public:
    static sk_sp<GrDawnRenderTarget> MakeWrapped(GrDawnGpu*, const GrSurfaceDesc&, int sampleCnt,
                                                 const GrDawnImageInfo&);

    ~GrDawnRenderTarget() override;

    // override of GrRenderTarget
    ResolveType getResolveType() const override {
        if (this->numSamples() > 1) {
            return kCanResolve_ResolveType;
        }
        return kAutoResolves_ResolveType;
    }

    bool canAttemptStencilAttachment() const override {
        return true;
    }

    GrBackendRenderTarget getBackendRenderTarget() const override;
    GrBackendFormat backendFormat() const override;
    dawn::Texture texture() const { return fInfo.fTexture; }

protected:
    GrDawnRenderTarget(GrDawnGpu* gpu,
                       const GrSurfaceDesc& desc,
                       int sampleCnt,
                       const GrDawnImageInfo& info,
                       GrBackendObjectOwnership);

    GrDawnGpu* getDawnGpu() const;

    void onAbandon() override;
    void onRelease() override;
    void onSetRelease(sk_sp<GrRefCntedCallback> releaseHelper) override {}

    // This accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override {
        // The plus 1 is to account for the resolve texture or if not using msaa the RT itself
        int numSamples = this->numSamples() + 1;
        return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                      numSamples, GrMipMapped::kNo);
    }

    static GrDawnRenderTarget* Create(GrDawnGpu*, const GrSurfaceDesc&, int sampleCnt,
                                      const GrDawnImageInfo&, GrBackendObjectOwnership);

    bool completeStencilAttachment() override;
    GrDawnImageInfo fInfo;
    typedef GrRenderTarget INHERITED;
};

#endif
