/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_Graphite.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureUtils.h"

namespace skgpu::graphite {

Image::Image(TextureProxyView view,
             const SkColorInfo& info)
    : SkImage_Base(SkImageInfo::Make(view.proxy()->dimensions(), info),
                   kNeedNewImageUniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::~Image() {}

bool Image::testingOnly_ReadPixels(Context* context,
                                   Recorder* recorder,
                                   const SkImageInfo& dstInfo,
                                   void* dstPixels,
                                   size_t dstRowBytes,
                                   int srcX,
                                   int srcY) {
    return ReadPixelsHelper([recorder]() {
                                recorder->priv().flushTrackedDevices();
                            },
                            context,
                            recorder,
                            fTextureProxyView.proxy(),
                            dstInfo,
                            dstPixels,
                            dstRowBytes,
                            srcX,
                            srcY);
}

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

} // namespace skgpu::graphite

sk_sp<SkImage> SkImage::makeTextureImage(skgpu::graphite::Recorder* recorder,
                                         skgpu::graphite::Mipmapped mipmapped,
                                         SkBudgeted budgeted) const {
    if (!recorder) {
        return nullptr;
    }
    if (this->dimensions().area() <= 1) {
        mipmapped = skgpu::graphite::Mipmapped::kNo;
    }

    if (as_IB(this)->isGraphiteBacked()) {
        if (mipmapped == skgpu::graphite::Mipmapped::kNo || this->hasMipmaps()) {
            const SkImage* image = this;
            return sk_ref_sp(const_cast<SkImage*>(image));
        }
    }
    auto [view, ct] = as_IB(this)->asView(recorder, mipmapped, budgeted);
    if (!view) {
        return nullptr;
    }
    SkASSERT(view.proxy());
    SkASSERT(mipmapped == skgpu::graphite::Mipmapped::kNo ||
             view.proxy()->mipmapped() == skgpu::graphite::Mipmapped::kYes);
    SkColorInfo colorInfo(ct, this->alphaType(), this->refColorSpace());
    return sk_make_sp<skgpu::graphite::Image>(std::move(view),
                                              std::move(colorInfo));
}
