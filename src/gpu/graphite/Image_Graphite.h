/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Image_Graphite_DEFINED
#define skgpu_graphite_Image_Graphite_DEFINED

#include "src/gpu/graphite/Image_Base_Graphite.h"

#include "include/gpu/graphite/Image.h"
#include "src/gpu/graphite/TextureProxyView.h"

namespace skgpu {
    class RefCntedCallback;
}

namespace skgpu::graphite {

class Context;
class Recorder;

class Image final : public Image_Base {
public:
    Image(uint32_t uniqueID, TextureProxyView, const SkColorInfo&);
    ~Image() override;

    bool onHasMipmaps() const override {
        return fTextureProxyView.proxy()->mipmapped() == skgpu::Mipmapped::kYes;
    }

    bool onIsProtected() const override {
        return fTextureProxyView.proxy()->isProtected();
    }

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kGraphite; }

    size_t textureSize() const override;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override;

    const TextureProxyView& textureProxyView() const { return fTextureProxyView; }

    static sk_sp<TextureProxy> MakePromiseImageLazyProxy(
            const Caps*,
            SkISize dimensions,
            TextureInfo,
            Volatile,
            SkImages::GraphitePromiseImageFulfillProc,
            sk_sp<RefCntedCallback>,
            SkImages::GraphitePromiseTextureReleaseProc);
    sk_sp<SkImage> makeTextureImage(Recorder*, RequiredProperties) const override;

private:
    sk_sp<SkImage> copyImage(const SkIRect& subset, Recorder*, RequiredProperties) const;
    using Image_Base::onMakeSubset;
    sk_sp<SkImage> onMakeSubset(Recorder*, const SkIRect&, RequiredProperties) const override;
    using Image_Base::onMakeColorTypeAndColorSpace;
    sk_sp<SkImage> makeColorTypeAndColorSpace(Recorder*,
                                              SkColorType targetCT,
                                              sk_sp<SkColorSpace> targetCS,
                                              RequiredProperties) const override;

    TextureProxyView fTextureProxyView;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Image_Graphite_DEFINED
