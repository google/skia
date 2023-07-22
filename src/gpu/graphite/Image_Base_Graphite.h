/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Image_Base_Graphite_DEFINED
#define skgpu_graphite_Image_Base_Graphite_DEFINED

#include "src/image/SkImage_Base.h"

namespace skgpu::graphite {

class Context;
class Recorder;
class TextureProxy;

class Image_Base : public SkImage_Base {
public:
    ~Image_Base() override {}

    bool onReadPixels(GrDirectContext*,
                      const SkImageInfo& dstInfo,
                      void* dstPixels,
                      size_t dstRowBytes,
                      int srcX,
                      int srcY,
                      CachingHint) const override { return false; }

    // From SkImage.h
    // TODO(egdaniel) This feels wrong. Re-think how this method is used and works.
    bool isValid(GrRecordingContext*) const override { return true; }

    // From SkImage_Base.h
    SkImage_Base::Type type() const override { return SkImage_Base::Type::kGraphite; }

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

    virtual sk_sp<SkImage> makeTextureImage(Recorder*, RequiredProperties) const = 0;

protected:
    Image_Base(const SkImageInfo& info, uint32_t uniqueID)
        : SkImage_Base(info, uniqueID) {}

};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Image_Base_Graphite_DEFINED
