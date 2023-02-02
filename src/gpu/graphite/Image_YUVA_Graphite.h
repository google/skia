/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Image_YUVA_Graphite_DEFINED
#define skgpu_graphite_Image_YUVA_Graphite_DEFINED

#include "src/gpu/graphite/Image_Base_Graphite.h"
#include "src/gpu/graphite/YUVATextureProxies.h"

namespace skgpu::graphite {

class Recorder;

class Image_YUVA final : public Image_Base {
public:
    Image_YUVA(uint32_t uniqueID,
               YUVATextureProxies proxies,
               const SkColorInfo&);

    ~Image_YUVA() override {}

    bool isYUVA() const override { return true; }

    bool onHasMipmaps() const override {
        // TODO: Add mipmap support
        return false;
    }

    sk_sp<SkImage> onReinterpretColorSpace(sk_sp<SkColorSpace>) const override {
        return nullptr;
    }

private:
    sk_sp<SkImage> onMakeTextureImage(Recorder*, RequiredImageProperties) const override {
        return nullptr;
    }
    using Image_Base::onMakeSubset;
    sk_sp<SkImage> onMakeSubset(const SkIRect&, Recorder*, RequiredImageProperties) const override {
        return nullptr;
    }
    using Image_Base::onMakeColorTypeAndColorSpace;
    sk_sp<SkImage> onMakeColorTypeAndColorSpace(SkColorType targetCT,
                                                sk_sp<SkColorSpace> targetCS,
                                                Recorder*,
                                                RequiredImageProperties) const override {
        return nullptr;
    }

    mutable YUVATextureProxies fYUVAProxies;
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_Image_YUVA_Graphite_DEFINED
