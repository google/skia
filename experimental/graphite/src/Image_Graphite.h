/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Image_Graphite_DEFINED
#define skgpu_Image_Graphite_DEFINED

#include "src/image/SkImage_Base.h"

namespace skgpu {

class Image_Graphite final : public SkImage_Base {
public:
    Image_Graphite(const SkImageInfo&);
    ~Image_Graphite() override;

    bool onReadPixels(GrDirectContext*,
                      const SkImageInfo& dstInfo,
                      void* dstPixels,
                      size_t dstRowBytes,
                      int srcX,
                      int srcY,
                      CachingHint) const override { return false; }

    bool onHasMipmaps() const override { return false; }

    bool getROPixels(GrDirectContext*,
                     SkBitmap*,
                     CachingHint = kAllow_CachingHint) const override { return false; }

    sk_sp<SkImage> onMakeSubset(const SkIRect&, GrDirectContext*) const override {
        return nullptr;
    }

    bool onIsValid(GrRecordingContext*) const override { return true; }

    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType,
                                                sk_sp<SkColorSpace>,
                                                GrDirectContext*) const override {
        return nullptr;
    }

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override {
        return nullptr;
    }

protected:

private:
#if SK_SUPPORT_GPU
    std::tuple<GrSurfaceProxyView, GrColorType> onAsView(
            GrRecordingContext*,
            GrMipmapped,
            GrImageTexGenPolicy policy) const override {
        return {};
    }

    std::unique_ptr<GrFragmentProcessor> onAsFragmentProcessor(
            GrRecordingContext*,
            SkSamplingOptions,
            const SkTileMode[2],
            const SkMatrix&,
            const SkRect* subset,
            const SkRect* domain) const override;
#endif

};

} // namespace skgpu

#endif // skgpu_Image_Graphite_DEFINED
