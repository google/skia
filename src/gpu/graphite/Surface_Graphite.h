/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Surface_Graphite_DEFINED
#define skgpu_graphite_Surface_Graphite_DEFINED

#include "src/image/SkSurface_Base.h"

#include "src/gpu/graphite/TextureProxyView.h"

namespace skgpu::graphite {

class Context;
class Device;
class Recorder;
class TextureProxy;

class Surface final : public SkSurface_Base {
public:
    static sk_sp<SkSurface> MakeGraphite(Recorder* recorder,
                                         const SkImageInfo& info,
                                         skgpu::Budgeted budgeted,
                                         Mipmapped = Mipmapped::kNo,
                                         const SkSurfaceProps* props = nullptr);

    Surface(sk_sp<Device>);
    ~Surface() override;

    SkImageInfo imageInfo() const override;

    Recorder* onGetRecorder() override;
    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&) override;
    sk_sp<SkImage> onNewImageSnapshot(const SkIRect* subset) override;
    sk_sp<SkImage> onAsImage() override;
    sk_sp<SkImage> onMakeImageCopy(const SkIRect* subset, Mipmapped) override;
    void onWritePixels(const SkPixmap&, int x, int y) override;
    void onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                     SkIRect srcRect,
                                     RescaleGamma rescaleGamma,
                                     RescaleMode rescaleMode,
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
    sk_sp<const SkCapabilities> onCapabilities() override;
    bool isGraphiteBacked() const override { return true; }

    TextureProxyView readSurfaceView() const;

#if GRAPHITE_TEST_UTILS && SK_SUPPORT_GPU
    // TODO: The long-term for the public API around surfaces and flushing/submitting will likely
    // be replaced with explicit control over Recorders and submitting Recordings to the Context
    // directly. For now, internal tools often rely on surface/canvas flushing to control what's
    // being timed (nanobench or viewer's stats layer), so we flush any pending draws to a DrawPass.
    // While this won't measure actual conversion of the task list to backend command buffers, that
    // should be fairly negligible since most of the work is handled in DrawPass::Make().
    // Additionally flushing pending work here ensures we don't batch across or clear prior recorded
    // work when looping in a benchmark, as the controlling code expects.
    GrSemaphoresSubmitted onFlush(BackendSurfaceAccess access,
                                  const GrFlushInfo&,
                                  const skgpu::MutableTextureState*) override;
#endif

    TextureProxy* backingTextureProxy();

private:
    sk_sp<Device> fDevice;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Surface_Graphite_DEFINED
