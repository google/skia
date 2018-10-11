/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDawnTexture_DEFINED
#define GrDawnTexture_DEFINED

#include "GrTexture.h"
#include "dawn/dawncpp.h"

class GrDawnGpu;
struct GrDawnImageInfo;

class GrDawnTexture : public GrTexture {
public:
    static sk_sp<GrDawnTexture> Make(GrDawnGpu*, const GrSurfaceDesc&, SkBudgeted, GrMipMapsStatus);

    static sk_sp<GrDawnTexture> MakeWrapped(GrDawnGpu*, const GrSurfaceDesc&,
                                           GrMipMapsStatus, const GrDawnImageInfo&);

    ~GrDawnTexture() override;

    GrBackendTexture getBackendTexture() const override;

    void textureParamsModified() override {}

    void upload(const GrMipLevel texels[], int mipLevels, dawn::CommandBufferBuilder copyBuilder);
    void upload(const GrMipLevel texels[], int mipLevels, const SkIRect& dstRect,
                dawn::CommandBufferBuilder copyBuilder);

    dawn::Texture texture() const { return fTexture.Clone(); }
    dawn::TextureView textureView() const { return fTextureView.Clone(); }
protected:
    GrDawnTexture(GrDawnGpu*, dawn::Texture texture, dawn::TextureView,
                 const GrSurfaceDesc&, const GrDawnImageInfo&, GrMipMapsStatus);

    GrDawnGpu* getDawnGpu() const;

    void setRelease(sk_sp<GrReleaseProcHelper> helper) override {
        // FIXME
    }

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
