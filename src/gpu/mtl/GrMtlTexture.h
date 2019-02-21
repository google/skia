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
                                                  GrWrapCacheable, GrIOType);

    ~GrMtlTexture() override;

    id<MTLTexture> mtlTexture() const { return fTexture; }

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override;

    void textureParamsModified() override {}

    bool reallocForMipmap(GrMtlGpu* gpu, uint32_t mipLevels);

    void setIdleProc(IdleProc proc, void* context) override {
        fIdleProc = proc;
        fIdleProcContext = context;
    }
    void* idleContext() const override { return fIdleProcContext; }

protected:
    GrMtlTexture(GrMtlGpu*, const GrSurfaceDesc&, id<MTLTexture>, GrMipMapsStatus);

    GrMtlGpu* getMtlGpu() const;

    void onAbandon() override {
        fTexture = nil;
        INHERITED::onAbandon();
    }
    void onRelease() override {
        fTexture = nil;
        INHERITED::onRelease();
    }

     bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
         return false;
     }

private:
    enum Wrapped { kWrapped };

    // Since all MTLResources are inherently ref counted, we can call the Release proc when we
    // delete the GrMtlTexture without worry of the MTLTexture getting deleted before it is done on
    // the GPU. Thus we do nothing special here with the releaseHelper.
    void onSetRelease(sk_sp<GrReleaseProcHelper> releaseHelper) override {}

    void willRemoveLastRefOrPendingIO() override {
        if (fIdleProc) {
            fIdleProc(fIdleProcContext);
            fIdleProc = nullptr;
            fIdleProcContext = nullptr;
        }
    }

    GrMtlTexture(GrMtlGpu*, SkBudgeted, const GrSurfaceDesc&, id<MTLTexture>,
                 GrMipMapsStatus);

    GrMtlTexture(GrMtlGpu*, Wrapped, const GrSurfaceDesc&, id<MTLTexture>, GrMipMapsStatus,
                 GrWrapCacheable, GrIOType);

    id<MTLTexture> fTexture;
    sk_sp<GrReleaseProcHelper> fReleaseHelper;
    IdleProc* fIdleProc = nullptr;
    void* fIdleProcContext = nullptr;

    typedef GrTexture INHERITED;
};

#endif
