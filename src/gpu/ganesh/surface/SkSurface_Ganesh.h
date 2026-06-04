/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkSurface_Ganesh_DEFINED
#define SkSurface_Ganesh_DEFINED

#include "include/core/SkTypes.h"

#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/gpu/ganesh/GrBackendSurface.h"
#include "src/image/SkSurface_Base.h"

class GrBackendSemaphore;
class GrDeferredDisplayList;
class GrRecordingContext;
class GrSurfaceCharacterization;
class SkCanvas;
class SkCapabilities;
class SkColorSpace;
class SkImage;
class SkPaint;
class SkPixmap;
class SkSurface;
enum GrSurfaceOrigin : int;
namespace skgpu {
namespace ganesh {
class Device;
}
}  // namespace skgpu
struct SkIRect;
struct SkISize;

class SkSurface_Ganesh : public SkSurface_Base {
public:
    SkSurface_Ganesh(sk_sp<skgpu::ganesh::Device>);
    ~SkSurface_Ganesh() override;

    // From SkSurface.h
    SkImageInfo imageInfo() const override;
    bool replaceBackendTexture(const GrBackendTexture&,
                               GrSurfaceOrigin,
                               ContentChangeMode,
                               TextureReleaseProc,
                               ReleaseContext) override;

    // From SkSurface_Base.h
    SkSurface_Base::Type type() const override { return SkSurface_Base::Type::kGanesh; }

    GrRecordingContext* onGetRecordingContext() const override;

    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&) override;
    sk_sp<SkImage> onNewImageSnapshot(const SkIRect* subset) override;
    void onWritePixels(const SkPixmap&, int x, int y) override;
    void onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                     SkIRect srcRect,
                                     RescaleGamma rescaleGamma,
                                     RescaleMode,
                                     ReadPixelsCallback callback,
                                     ReadPixelsContext context) override;
    void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                           bool readAlpha,
                                           sk_sp<SkColorSpace> dstColorSpace,
                                           SkIRect srcRect,
                                           SkISize dstSize,
                                           RescaleGamma rescaleGamma,
                                           RescaleMode,
                                           ReadPixelsCallback callback,
                                           ReadPixelsContext context) override;
    bool onCopyOnWrite(ContentChangeMode) override;
    void onDiscard() override;
    bool onWait(int numSemaphores,
                const GrBackendSemaphore* waitSemaphores,
                bool deleteSemaphoresAfterWait) override;
    bool onCharacterize(GrSurfaceCharacterization*) const override;
    bool onIsCompatible(const GrSurfaceCharacterization&) const override;
    void onDraw(SkCanvas* canvas,
                SkScalar x,
                SkScalar y,
                const SkSamplingOptions&,
                const SkPaint* paint) override;

    sk_sp<const SkCapabilities> onCapabilities() override;

    skgpu::ganesh::Device* getDevice();
    GrBackendTexture getBackendTexture(BackendHandleAccess);
    GrBackendRenderTarget getBackendRenderTarget(BackendHandleAccess);
    void resolveMSAA();
    bool draw(sk_sp<const GrDeferredDisplayList>);

private:
    sk_sp<skgpu::ganesh::Device> fDevice;

    using INHERITED = SkSurface_Base;
};

#endif  // SkSurface_Ganesh_DEFINED
