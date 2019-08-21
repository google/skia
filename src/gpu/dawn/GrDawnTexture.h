/*
 * Copyright 2019 Google Inc.
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
    static sk_sp<GrDawnTexture> Make(GrDawnGpu*, const SkISize& size, GrPixelConfig config,
                                     dawn::TextureFormat format, GrRenderable, int sampleCnt,
                                     SkBudgeted, int mipLevels, GrMipMapsStatus);

    static sk_sp<GrDawnTexture> MakeWrapped(GrDawnGpu*, const SkISize& size, GrPixelConfig config,
                                            GrMipMapsStatus, GrWrapCacheable,
                                            const GrDawnImageInfo&);

    ~GrDawnTexture() override;

    GrBackendTexture getBackendTexture() const override;
    GrBackendFormat backendFormat() const override;

    void textureParamsModified() override {}

    void upload(const GrMipLevel texels[], int mipLevels);
    void upload(const GrMipLevel texels[], int mipLevels, const SkIRect& dstRect);

    dawn::Texture texture() const { return fInfo.fTexture; }
    dawn::TextureView textureView() const { return fTextureView; }
protected:
    GrDawnTexture(GrDawnGpu*, const SkISize& size, GrPixelConfig config,
                  dawn::TextureView, const GrDawnImageInfo&, GrMipMapsStatus);

    GrDawnGpu* getDawnGpu() const;

    void onAbandon() override;
    void onRelease() override;

    bool onStealBackendTexture(GrBackendTexture*, SkImage::BackendTextureReleaseProc*) override {
        return false;
    }

private:
    GrDawnTexture(GrDawnGpu*, const GrSurfaceDesc&, const GrDawnImageInfo&, GrMipMapsStatus);

    GrDawnImageInfo          fInfo;
    dawn::TextureView        fTextureView;

    typedef GrTexture INHERITED;
};

#endif
