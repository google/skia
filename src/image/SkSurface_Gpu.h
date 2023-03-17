/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_Gpu_DEFINED
#define SkSurface_Gpu_DEFINED

#include "include/core/SkTypes.h"

#if defined(SK_GANESH)
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/gpu/GrBackendSurface.h"
#include "src/image/SkSurface_Base.h"

class GrBackendSemaphore;
class GrRecordingContext;
class SkCanvas;
class SkCapabilities;
class SkColorSpace;
class SkDeferredDisplayList;
class SkImage;
class SkPaint;
class SkPixmap;
class SkSurface;
class SkSurfaceCharacterization;
enum GrSurfaceOrigin : int;
enum class GrSemaphoresSubmitted : bool;
namespace skgpu { class MutableTextureState; }
namespace skgpu {
namespace ganesh {
class Device;
}
}  // namespace skgpu
struct GrFlushInfo;
struct SkIPoint;
struct SkIRect;
struct SkISize;

class SkSurface_Gpu : public SkSurface_Base {
public:
    SkSurface_Gpu(sk_sp<skgpu::ganesh::Device>);
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
    skgpu::ganesh::Device* getDevice();

private:
    sk_sp<skgpu::ganesh::Device> fDevice;

    using INHERITED = SkSurface_Base;
};

#endif // defined(SK_GANESH)

#endif // SkSurface_Gpu_DEFINED
