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
#include "src/gpu/GrGpu.h"
#include "src/gpu/mtl/GrMtlAttachment.h"

#import <Metal/Metal.h>

class GrMtlGpu;

class GrMtlRenderTarget: public GrRenderTarget {
public:
    // If sampleCnt is greater than 1 and the texture is single sampled, then a MSAA texture
    // is created that will resolve to the wrapped single sample texture.
    static sk_sp<GrMtlRenderTarget> MakeWrappedRenderTarget(GrMtlGpu*,
                                                            SkISize,
                                                            int sampleCnt,
                                                            id<MTLTexture>);

    ~GrMtlRenderTarget() override;

    bool canAttemptStencilAttachment(bool useMSAASurface) const override {
        SkASSERT(useMSAASurface == (this->numSamples() > 1));
        return true;
    }

    GrMtlAttachment* colorAttachment() const { return fColorAttachment.get(); }
    id<MTLTexture> colorMTLTexture() const { return fColorAttachment->mtlTexture(); }
    GrMtlAttachment* resolveAttachment() const { return fResolveAttachment.get(); }
    id<MTLTexture> resolveMTLTexture() const { return fResolveAttachment->mtlTexture(); }

    GrBackendRenderTarget getBackendRenderTarget() const override;

    GrBackendFormat backendFormat() const override;

protected:
    GrMtlRenderTarget(GrMtlGpu* gpu,
                      SkISize,
                      sk_sp<GrMtlAttachment> colorAttachment,
                      sk_sp<GrMtlAttachment> resolveAttachment);

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

    sk_sp<GrMtlAttachment> fColorAttachment;
    sk_sp<GrMtlAttachment> fResolveAttachment;

private:
    // Extra param to disambiguate from constructor used by subclasses.
    enum Wrapped { kWrapped };
    GrMtlRenderTarget(GrMtlGpu* gpu,
                      SkISize,
                      sk_sp<GrMtlAttachment> colorAttachment,
                      sk_sp<GrMtlAttachment> resolveAttachment,
                      Wrapped);

    bool completeStencilAttachment(GrAttachment* stencil, bool useMSAASurface) override;

    using INHERITED = GrRenderTarget;
};


#endif

