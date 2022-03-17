/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/graphite/src/Image_Graphite.h"

#include "experimental/graphite/include/Recorder.h"
#include "experimental/graphite/src/Caps.h"
#include "experimental/graphite/src/RecorderPriv.h"

namespace skgpu {

Image::Image(TextureProxyView view,
             const SkColorInfo& info)
    : SkImage_Base(SkImageInfo::Make(view.proxy()->dimensions(), info),
                   kNeedNewImageUniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::~Image() {}

std::tuple<TextureProxyView, SkColorType> Image::onAsView(Recorder*,
                                                          Mipmapped mipmapped,
                                                          SkBudgeted) const {
    if (fTextureProxyView.proxy()->mipmapped() != mipmapped) {
        // We will not generate miplevels
        return {};
    }
    SkColorType ct = this->colorType();
    return {fTextureProxyView, ct};
}

} // namespace skgpu

sk_sp<SkImage> SkImage::makeTextureImage(skgpu::Recorder* recorder,
                                         skgpu::Mipmapped mipmapped,
                                         SkBudgeted budgeted) const {
    if (!recorder) {
        return nullptr;
    }
    if (this->dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    if (as_IB(this)->isGraphiteBacked()) {
        if (mipmapped == skgpu::Mipmapped::kNo || this->hasMipmaps()) {
            const SkImage* image = this;
            return sk_ref_sp(const_cast<SkImage*>(image));
        }
    }
    auto [view, ct] = as_IB(this)->asView(recorder, mipmapped, budgeted);
    if (!view) {
        return nullptr;
    }
    SkASSERT(view.proxy());
    SkASSERT(mipmapped == skgpu::Mipmapped::kNo ||
             view.proxy()->mipmapped() == skgpu::Mipmapped::kYes);
    SkColorInfo colorInfo(ct, this->alphaType(), this->refColorSpace());
    return sk_make_sp<skgpu::Image>(std::move(view),
                                    std::move(colorInfo));
}



