/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Image_YUVA_Graphite_DEFINED
#define skgpu_graphite_Image_YUVA_Graphite_DEFINED

#include "src/gpu/graphite/Image_Base_Graphite.h"

#include "include/gpu/graphite/Image.h"
#include "src/gpu/graphite/YUVATextureProxies.h"

namespace skgpu {
    class RefCntedCallback;
}

namespace skgpu::graphite {

class Recorder;

class Image_YUVA final : public Image_Base {
public:
    Image_YUVA(uint32_t uniqueID,
               YUVATextureProxies proxies,
               sk_sp<SkColorSpace>);

    ~Image_YUVA() override {}

    SkImage_Base::Type type() const override { return SkImage_Base::Type::kGraphiteYUVA; }

    size_t textureSize() const override;

    bool onHasMipmaps() const override { return fYUVAProxies.mipmapped() == Mipmapped::kYes; }

    bool onIsProtected() const override {
        // TODO: add protected content support
        return false;
    }

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override;

    const YUVATextureProxies& yuvaProxies() const {
        return fYUVAProxies;
    }

    static sk_sp<TextureProxy> MakePromiseImageLazyProxy(
            const Caps*,
            SkISize dimensions,
            TextureInfo,
            Volatile,
            SkImages::GraphitePromiseImageYUVAFulfillProc,
            sk_sp<RefCntedCallback>,
            SkImages::GraphitePromiseTextureContext,
            SkImages::GraphitePromiseTextureReleaseProc);

private:
    sk_sp<SkImage> makeTextureImage(Recorder*, RequiredProperties) const override {
        return nullptr;
    }
    using Image_Base::onMakeSubset;
    sk_sp<SkImage> onMakeSubset(Recorder*, const SkIRect&, RequiredProperties) const override {
        return nullptr;
    }
    using Image_Base::onMakeColorTypeAndColorSpace;
    sk_sp<SkImage> makeColorTypeAndColorSpace(Recorder*,
                                              SkColorType targetCT,
                                              sk_sp<SkColorSpace> targetCS,
                                              RequiredProperties) const override {
        return nullptr;
    }

    mutable YUVATextureProxies fYUVAProxies;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Image_YUVA_Graphite_DEFINED
