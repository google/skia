/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlLayerObjects_DEFINED
#define GrMtlLayerObjects_DEFINED

#include "src/gpu/mtl/GrMtlRenderTarget.h"
#include "src/gpu/mtl/GrMtlTextureRenderTarget.h"

#import <QuartzCore/CAMetalLayer.h>

class GrMtlLayerRenderTarget: public GrMtlRenderTarget {
public:
    static sk_sp<GrMtlLayerRenderTarget> MakeWrappedRenderTarget(GrMtlGpu*,
                                                            const GrSurfaceDesc&,
                                                            int sampleCnt,
                                                            CAMetalLayer* layer,
                                                            GrMTLHandle* drawable);

    ~GrMtlLayerRenderTarget() override {}

    id<MTLTexture> mtlColorTexture() const override;
    id<MTLTexture> mtlResolveTexture() const override;

    GrBackendRenderTarget getBackendRenderTarget() const override;

    GrBackendFormat backendFormat() const override;

protected:
    GrMtlLayerRenderTarget(GrMtlGpu* gpu,
                      const GrSurfaceDesc& desc,
                      int sampleCnt,
                      id<MTLTexture> colorTexture,
                      CAMetalLayer* layer,
                      GrMTLHandle* drawable);
    GrMtlLayerRenderTarget(GrMtlGpu* gpu,
                      const GrSurfaceDesc& desc,
                      CAMetalLayer* layer,
                      GrMTLHandle* drawable);

private:
    // Extra param to disambiguate from constructor used by subclasses.
    enum Wrapped { kWrapped };
    GrMtlLayerRenderTarget(GrMtlGpu* gpu,
                      const GrSurfaceDesc& desc,
                      int sampleCnt,
                      id<MTLTexture> colorTexture,
                           CAMetalLayer* layer,
                           GrMTLHandle* drawable,
                      Wrapped);
    GrMtlLayerRenderTarget(GrMtlGpu* gpu,
                      const GrSurfaceDesc& desc,
                           CAMetalLayer* layer,
                           GrMTLHandle* drawable,
                      Wrapped);

    CAMetalLayer* fLayer;
    mutable GrMTLHandle* fDrawable;
    int fSampleCount;
};

class GrMtlLayerTexture : public GrMtlTexture {
public:
    ~GrMtlLayerTexture() override {}

    id<MTLTexture> mtlTexture() const override;

protected:
    GrMtlLayerTexture(GrMtlGpu* gpu, const GrSurfaceDesc&, CAMetalLayer* layer, GrMTLHandle* drawable,
                      GrMipMapsStatus);

private:
    CAMetalLayer* fLayer;
    mutable GrMTLHandle* fDrawable;
};

class GrMtlLayerTextureRenderTarget: public GrMtlLayerTexture, public GrMtlLayerRenderTarget  {
public:
    static sk_sp<GrMtlLayerTextureRenderTarget> MakeWrappedTextureRenderTarget(GrMtlGpu*,
                                                                          const GrSurfaceDesc&,
                                                                          int sampleCnt,
                                                                          CAMetalLayer* layer,
                                                                          GrMTLHandle* drawable,
                                                                          GrWrapCacheable);
    GrBackendFormat backendFormat() const override {
        return GrMtlLayerTexture::backendFormat();
    }

protected:
    void onAbandon() override {
        GrMtlLayerRenderTarget::onAbandon();
        GrMtlLayerTexture::onAbandon();
    }

    void onRelease() override {
        GrMtlLayerRenderTarget::onRelease();
        GrMtlLayerTexture::onRelease();
    }

private:
    GrMtlLayerTextureRenderTarget(GrMtlGpu* gpu,
                             const GrSurfaceDesc& desc,
                             int sampleCnt,
                             id<MTLTexture> colorTexture,
                             CAMetalLayer* layer,
                             GrMTLHandle* drawable,
                             GrMipMapsStatus,
                             GrWrapCacheable cacheable);

    GrMtlLayerTextureRenderTarget(GrMtlGpu* gpu,
                             const GrSurfaceDesc& desc,
                             CAMetalLayer* layer,
                             GrMTLHandle* drawable,
                             GrMipMapsStatus,
                             GrWrapCacheable cacheable);

    size_t onGpuMemorySize() const override {
        // TODO: When used as render targets certain formats may actually have a larger size than
        // the base format size. Check to make sure we are reporting the correct value here.
        // The plus 1 is to account for the resolve texture or if not using msaa the RT itself
        int numColorSamples = this->numSamples();
        if (numColorSamples > 1) {
            ++numColorSamples;
        }
        const GrCaps& caps = *this->getGpu()->caps();
        return GrSurface::ComputeSize(caps, this->backendFormat(), this->width(), this->height(),
                                      numColorSamples, GrMipMapped::kNo);
    }
};


#endif

