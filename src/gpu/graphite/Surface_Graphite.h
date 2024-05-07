/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Surface_Graphite_DEFINED
#define skgpu_graphite_Surface_Graphite_DEFINED

#include "src/image/SkSurface_Base.h"

#include "include/gpu/GpuTypes.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/TextureProxyView.h"

namespace skgpu::graphite {

class Context;
class Device;
class Image;
class Recorder;
class TextureProxy;

class Surface final : public SkSurface_Base {
public:
    // Convenience factory to create a Device, instantiate its target proxy and return as a Surface.
    static sk_sp<Surface> Make(Recorder* recorder,
                               const SkImageInfo& info,
                               std::string_view label,
                               Budgeted budgeted,
                               Mipmapped mipmapped = Mipmapped::kNo,
                               SkBackingFit backingFit = SkBackingFit::kExact,
                               const SkSurfaceProps* props = nullptr) {
        return Make(recorder, info, std::move(label), budgeted, mipmapped, backingFit, props,
                    LoadOp::kClear, /*registerWithRecorder=*/true);
    }
    // Make a surface that is not registered with the provided recorder. This surface should be
    // short-lived and it must be flushed manually for its draw commands to be recorded. Most
    // scratch surfaces will be budgeted, but if the underlying texture is being returned as a
    // client-owned image, that may not be the case.
    static sk_sp<Surface> MakeScratch(Recorder* recorder,
                                      const SkImageInfo& info,
                                      std::string_view label,
                                      Budgeted budgeted = Budgeted::kYes,
                                      Mipmapped mipmapped = Mipmapped::kNo,
                                      SkBackingFit backingFit = SkBackingFit::kApprox) {
        return Make(recorder, info, std::move(label), budgeted, mipmapped, backingFit,
                    /*props=*/nullptr, LoadOp::kDiscard, /*registerWithRecorder=*/false);
    }

    Surface(sk_sp<Device>);
    ~Surface() override;

    // From SkSurface.h
    SkImageInfo imageInfo() const override;

    // From SkSurface_Base.h
    SkSurface_Base::Type type() const override { return SkSurface_Base::Type::kGraphite; }

    Recorder* onGetRecorder() const override;
    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&) override;
    sk_sp<SkImage> onNewImageSnapshot(const SkIRect* subset) override;
    void onWritePixels(const SkPixmap&, int x, int y) override;
    void onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                     SkIRect srcRect,
                                     RescaleGamma rescaleGamma,
                                     RescaleMode rescaleMode,
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
    sk_sp<const SkCapabilities> onCapabilities() override;

    TextureProxyView readSurfaceView() const;
    sk_sp<Image> asImage() const;
    sk_sp<Image> makeImageCopy(const SkIRect* subset, Mipmapped) const;
    TextureProxy* backingTextureProxy() const;

private:
    // Regular and scratch surfaces differ by initial clear and if they are registered or not,
    // otherwise are constructed the same.
    static sk_sp<Surface> Make(Recorder* recorder,
                               const SkImageInfo&,
                               std::string_view label,
                               Budgeted,
                               Mipmapped,
                               SkBackingFit,
                               const SkSurfaceProps* props,
                               LoadOp initialLoadOp,
                               bool registerWithRecorder);

    sk_sp<Device> fDevice;
    sk_sp<Image>  fImageView; // the image object returned by asImage()

    friend void Flush(SkSurface*);
};

// TODO: The long-term for the public API around surfaces and flushing/submitting will likely
// be replaced with explicit control over Recorders and submitting Recordings to the Context
// directly. For now, internal tools often rely on surface/canvas flushing to control what's
// being timed (nanobench or viewer's stats layer), so we flush any pending draws to a DrawPass.
// While this won't measure actual conversion of the task list to backend command buffers, that
// should be fairly negligible since most of the work is handled in DrawPass::Make().
// Additionally flushing pending work here ensures we don't batch across or clear prior recorded
// work when looping in a benchmark, as the controlling code expects.
void Flush(sk_sp<SkSurface> surface);
void Flush(SkSurface* surface);

} // namespace skgpu::graphite

#endif // skgpu_graphite_Surface_Graphite_DEFINED
