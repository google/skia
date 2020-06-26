/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Gpu_DEFINED
#define SkImage_Gpu_DEFINED

#include "include/gpu/GrContext.h"
#include "src/core/SkImagePriv.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/SkGr.h"
#include "src/image/SkImage_GpuBase.h"

class GrTexture;

class SkBitmap;
struct SkYUVAIndex;

class SkImage_Gpu : public SkImage_GpuBase {
public:
    SkImage_Gpu(sk_sp<GrContext>, uint32_t uniqueID, GrSurfaceProxyView, SkColorType, SkAlphaType,
                sk_sp<SkColorSpace>);
    ~SkImage_Gpu() override;

    GrSemaphoresSubmitted onFlush(GrContext*, const GrFlushInfo&) override;

    GrTextureProxy* peekProxy() const override {
        return fView.asTextureProxy();
    }

    const GrSurfaceProxyView* view(GrRecordingContext* context) const override {
        if (!fView.proxy()) {
            return nullptr;
        }
        return &fView;
    }

    bool onIsTextureBacked() const override {
        SkASSERT(fView.proxy());
        return true;
    }

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(GrRecordingContext*,
                                                SkColorType, sk_sp<SkColorSpace>) const final;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

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
    GrSurfaceProxyView fView;

    typedef SkImage_GpuBase INHERITED;
};

#endif
