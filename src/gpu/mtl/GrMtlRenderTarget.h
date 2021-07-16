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

    // Returns the GrMtlAttachment of the non-msaa attachment. If the color attachment has 1 sample,
    // then the color attachment will be returned. Otherwise, the resolve attachment is returned.
    GrMtlAttachment* nonMSAAAttachment() const {
        if (fColorAttachment->numSamples() == 1) {
            return fColorAttachment.get();
        } else {
            return fResolveAttachment.get();
        }
    }

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

    // This returns zero since the memory should all be handled by the attachments
    size_t onGpuMemorySize() const override { return 0; }

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

