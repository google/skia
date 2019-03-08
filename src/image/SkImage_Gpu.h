/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Gpu_DEFINED
#define SkImage_Gpu_DEFINED

#include "GrContext.h"
#include "GrGpuResourcePriv.h"
#include "GrSurfaceProxyPriv.h"
#include "SkGr.h"
#include "SkImagePriv.h"
#include "SkImage_GpuBase.h"

class GrTexture;

class SkBitmap;
struct SkYUVAIndex;

class SkImage_Gpu : public SkImage_GpuBase {
public:
    SkImage_Gpu(sk_sp<GrContext>, uint32_t uniqueID, SkAlphaType, sk_sp<GrTextureProxy>,
                sk_sp<SkColorSpace>);
    ~SkImage_Gpu() override;

    SkImageInfo onImageInfo() const override;

    GrTextureProxy* peekProxy() const override {
        return fProxy.get();
    }
    sk_sp<GrTextureProxy> asTextureProxyRef(GrRecordingContext*) const override {
        return fProxy;
    }

    bool onIsTextureBacked() const override { return SkToBool(fProxy.get()); }

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(GrRecordingContext*,
                                                SkColorType, sk_sp<SkColorSpace>) const final;

    /**
     * This is the implementation of SkDeferredDisplayListRecorder::makePromiseImage.
     */
    static sk_sp<SkImage> MakePromiseTexture(GrContext* context,
                                             const GrBackendFormat& backendFormat,
                                             int width,
                                             int height,
                                             GrMipMapped mipMapped,
                                             GrSurfaceOrigin origin,
                                             SkColorType colorType,
                                             SkAlphaType alphaType,
                                             sk_sp<SkColorSpace> colorSpace,
                                             PromiseImageTextureFulfillProc textureFulfillProc,
                                             PromiseImageTextureReleaseProc textureReleaseProc,
                                             PromiseImageTextureDoneProc textureDoneProc,
                                             PromiseImageTextureContext textureContext,
                                             PromiseImageApiVersion);

    static sk_sp<SkImage> ConvertYUVATexturesToRGB(GrContext*, SkYUVColorSpace yuvColorSpace,
                                                   const GrBackendTexture yuvaTextures[],
                                                   const SkYUVAIndex yuvaIndices[4],
                                                   SkISize imageSize, GrSurfaceOrigin imageOrigin,
                                                   GrRenderTargetContext*);

private:
    sk_sp<GrTextureProxy> fProxy;

    typedef SkImage_GpuBase INHERITED;
};

#endif
