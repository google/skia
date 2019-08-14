/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlRenderTarget_DEFINED
#define GrMtlRenderTarget_DEFINED

#include "src/gpu/GrRenderTarget.h"

#include "include/gpu/GrBackendSurface.h"

#import <Metal/Metal.h>

class GrMtlGpu;

class GrMtlRenderTarget: public GrRenderTarget {
public:
    static sk_sp<GrMtlRenderTarget> MakeWrappedRenderTarget(GrMtlGpu*,
                                                            const GrSurfaceDesc&,
                                                            int sampleCnt,
                                                            id<MTLTexture>);

    ~GrMtlRenderTarget() override;

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

    id<MTLTexture> mtlColorTexture() const { return fColorTexture; }
    id<MTLTexture> mtlResolveTexture() const { return fResolveTexture; }

    GrBackendRenderTarget getBackendRenderTarget() const override;

    GrBackendFormat backendFormat() const override;

protected:
    GrMtlRenderTarget(GrMtlGpu* gpu,
                      const GrSurfaceDesc& desc,
                      int sampleCnt,
                      id<MTLTexture> colorTexture,
                      id<MTLTexture> resolveTexture);

    GrMtlRenderTarget(GrMtlGpu* gpu,
                      const GrSurfaceDesc& desc,
                      id<MTLTexture> colorTexture);

    GrMtlGpu* getMtlGpu() const;

    void onAbandon() override;
    void onRelease() override;

    // This accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override {
        int numColorSamples = this->numSamples();
        // TODO: When used as render targets certain formats may actually have a larger size than
        // the base format size. Check to make sure we are reporting the correct value here.
        // The plus 1 is to account for the resolve texture or if not using msaa the RT itself
        if (numColorSamples > 1) {
            ++numColorSamples;
        }
        return GrSurface::ComputeSize(this->config(), this->width(), this->height(),
                                      numColorSamples, GrMipMapped::kNo);
    }

    id<MTLTexture> fColorTexture;
    id<MTLTexture> fResolveTexture;

private:
    // Extra param to disambiguate from constructor used by subclasses.
    enum Wrapped { kWrapped };
    GrMtlRenderTarget(GrMtlGpu* gpu,
                      const GrSurfaceDesc& desc,
                      int sampleCnt,
                      id<MTLTexture> colorTexture,
                      id<MTLTexture> resolveTexture,
                      Wrapped);
    GrMtlRenderTarget(GrMtlGpu* gpu,
                      const GrSurfaceDesc& desc,
                      id<MTLTexture> colorTexture,
                      Wrapped);

    bool completeStencilAttachment() override;

    typedef GrRenderTarget INHERITED;
};


#endif

