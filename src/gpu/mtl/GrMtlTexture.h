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

class GrMtlGpu;

class GrMtlTexture : public GrTexture {
public:
    static sk_sp<GrMtlTexture> MakeNewTexture(GrMtlGpu*,
                                              SkBudgeted budgeted,
                                              SkISize,
                                              MTLTextureDescriptor*,
                                              GrMipMapsStatus);

    static sk_sp<GrMtlTexture> MakeWrappedTexture(GrMtlGpu*,
                                                  SkISize,
                                                  id<MTLTexture>,
                                                  GrWrapCacheable,
                                                  GrIOType);

    ~GrMtlTexture() override;

    id<MTLTexture> mtlTexture() const { return fTexture; }

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override;

    void textureParamsModified() override {}

    bool reallocForMipmap(GrMtlGpu* gpu, uint32_t mipLevels);

protected:
    GrMtlTexture(GrMtlGpu*, SkISize, id<MTLTexture>, GrMipMapsStatus);

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

    GrMtlTexture(GrMtlGpu*, SkBudgeted, SkISize, id<MTLTexture>, GrMipMapsStatus);

    GrMtlTexture(GrMtlGpu*,
                 Wrapped,
                 SkISize,
                 id<MTLTexture>,
                 GrMipMapsStatus,
                 GrWrapCacheable,
                 GrIOType);

    id<MTLTexture> fTexture;

    typedef GrTexture INHERITED;
};

#endif
