/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTRenderTarget_DEFINED
#define GrNXTRenderTarget_DEFINED

#include "GrNXTTypes.h"
#include "GrRenderTarget.h"

class GrNXTGpu;

class GrNXTRenderTarget: public GrRenderTarget {
public:
    static sk_sp<GrNXTRenderTarget> MakeWrapped(GrNXTGpu*, const GrSurfaceDesc&,
                                                const GrNXTImageInfo*);

    ~GrNXTRenderTarget() override;

    // override of GrRenderTarget
    ResolveType getResolveType() const override {
        if (this->numColorSamples() > 1) {
            return kCanResolve_ResolveType;
        }
        return kAutoResolves_ResolveType;
    }

    bool canAttemptStencilAttachment() const override {
        return true;
    }

    GrBackendObject getRenderTargetHandle() const override;
    GrBackendRenderTarget getBackendRenderTarget() const override;

protected:
    GrNXTRenderTarget(GrNXTGpu* gpu,
                     const GrSurfaceDesc& desc,
                     const GrNXTImageInfo& info);

    GrNXTGpu* getNXTGpu() const;

    void onAbandon() override;
    void onRelease() override;

    // This accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override {
        // The plus 1 is to account for the resolve texture or if not using msaa the RT itself
        int numColorSamples = this->numColorSamples() + 1;
        return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                      numColorSamples, GrMipMapped::kNo);
    }

    bool completeStencilAttachment() override;
    GrNXTImageInfo fInfo;
    typedef GrRenderTarget INHERITED;
};

#endif
