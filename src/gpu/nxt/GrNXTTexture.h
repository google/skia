/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrNXTTexture_DEFINED
#define GrNXTTexture_DEFINED

#include "GrTexture.h"
#include "nxt/nxtcpp.h"

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

    void upload(const GrMipLevel texels[], int mipLevels, nxt::CommandBufferBuilder copyBuilder);
    void upload(const GrMipLevel texels[], int mipLevels, const SkIRect& dstRect,
                nxt::CommandBufferBuilder copyBuilder);

    nxt::Texture texture() const { return fTexture.Clone(); }
    nxt::TextureView textureView() const { return fTextureView.Clone(); }
protected:
    GrNXTTexture(GrNXTGpu*, nxt::Texture texture, nxt::TextureView,
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
    nxt::Texture             fTexture;
    nxt::TextureView         fTextureView;

    typedef GrTexture INHERITED;
};

#endif
