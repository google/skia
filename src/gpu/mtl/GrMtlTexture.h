/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTexture_DEFINED
#define GrMtlTexture_DEFINED

#include "src/gpu/GrTexture.h"
#include "src/gpu/mtl/GrMtlAttachment.h"
#import <Metal/Metal.h>

class GrMtlGpu;

class GrMtlTexture : public GrTexture {
public:
    static sk_sp<GrMtlTexture> MakeNewTexture(GrMtlGpu*,
                                              SkBudgeted budgeted,
                                              SkISize dimensions,
                                              MTLPixelFormat format,
                                              uint32_t mipLevels,
                                              GrMipmapStatus);

    static sk_sp<GrMtlTexture> MakeWrappedTexture(GrMtlGpu*,
                                                  SkISize,
                                                  id<MTLTexture>,
                                                  GrWrapCacheable,
                                                  GrIOType);

    ~GrMtlTexture() override;

    GrMtlAttachment* attachment() const { return fTexture.get(); }
    id<MTLTexture> mtlTexture() const { return fTexture->mtlTexture(); }

    GrBackendTexture getBackendTexture() const override;

    GrBackendFormat backendFormat() const override;

    void textureParamsModified() override {}

    bool reallocForMipmap(GrMtlGpu* gpu, uint32_t mipLevels);

protected:
    GrMtlTexture(GrMtlGpu*, SkISize, sk_sp<GrMtlAttachment>, GrMipmapStatus);

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

    GrMtlTexture(GrMtlGpu*, SkBudgeted, SkISize, sk_sp<GrMtlAttachment>, GrMipmapStatus);

    GrMtlTexture(GrMtlGpu*,
                 Wrapped,
                 SkISize,
                 sk_sp<GrMtlAttachment>,
                 GrMipmapStatus,
                 GrWrapCacheable,
                 GrIOType);

    sk_sp<GrMtlAttachment> fTexture;

    using INHERITED = GrTexture;
};

#endif
