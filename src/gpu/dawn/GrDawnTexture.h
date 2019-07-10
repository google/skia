/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnTexture_DEFINED
#define GrDawnTexture_DEFINED

#include "include/gpu/GrTexture.h"
#include "dawn/dawncpp.h"

class GrDawnGpu;
struct GrDawnImageInfo;

class GrDawnTexture : public GrTexture {
public:
    static sk_sp<GrDawnTexture> Make(GrDawnGpu*, const GrSurfaceDesc&, SkBudgeted, GrMipMapsStatus);

    static sk_sp<GrDawnTexture> MakeWrapped(GrDawnGpu*, const GrSurfaceDesc&,
                                            GrMipMapsStatus, GrWrapCacheable,
                                            const GrDawnImageInfo&);

    ~GrDawnTexture() override;

    GrBackendTexture getBackendTexture() const override;

    void textureParamsModified() override {}
    GrBackendFormat backendFormat() const override { return GrBackendFormat::MakeDawn(fInfo.fFormat); }

    void upload(const GrMipLevel texels[], int mipLevels, dawn::CommandEncoder copyEncoder);
    void upload(const GrMipLevel texels[], int mipLevels, const SkIRect& dstRect,
                dawn::CommandEncoder copyEncoder);

    dawn::Texture texture() const { return fTexture; }
    dawn::TextureView textureView() const { return fTextureView; }
protected:
    GrDawnTexture(GrDawnGpu*, dawn::Texture texture, dawn::TextureView,
                 const GrSurfaceDesc&, const GrDawnImageInfo&, GrMipMapsStatus);

    GrDawnGpu* getDawnGpu() const;

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
        return false;
    }

private:
    GrDawnTexture(GrDawnGpu*, const GrSurfaceDesc&, const GrDawnImageInfo&, GrMipMapsStatus);

    GrDawnImageInfo           fInfo;
    dawn::Texture            fTexture;
    dawn::TextureView        fTextureView;

    typedef GrTexture INHERITED;
};

#endif
