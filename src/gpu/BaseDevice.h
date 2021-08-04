/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef BaseDevice_DEFINED
#define BaseDevice_DEFINED

#include "include/core/SkImage.h"
#include "include/private/GrTypesPriv.h"

class GrRenderTargetProxy;
namespace skgpu {
    class SurfaceFillContext;
#if SK_GPU_V1
    namespace v1 { class SurfaceDrawContext; }
#endif // SK_GPU_V1
}
class GrSurfaceProxyView;

// NOTE: when not defined, SkGpuDevice extends SkBaseDevice directly and manages its clip stack
// using GrClipStack. When false, SkGpuDevice continues to extend SkClipStackDevice and uses
// SkClipStack and GrClipStackClip to manage the clip stack.
#if !defined(SK_DISABLE_NEW_GR_CLIP_STACK)
    #include "src/core/SkDevice.h"
    #define BASE_DEVICE   SkBaseDevice
#else
    #include "src/core/SkClipStackDevice.h"
    #define BASE_DEVICE   SkClipStackDevice
#endif

namespace skgpu {

class BaseDevice : public BASE_DEVICE {
public:
    enum InitContents {
        kClear_InitContents,
        kUninit_InitContents
    };

    BaseDevice(sk_sp<GrRecordingContext>, const SkImageInfo&, const SkSurfaceProps&);

    virtual GrSurfaceProxyView readSurfaceView() = 0;

    BaseDevice* asGpuDevice() override { return this; }

#if SK_GPU_V1
    virtual skgpu::v1::SurfaceDrawContext* surfaceDrawContext() { return nullptr; }
#endif

    virtual skgpu::SurfaceFillContext* surfaceFillContext() = 0;
    GrRenderTargetProxy* targetProxy();
    GrRecordingContext* recordingContext() const { return fContext.get(); }

    virtual bool wait(int numSemaphores,
                      const GrBackendSemaphore* waitSemaphores,
                      bool deleteSemaphoresAfterWait) = 0;
    virtual void discard() = 0;

    virtual bool replaceBackingProxy(SkSurface::ContentChangeMode,
                                     sk_sp<GrRenderTargetProxy>,
                                     GrColorType,
                                     sk_sp<SkColorSpace>,
                                     GrSurfaceOrigin,
                                     const SkSurfaceProps&) = 0;
    bool replaceBackingProxy(SkSurface::ContentChangeMode);

    using RescaleGamma       = SkImage::RescaleGamma;
    using RescaleMode        = SkImage::RescaleMode;
    using ReadPixelsCallback = SkImage::ReadPixelsCallback;
    using ReadPixelsContext  = SkImage::ReadPixelsContext;

    virtual void asyncRescaleAndReadPixels(const SkImageInfo& info,
                                           const SkIRect& srcRect,
                                           RescaleGamma rescaleGamma,
                                           RescaleMode rescaleMode,
                                           ReadPixelsCallback callback,
                                           ReadPixelsContext context) = 0;

    virtual void asyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                                 sk_sp<SkColorSpace> dstColorSpace,
                                                 const SkIRect& srcRect,
                                                 SkISize dstSize,
                                                 RescaleGamma rescaleGamma,
                                                 RescaleMode,
                                                 ReadPixelsCallback callback,
                                                 ReadPixelsContext context) = 0;

protected:
    sk_sp<GrRecordingContext> fContext;

private:
    using INHERITED = BASE_DEVICE;
};

} // namespace skgpu

#undef BASE_DEVICE

#endif // BaseDevice_DEFINED
