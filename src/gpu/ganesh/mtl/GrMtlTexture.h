/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrMtlTexture_DEFINED
#define GrMtlTexture_DEFINED

#import <Metal/Metal.h>
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "src/gpu/ganesh/GrTexture.h"
#include "src/gpu/ganesh/mtl/GrMtlAttachment.h"

class GrMtlGpu;

class GrMtlTexture : public GrTexture {
public:
    static sk_sp<GrMtlTexture> MakeNewTexture(GrMtlGpu*,
                                              skgpu::Budgeted budgeted,
                                              SkISize dimensions,
                                              MTLPixelFormat format,
                                              uint32_t mipLevels,
                                              GrMipmapStatus,
                                              std::string_view label);

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
    GrMtlTexture(GrMtlGpu*,
                 SkISize,
                 sk_sp<GrMtlAttachment>,
                 GrMipmapStatus,
                 std::string_view label);

    GrMtlGpu* getMtlGpu() const;

    void onAbandon() override {
        fTexture = nil;
        INHERITED::onAbandon();
    }
    void onRelease() override {
        fTexture = nil;
        INHERITED::onRelease();
    }

    bool onStealBackendTexture(GrBackendTexture*, SkImages::BackendTextureReleaseProc*) override {
        return false;
    }

    void onSetLabel() override;

private:
    enum Wrapped { kWrapped };

    GrMtlTexture(GrMtlGpu*,
                 skgpu::Budgeted,
                 SkISize,
                 sk_sp<GrMtlAttachment>,
                 GrMipmapStatus,
                 std::string_view label);

    GrMtlTexture(GrMtlGpu*,
                 Wrapped,
                 SkISize,
                 sk_sp<GrMtlAttachment>,
                 GrMipmapStatus,
                 GrWrapCacheable,
                 GrIOType,
                 std::string_view label);

    sk_sp<GrMtlAttachment> fTexture;

    using INHERITED = GrTexture;
};

#endif
