/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTexture_DEFINED
#define GrMtlTexture_DEFINED

#include "GrTexture.h"

#import <Metal/Metal.h>

class GrMtlGpu;

class GrMtlTexture : public GrTexture {
public:
    static sk_sp<GrMtlTexture> CreateNewTexture(GrMtlGpu*, SkBudgeted budgeted,
                                                const GrSurfaceDesc&, int mipLevels);

    static sk_sp<GrMtlTexture> MakeWrappedTexture(GrMtlGpu*, const GrSurfaceDesc&,
                                                  GrWrapOwnership);

    ~GrMtlTexture() override;

    id<MTLTexture> mtlTexture() const { return fTexture; }

    GrBackendObject getTextureHandle() const override;
    GrBackendTexture getBackendTexture() const override;

    void textureParamsModified() override {}

    bool reallocForMipmap(GrMtlGpu* gpu, uint32_t mipLevels);

    void setRelease(GrTexture::ReleaseProc proc, GrTexture::ReleaseCtx ctx) override {
        // Since all MTLResources are inherently ref counted, we can call the Release proc when we
        // delete the GrMtlTexture without worry of the MTLTexture getting deleted before it is done
        // on the GPU.
        fReleaseProc = proc;
        fReleaseCtx = ctx;
    }

protected:
    GrMtlTexture(GrMtlGpu*, const GrSurfaceDesc&);

    GrMtlGpu* getMtlGpu() const;

    void onAbandon() override {
        fTexture = nil;
    }
    void onRelease() override {
        fTexture = nil;
    }

     bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
         return false;
     }

private:
    enum Wrapped { kWrapped };
    GrMtlTexture(GrMtlGpu*, SkBudgeted, const GrSurfaceDesc&, id<MTLTexture>, GrMipMapsStatus);
   // GrMtlTexture(GrMtlGpu*, Wrapped, const GrSurfaceDesc&, GrMtlImage::Wrapped wrapped);

    id<MTLTexture> fTexture;

    ReleaseProc fReleaseProc = nullptr;
    ReleaseCtx fReleaseCtx = nullptr;

    typedef GrTexture INHERITED;
};

#endif
