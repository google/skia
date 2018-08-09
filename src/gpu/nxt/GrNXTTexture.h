/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTTexture_DEFINED
#define GrNXTTexture_DEFINED

#include "GrTexture.h"
#include "dawn/dawncpp.h"

class GrNXTGpu;
struct GrNXTImageInfo;

class GrNXTTexture : public GrTexture {
public:
    static sk_sp<GrNXTTexture> Make(GrNXTGpu*, const GrSurfaceDesc&, SkBudgeted, GrMipMapsStatus);

    static sk_sp<GrNXTTexture> MakeWrapped(GrNXTGpu*, const GrSurfaceDesc&,
                                           GrWrapOwnership, const GrNXTImageInfo*);

    ~GrNXTTexture() override;

    GrBackendTexture getBackendTexture() const override;

    void textureParamsModified() override {}

    void upload(const GrMipLevel texels[], int mipLevels, dawn::CommandBufferBuilder copyBuilder);
    void upload(const GrMipLevel texels[], int mipLevels, const SkIRect& dstRect,
                dawn::CommandBufferBuilder copyBuilder);

    dawn::Texture texture() const { return fTexture.Clone(); }
    dawn::TextureView textureView() const { return fTextureView.Clone(); }
protected:
    GrNXTTexture(GrNXTGpu*, dawn::Texture texture, dawn::TextureView,
                 const GrSurfaceDesc&, const GrNXTImageInfo&, GrMipMapsStatus);

    GrNXTGpu* getNXTGpu() const;

    void setRelease(sk_sp<GrReleaseProcHelper> helper) override {
        // FIXME
    }

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
        return false;
    }

private:
    GrNXTTexture(GrNXTGpu*, const GrSurfaceDesc&, const GrNXTImageInfo&, GrMipMapsStatus);

    GrNXTImageInfo           fInfo;
    dawn::Texture            fTexture;
    dawn::TextureView        fTextureView;

    typedef GrTexture INHERITED;
};

#endif
