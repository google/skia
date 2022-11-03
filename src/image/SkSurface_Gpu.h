/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_Gpu_DEFINED
#define SkSurface_Gpu_DEFINED

#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/image/SkSurface_Base.h"

#if SK_SUPPORT_GPU

class GrBackendFormat;
namespace skgpu::v1 { class Device; }

class SkSurface_Gpu : public SkSurface_Base {
public:
    SkSurface_Gpu(sk_sp<skgpu::v1::Device>);
    ~SkSurface_Gpu() override;

    SkImageInfo imageInfo() const override;

    GrRecordingContext* onGetRecordingContext() override;

    GrBackendTexture onGetBackendTexture(BackendHandleAccess) override;
    GrBackendRenderTarget onGetBackendRenderTarget(BackendHandleAccess) override;
    bool onReplaceBackendTexture(const GrBackendTexture&, GrSurfaceOrigin, ContentChangeMode, TextureReleaseProc,
                                 ReleaseContext) override;

    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&) override;
    sk_sp<SkImage> onNewImageSnapshot(const SkIRect* subset) override;
    void onWritePixels(const SkPixmap&, int x, int y) override;
    void onAsyncReadPixels(const SkImageInfo& info, SkIRect srcRect,
                           ReadPixelsCallback callback, ReadPixelsContext context) override;
    void onAsyncRescaleAndReadPixels(const SkImageInfo& info, SkIRect srcRect,
                                     RescaleGamma rescaleGamma, RescaleMode,
                                     ReadPixelsCallback callback,
                                     ReadPixelsContext context) override;
    void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                           sk_sp<SkColorSpace> dstColorSpace,
                                           SkIRect srcRect,
                                           SkISize dstSize,
                                           RescaleGamma rescaleGamma,
                                           RescaleMode,
                                           ReadPixelsCallback callback,
                                           ReadPixelsContext context) override;
    bool onCopyOnWrite(ContentChangeMode) override;
    void onDiscard() override;
    void onResolveMSAA() override;
    GrSemaphoresSubmitted onFlush(BackendSurfaceAccess access, const GrFlushInfo& info,
                                  const skgpu::MutableTextureState*) override;
    bool onWait(int numSemaphores, const GrBackendSemaphore* waitSemaphores,
                 bool deleteSemaphoresAfterWait) override;
    bool onCharacterize(SkSurfaceCharacterization*) const override;
    bool onIsCompatible(const SkSurfaceCharacterization&) const override;
    void onDraw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkSamplingOptions&,
                const SkPaint* paint) override;
    bool onDraw(sk_sp<const SkDeferredDisplayList>, SkIPoint offset) override;

    sk_sp<const SkCapabilities> onCapabilities() override;
    skgpu::v1::Device* getDevice();

private:
    sk_sp<skgpu::v1::Device> fDevice;

    using INHERITED = SkSurface_Base;
};

#endif // SK_SUPPORT_GPU

#endif // SkSurface_Gpu_DEFINED
