/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/SpecialImage_Graphite.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "src/core/SkSpecialImage.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/TextureUtils.h"

namespace skgpu::graphite {

class SkSpecialImage_Graphite final : public SkSpecialImage {
public:
    SkSpecialImage_Graphite(const SkIRect& subset,
                            uint32_t uniqueID,
                            TextureProxyView view,
                            const SkColorInfo& colorInfo,
                            const SkSurfaceProps& props)
            : SkSpecialImage(subset, uniqueID, colorInfo, props)
            , fTextureProxyView(std::move(view)) {
    }

    size_t getSize() const override {
        // TODO: return VRAM size here
        return 0;
    }

    bool isGraphiteBacked() const override { return true; }

    TextureProxyView textureProxyView() const { return fTextureProxyView; }

    SkISize backingStoreDimensions() const override {
        return fTextureProxyView.proxy()->dimensions();
    }

    sk_sp<SkSpecialImage> onMakeBackingStoreSubset(const SkIRect& subset) const override {
        return SkSpecialImages::MakeGraphite(subset,
                                             this->uniqueID(),
                                             fTextureProxyView,
                                             this->colorInfo(),
                                             this->props());
    }

    sk_sp<SkImage> asImage() const override {
        return sk_make_sp<Image>(this->uniqueID(), fTextureProxyView, this->colorInfo());
    }

private:
    TextureProxyView fTextureProxyView;
};

} // namespace skgpu::graphite

namespace SkSpecialImages {

sk_sp<SkSpecialImage> MakeGraphite(skgpu::graphite::Recorder* recorder,
                                   const SkIRect& subset,
                                   sk_sp<SkImage> image,
                                   const SkSurfaceProps& props) {
    if (!recorder || !image || subset.isEmpty()) {
        return nullptr;
    }

    SkASSERT(image->bounds().contains(subset));

    // This will work even if the image is a raster-backed image and the Recorder's
    // client ImageProvider does a valid upload.
    if (!as_IB(image)->isGraphiteBacked()) {
        auto [graphiteImage, _] =
                skgpu::graphite::GetGraphiteBacked(recorder, image.get(), {});
        if (!graphiteImage) {
            return nullptr;
        }

        image = graphiteImage;
    }
    auto [view, ct] = skgpu::graphite::AsView(recorder, image.get(), skgpu::Mipmapped::kNo);
    return MakeGraphite(subset, image->uniqueID(), std::move(view),
                        {ct, image->alphaType(), image->refColorSpace()}, props);
}

sk_sp<SkSpecialImage> MakeGraphite(const SkIRect& subset,
                                   uint32_t uniqueID,
                                   skgpu::graphite::TextureProxyView view,
                                   const SkColorInfo& colorInfo,
                                   const SkSurfaceProps& props) {
    if (!view) {
        return nullptr;
    }

    SkASSERT(SkIRect::MakeSize(view.dimensions()).contains(subset));
    return sk_make_sp<skgpu::graphite::SkSpecialImage_Graphite>(subset, uniqueID,
                                                                std::move(view), colorInfo, props);
}

skgpu::graphite::TextureProxyView AsTextureProxyView(const SkSpecialImage* img) {
    if (!img || !img->isGraphiteBacked()) {
        return {};
    }
    auto grImg = static_cast<const skgpu::graphite::SkSpecialImage_Graphite*>(img);
    return grImg->textureProxyView();
}

}  // namespace SkSpecialImages
