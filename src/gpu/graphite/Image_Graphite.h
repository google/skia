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
class Recorder;

class Image final : public Image_Base {
public:
    Image(uint32_t uniqueID, TextureProxyView, const SkColorInfo&);
    Image(TextureProxyView, const SkColorInfo&);
    ~Image() override;

    bool onHasMipmaps() const override {
        return fTextureProxyView.proxy()->mipmapped() == skgpu::Mipmapped::kYes;
    }

    using Image_Base::onMakeSubset;

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override;

    TextureProxyView textureProxyView() const { return fTextureProxyView; }

    static sk_sp<TextureProxy> MakePromiseImageLazyProxy(SkISize dimensions,
                                                         TextureInfo,
                                                         Volatile,
                                                         GraphitePromiseImageFulfillProc,
                                                         sk_sp<RefCntedCallback>,
                                                         GraphitePromiseTextureReleaseProc);

private:
    sk_sp<SkImage> onMakeTextureImage(Recorder*, RequiredImageProperties) const override;
    sk_sp<SkImage> copyImage(const SkIRect& subset, Recorder*, RequiredImageProperties) const;
    sk_sp<SkImage> onMakeSubset(const SkIRect&, Recorder*, RequiredImageProperties) const override;
    using Image_Base::onMakeColorTypeAndColorSpace;
    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                sk_sp<SkColorSpace> targetCS,
                                                Recorder*,
                                                RequiredImageProperties) const override;

    TextureProxyView fTextureProxyView;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Image_Graphite_DEFINED
