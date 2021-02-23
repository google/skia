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
#include "src/image/SkImage_GpuBase.h"

class GrDirectContext;
class GrRecordingContext;
class GrTexture;

class SkBitmap;

class SkImage_Gpu final : public SkImage_GpuBase {
public:
    SkImage_Gpu(sk_sp<GrImageContext>, uint32_t uniqueID, GrSurfaceProxyView, SkColorType,
                SkAlphaType, sk_sp<SkColorSpace>);
    SkImage_Gpu(sk_sp<GrImageContext> context,
                uint32_t uniqueID,
                GrSurfaceProxyView view,
                SkColorInfo info)
            : SkImage_Gpu(std::move(context),
                          uniqueID,
                          std::move(view),
                          info.colorType(),
                          info.alphaType(),
                          info.refColorSpace()) {}

    ~SkImage_Gpu() override;

    // If this is image is a cached SkSurface snapshot then this method is called by the SkSurface
    // before a write to check if the surface must make a copy to avoid modifying the image's
    // contents.
    bool surfaceMustCopyOnWrite(GrSurfaceProxy* surfaceProxy) const;

    bool onHasMipmaps() const override {
        return fView.asTextureProxy()->mipmapped() == GrMipmapped::kYes;
    }

    GrSemaphoresSubmitted onFlush(GrDirectContext*, const GrFlushInfo&) override;

    GrBackendTexture onGetBackendTexture(bool flushPendingGrContextIO,
                                         GrSurfaceOrigin* origin) const final;

    bool onIsTextureBacked() const override {
        SkASSERT(fView.proxy());
        return true;
    }

    size_t onTextureSize() const override { return fView.proxy()->gpuMemorySize(); }

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType, sk_sp<SkColorSpace>,
                                                GrDirectContext*) const final;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const final;

    void onAsyncRescaleAndReadPixels(const SkImageInfo&,
                                     const SkIRect& srcRect,
                                     RescaleGamma,
                                     RescaleMode,
                                     ReadPixelsCallback,
                                     ReadPixelsContext) override;

    void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                           sk_sp<SkColorSpace>,
                                           const SkIRect& srcRect,
                                           const SkISize& dstSize,
                                           RescaleGamma,
                                           RescaleMode,
                                           ReadPixelsCallback,
                                           ReadPixelsContext) override;

private:
    std::tuple<GrSurfaceProxyView, GrColorType> onAsView(GrRecordingContext*,
                                                         GrMipmapped,
                                                         GrImageTexGenPolicy) const override;

    GrSurfaceProxyView fView;

    using INHERITED = SkImage_GpuBase;
};

#endif
