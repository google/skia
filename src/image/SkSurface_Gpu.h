/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_Gpu_DEFINED
#define SkSurface_Gpu_DEFINED

#include "include/private/GrTypesPriv.h"
#include "src/image/SkSurface_Base.h"

#if SK_SUPPORT_GPU

class GrBackendFormat;
namespace skgpu { class BaseDevice; }

class SkSurface_Gpu : public SkSurface_Base {
public:
    SkSurface_Gpu(sk_sp<skgpu::BaseDevice>);
    ~SkSurface_Gpu() override;

    GrRecordingContext* onGetRecordingContext() override;

    GrBackendTexture onGetBackendTexture(BackendHandleAccess) override;
    GrBackendRenderTarget onGetBackendRenderTarget(BackendHandleAccess) override;
    bool onReplaceBackendTexture(const GrBackendTexture&, GrSurfaceOrigin, ContentChangeMode, TextureReleaseProc,
                                 ReleaseContext) override;

    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&) override;
    sk_sp<SkImage> onNewImageSnapshot(const SkIRect* subset) override;
    void onWritePixels(const SkPixmap&, int x, int y) override;
    void onAsyncRescaleAndReadPixels(const SkImageInfo& info, const SkIRect& srcRect,
                                     RescaleGamma rescaleGamma, RescaleMode,
                                     ReadPixelsCallback callback,
                                     ReadPixelsContext context) override;
    void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                           sk_sp<SkColorSpace> dstColorSpace,
                                           const SkIRect& srcRect,
                                           const SkISize& dstSize,
                                           RescaleGamma rescaleGamma,
                                           RescaleMode,
                                           ReadPixelsCallback callback,
                                           ReadPixelsContext context) override;
#ifdef SK_SURFACE_COPY_ON_WRITE_CRASHES
    void onCopyOnWrite(ContentChangeMode) override;
#else
    bool onCopyOnWrite(ContentChangeMode) override;
#endif
    void onDiscard() override;
    GrSemaphoresSubmitted onFlush(BackendSurfaceAccess access, const GrFlushInfo& info,
                                  const GrBackendSurfaceMutableState*) override;
    bool onWait(int numSemaphores, const GrBackendSemaphore* waitSemaphores,
                 bool deleteSemaphoresAfterWait) override;
    bool onCharacterize(SkSurfaceCharacterization*) const override;
    bool onIsCompatible(const SkSurfaceCharacterization&) const override;
    void onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkSamplingOptions&,
                const SkPaint* paint) override;
    bool onDraw(sk_sp<const SkDeferredDisplayList>, SkIPoint offset) override;

    skgpu::BaseDevice* getDevice();

private:
    sk_sp<skgpu::BaseDevice> fDevice;

    using INHERITED = SkSurface_Base;
};

#endif // SK_SUPPORT_GPU

#endif // SkSurface_Gpu_DEFINED
