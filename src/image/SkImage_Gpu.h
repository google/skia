/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImage_Gpu_DEFINED
#define SkImage_Gpu_DEFINED

#include "src/core/SkImagePriv.h"
#include "src/gpu/GrGpuResourcePriv.h"
#include "src/gpu/GrSurfaceProxyPriv.h"
#include "src/gpu/GrSurfaceProxyView.h"
#include "src/gpu/SkGr.h"
#include "src/image/SkImage_GpuBase.h"

class GrDirectContext;
class GrRecordingContext;
class GrTexture;

class SkBitmap;
struct SkYUVAIndex;

class SkImage_Gpu : public SkImage_GpuBase {
public:
    SkImage_Gpu(sk_sp<GrImageContext>, uint32_t uniqueID, GrSurfaceProxyView, SkColorType,
                SkAlphaType, sk_sp<SkColorSpace>);
    ~SkImage_Gpu() override;

    GrSemaphoresSubmitted onFlush(GrDirectContext*, const GrFlushInfo&) override;

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

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                GrDirectContext*) const final;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

    void onAsyncRescaleAndReadPixels(const SkImageInfo&,
                                     const SkIRect& srcRect,
                                     RescaleGamma,
                                     SkFilterQuality,
                                     ReadPixelsCallback,
                                     ReadPixelsContext) override;

    void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                           sk_sp<SkColorSpace>,
                                           const SkIRect& srcRect,
                                           const SkISize& dstSize,
                                           RescaleGamma,
                                           SkFilterQuality,
                                           ReadPixelsCallback,
                                           ReadPixelsContext) override;

    /**
     * This is the implementation of SkDeferredDisplayListRecorder::makePromiseImage.
     */
    static sk_sp<SkImage> MakePromiseTexture(GrRecordingContext*,
                                             const GrBackendFormat& backendFormat,
                                             SkISize dimensions,
                                             GrMipmapped mipMapped,
                                             GrSurfaceOrigin origin,
                                             SkColorType colorType,
                                             SkAlphaType alphaType,
                                             sk_sp<SkColorSpace> colorSpace,
                                             PromiseImageTextureFulfillProc textureFulfillProc,
                                             PromiseImageTextureReleaseProc textureReleaseProc,
                                             PromiseImageTextureContext textureContext);

private:
    GrSurfaceProxyView fView;

    using INHERITED = SkImage_GpuBase;
};

#endif
