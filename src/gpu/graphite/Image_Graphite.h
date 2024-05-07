/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Image_Graphite_DEFINED
#define skgpu_graphite_Image_Graphite_DEFINED

#include "src/gpu/graphite/Image_Base_Graphite.h"

#include "src/gpu/graphite/TextureProxyView.h"

namespace skgpu {
    class RefCntedCallback;
}

namespace skgpu::graphite {

class Context;
class Device;
class Recorder;

class Image final : public Image_Base {
public:
    Image(TextureProxyView, const SkColorInfo&);
    ~Image() override;

    // Create an Image that wraps the Device and automatically flushes or references the Device's
    // pending tasks when the Image is used in a draw to another canvas.
    static sk_sp<Image> WrapDevice(sk_sp<Device> device);

    // Create an Image by copying the provided texture proxy view into a new texturable proxy.
    // The source texture does not have to be texturable if it is blittable.
    static sk_sp<Image> Copy(Recorder*,
                             const TextureProxyView& srcView,
                             const SkColorInfo&,
                             const SkIRect& subset,
                             Budgeted,
                             Mipmapped,
                             SkBackingFit,
                             std::string_view label);

    const TextureProxyView& textureProxyView() const { return fTextureProxyView; }

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kGraphite; }

    bool onHasMipmaps() const override {
        return fTextureProxyView.proxy()->mipmapped() == Mipmapped::kYes;
    }

    bool onIsProtected() const override {
        return fTextureProxyView.proxy()->isProtected();
    }

    size_t textureSize() const override;

    sk_sp<Image> copyImage(Recorder*,
                           const SkIRect& subset,
                           Budgeted,
                           Mipmapped,
                           SkBackingFit,
                           std::string_view label) const override;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override;

#if defined(GRAPHITE_TEST_UTILS)
    bool onReadPixelsGraphite(Recorder*,
                              const SkPixmap& dst,
                              int srcX,
                              int srcY) const override;
#endif

private:

    TextureProxyView fTextureProxyView;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Image_Graphite_DEFINED
