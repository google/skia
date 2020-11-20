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
#include "include/ports/SkCFObject.h"
#include "src/gpu/GrGpu.h"

#import <Metal/Metal.h>

class GrMtlGpu;

class GrMtlRenderTarget: public GrRenderTarget {
public:
    // If sampleCnt is greater than 1 and the texture is single sampled, then a MSAA texture
    // is created that will resolve to the wrapped single sample texture.
    static sk_sp<GrMtlRenderTarget> MakeWrappedRenderTarget(GrMtlGpu*,
                                                            SkISize,
                                                            int sampleCnt,
                                                            sk_cfp<id<MTLTexture>>);

    ~GrMtlRenderTarget() override;

    bool canAttemptStencilAttachment() const override {
        return true;
    }

    id<MTLTexture> mtlColorTexture() const { return fColorTexture.get(); }
    id<MTLTexture> mtlResolveTexture() const { return fResolveTexture.get(); }

    GrBackendRenderTarget getBackendRenderTarget() const override;

    GrBackendFormat backendFormat() const override;

protected:
    GrMtlRenderTarget(GrMtlGpu* gpu,
                      SkISize,
                      int sampleCnt,
                      sk_cfp<id<MTLTexture>> colorTexture,
                      sk_cfp<id<MTLTexture>> resolveTexture);

    GrMtlRenderTarget(GrMtlGpu* gpu, SkISize, sk_cfp<id<MTLTexture>> colorTexture);

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
        return GrSurface::ComputeSize(this->backendFormat(), this->dimensions(),
                                      numColorSamples, GrMipmapped::kNo);
    }

    sk_cfp<id<MTLTexture>> fColorTexture;
    sk_cfp<id<MTLTexture>> fResolveTexture;

private:
    // Extra param to disambiguate from constructor used by subclasses.
    enum Wrapped { kWrapped };
    GrMtlRenderTarget(GrMtlGpu* gpu,
                      SkISize,
                      int sampleCnt,
                      sk_cfp<id<MTLTexture>> colorTexture,
                      sk_cfp<id<MTLTexture>> resolveTexture,
                      Wrapped);
    GrMtlRenderTarget(GrMtlGpu* gpu, SkISize, sk_cfp<id<MTLTexture>> colorTexture, Wrapped);

    bool completeStencilAttachment() override;

    using INHERITED = GrRenderTarget;
};


#endif

