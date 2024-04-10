/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Image_Base_Graphite_DEFINED
#define skgpu_graphite_Image_Base_Graphite_DEFINED

#include "include/gpu/GpuTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkSpinlock.h"
#include "src/image/SkImage_Base.h"

enum class SkBackingFit;

namespace skgpu::graphite {

class Context;
class Device;
class Image;
class Recorder;
class TextureProxy;

class Image_Base : public SkImage_Base {
public:
    ~Image_Base() override;

    // Must be called at the time of recording an action that reads from the image, be it a draw
    // or a copy operation.
    void notifyInUse(Recorder*) const;

    // Returns true if this image is linked to a device that may render their shared texture(s).
    bool isDynamic() const;

    // Always copy this image, even if 'subset' and mipmapping match this image exactly.
    // The base implementation performs all copies as draws.
    virtual sk_sp<Image> copyImage(Recorder*, const SkIRect& subset,
                                   Budgeted, Mipmapped, SkBackingFit) const;

    // From SkImage.h
    // TODO(egdaniel) This feels wrong. Re-think how this method is used and works.
    bool isValid(GrRecordingContext*) const override { return true; }

    // From SkImage_Base.h
    sk_sp<SkImage> onMakeSubset(Recorder*, const SkIRect&, RequiredProperties) const override;
    sk_sp<SkImage> makeColorTypeAndColorSpace(Recorder*,
                                              SkColorType targetCT,
                                              sk_sp<SkColorSpace> targetCS,
                                              RequiredProperties) const override;

    // No-ops for Ganesh APIs
    bool onReadPixels(GrDirectContext*,
                      const SkImageInfo& dstInfo,
                      void* dstPixels,
                      size_t dstRowBytes,
                      int srcX,
                      int srcY,
                      CachingHint) const override { return false; }

    bool getROPixels(GrDirectContext*,
                     SkBitmap*,
                     CachingHint = kAllow_CachingHint) const override { return false; }

    sk_sp<SkImage> onMakeSubset(GrDirectContext*, const SkIRect&) const override;

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType,
                                                sk_sp<SkColorSpace>,
                                                GrDirectContext*) const override;

    void onAsyncRescaleAndReadPixels(const SkImageInfo&,
                                     SkIRect srcRect,
                                     RescaleGamma,
                                     RescaleMode,
                                     ReadPixelsCallback,
                                     ReadPixelsContext) const override;

    void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace,
                                           bool readAlpha,
                                           sk_sp<SkColorSpace>,
                                           SkIRect srcRect,
                                           SkISize dstSize,
                                           RescaleGamma,
                                           RescaleMode,
                                           ReadPixelsCallback,
                                           ReadPixelsContext) const override;

protected:
    Image_Base(const SkImageInfo& info, uint32_t uniqueID);

    // Create a new flattened copy of the base image using draw operations.
    static sk_sp<Image> CopyAsDraw(Recorder*,
                                   const Image_Base*,
                                   const SkIRect& subset,
                                   const SkColorInfo& dstColorInfo,
                                   Budgeted, Mipmapped, SkBackingFit);

    // If the passed-in image is linked with Devices that modify its texture, copy the links to
    // this Image. This is used when a new Image is created that shares the same texture proxy as
    // a dynamic image. This can only be called before the Image has been returned from a factory.
    void linkDevices(const Image_Base*);
    // Link this image to the Device that can write to their shared texture proxy, so that when the
    // image is sampled in a draw, any pending work from the Device is automatically flushed. This
    // can only be called before the Image has been returned from a factory function.
    void linkDevice(sk_sp<Device>);

private:
    // Devices are flushed in notifyImageInUse(). If a linked device is uniquely held by the image
    // or if it's marked as immutable, it will be unlinked (allowing it to be destroyed eventually).
    // If all linked devices are removed, this array will become empty. Other than initialization,
    // this array cannot transition from an empty state to having linked devices, so while it's
    // empty no locking is required.
    mutable skia_private::STArray<1, sk_sp<Device>> fLinkedDevices SK_GUARDED_BY(fDeviceLinkLock);
    mutable SkSpinlock fDeviceLinkLock;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Image_Base_Graphite_DEFINED
