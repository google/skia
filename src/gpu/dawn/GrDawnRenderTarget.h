/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnRenderTarget_DEFINED
#define GrDawnRenderTarget_DEFINED

#include "include/gpu/dawn/GrDawnTypes.h"
#include "src/gpu/GrRenderTarget.h"

class GrDawnGpu;

class GrDawnRenderTarget: public GrRenderTarget {
public:
    static sk_sp<GrDawnRenderTarget> MakeWrapped(GrDawnGpu*, const SkISize& dimensions,
                                                 GrPixelConfig config, int sampleCnt,
                                                 const GrDawnImageInfo&);

    ~GrDawnRenderTarget() override;

    bool canAttemptStencilAttachment() const override {
        return true;
    }

    GrBackendRenderTarget getBackendRenderTarget() const override;
    GrBackendFormat backendFormat() const override;
    dawn::Texture texture() const { return fInfo.fTexture; }

protected:
    GrDawnRenderTarget(GrDawnGpu* gpu,
                       const SkISize& dimensions,
                       GrPixelConfig config,
                       int sampleCnt,
                       const GrDawnImageInfo& info);

    void onAbandon() override;
    void onRelease() override;
    void onSetRelease(sk_sp<GrRefCntedCallback> releaseHelper) override {}

    // This accounts for the texture's memory and any MSAA renderbuffer's memory.
    size_t onGpuMemorySize() const override;

    static GrDawnRenderTarget* Create(GrDawnGpu*, const GrSurfaceDesc&, int sampleCnt,
                                      const GrDawnImageInfo&);

    bool completeStencilAttachment() override;
    GrDawnImageInfo fInfo;
    typedef GrRenderTarget INHERITED;
};

#endif
