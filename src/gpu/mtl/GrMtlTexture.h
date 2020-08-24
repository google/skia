/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTexture_DEFINED
#define GrMtlTexture_DEFINED

#include "src/gpu/GrTexture.h"

#import <Metal/Metal.h>
#include "include/ports/SkCFObject.h"

class GrMtlGpu;

class GrMtlTexture : public GrTexture {
public:
    static sk_sp<GrMtlTexture> MakeNewTexture(GrMtlGpu*,
                                              SkBudgeted budgeted,
                                              SkISize,
                                              MTLTextureDescriptor*,
                                              GrMipmapStatus);

    static sk_sp<GrMtlTexture> MakeWrappedTexture(GrMtlGpu*,
                                                  SkISize,
                                                  sk_cf_obj<id<MTLTexture>>,
                                                  GrWrapCacheable,
                                                  GrIOType);

    ~GrMtlTexture() override;

    id<MTLTexture> mtlTexture() const { return fTexture.get(); }

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override;

    void textureParamsModified() override {}

    bool reallocForMipmap(GrMtlGpu* gpu, uint32_t mipLevels);

protected:
    GrMtlTexture(GrMtlGpu*, SkISize, sk_cf_obj<id<MTLTexture>>, GrMipmapStatus);

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

    GrMtlTexture(GrMtlGpu*, SkBudgeted, SkISize, sk_cf_obj<id<MTLTexture>>, GrMipmapStatus);

    GrMtlTexture(GrMtlGpu*,
                 Wrapped,
                 SkISize,
                 sk_cf_obj<id<MTLTexture>>,
                 GrMipmapStatus,
                 GrWrapCacheable,
                 GrIOType);

    sk_cf_obj<id<MTLTexture>> fTexture;

    typedef GrTexture INHERITED;
};

#endif
