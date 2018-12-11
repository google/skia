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
                                                const GrSurfaceDesc&,
                                                MTLTextureDescriptor*,
                                                GrMipMapsStatus);

    static sk_sp<GrMtlTexture> MakeWrappedTexture(GrMtlGpu*, const GrSurfaceDesc&, id<MTLTexture>,
                                                  GrIOType, bool purgeImmediately);

    ~GrMtlTexture() override;

    id<MTLTexture> mtlTexture() const { return fTexture; }

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override;

    void textureParamsModified() override {}

    bool reallocForMipmap(GrMtlGpu* gpu, uint32_t mipLevels);

    void setRelease(sk_sp<GrReleaseProcHelper> releaseHelper) override {
        // Since all MTLResources are inherently ref counted, we can call the Release proc when we
        // delete the GrMtlTexture without worry of the MTLTexture getting deleted before it is done
        // on the GPU.
        fReleaseHelper = std::move(releaseHelper);
    }

    void setPurgeableProc(PurgeableProc proc, void* context)  override {
        fPurgeableProc = proc;
        fPurgeableProcCtx = context;
    }

protected:
    GrMtlTexture(GrMtlGpu*, const GrSurfaceDesc&, id<MTLTexture>, GrMipMapsStatus);

    GrMtlGpu* getMtlGpu() const;

    void onAbandon() override {
        this->invokeReleaseProc();
        fTexture = nil;
    }
    void onRelease() override {
        this->invokeReleaseProc();
        fTexture = nil;
    }

     bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
         return false;
     }

private:
    enum Wrapped { kWrapped };

    void invokeReleaseProc() {
        // Depending on the ref count of fReleaseHelper this may or may not actually trigger the
        // ReleaseProc to be called.
        fReleaseHelper.reset();
    }

    void becamePurgeable() const override {
        if (fPurgeableProc) {
            fPurgeableProc(fPurgeableProcCtx);
        }
    }

    GrMtlTexture(GrMtlGpu*, SkBudgeted, const GrSurfaceDesc&, id<MTLTexture>,
                 GrMipMapsStatus);

    GrMtlTexture(GrMtlGpu*, Wrapped, const GrSurfaceDesc&, id<MTLTexture>, GrMipMapsStatus,
                 GrIOType, bool purgeImmediately);

    id<MTLTexture> fTexture;

    sk_sp<GrReleaseProcHelper> fReleaseHelper;

    typedef GrTexture INHERITED;
};

#endif
