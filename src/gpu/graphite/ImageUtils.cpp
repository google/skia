/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ImageUtils.h"

#include "include/core/SkBitmap.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkSpecialSurface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"
#include "src/image/SkImage_Base.h"

namespace {

bool valid_client_provided_image(const SkImage* clientProvided,
                                 const SkImage* original,
                                 SkImage::RequiredProperties requiredProps) {
    if (!clientProvided ||
        !as_IB(clientProvided)->isGraphiteBacked() ||
        original->dimensions() != clientProvided->dimensions() ||
        original->alphaType() != clientProvided->alphaType()) {
        return false;
    }

    uint32_t origChannels = SkColorTypeChannelFlags(original->colorType());
    uint32_t clientChannels = SkColorTypeChannelFlags(clientProvided->colorType());
    if ((origChannels & clientChannels) != origChannels) {
        return false;
    }

    return true;
}

} // anonymous namespace

namespace skgpu::graphite {

std::pair<sk_sp<SkImage>, SkSamplingOptions> GetGraphiteBacked(Recorder* recorder,
                                                               const SkImage* imageIn,
                                                               SkSamplingOptions sampling) {
    skgpu::Mipmapped mipmapped = (sampling.mipmap != SkMipmapMode::kNone)
                                     ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;

    if (imageIn->dimensions().area() <= 1 && mipmapped == skgpu::Mipmapped::kYes) {
        mipmapped = skgpu::Mipmapped::kNo;
        sampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
    }

    sk_sp<SkImage> result;
    if (as_IB(imageIn)->isGraphiteBacked()) {
        result = sk_ref_sp(imageIn);

        // If the preexisting Graphite-backed image doesn't have the required mipmaps we will drop
        // down the sampling
        if (mipmapped == skgpu::Mipmapped::kYes && !result->hasMipmaps()) {
            mipmapped = skgpu::Mipmapped::kNo;
            sampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
        }
    } else {
        auto clientImageProvider = recorder->clientImageProvider();
        result = clientImageProvider->findOrCreate(
                recorder, imageIn, {mipmapped == skgpu::Mipmapped::kYes});

        if (!valid_client_provided_image(
                    result.get(), imageIn, {mipmapped == skgpu::Mipmapped::kYes})) {
            // The client did not fulfill the ImageProvider contract so drop the image.
            result = nullptr;
        }
    }

    if (sampling.isAniso() && result) {
        sampling = SkSamplingPriv::AnisoFallback(result->hasMipmaps());
    }

    return { result, sampling };
}

std::tuple<skgpu::graphite::TextureProxyView, SkColorType> AsView(Recorder* recorder,
                                                                  const SkImage* image,
                                                                  skgpu::Mipmapped mipmapped) {
    if (!recorder || !image) {
        return {};
    }

    if (!as_IB(image)->isGraphiteBacked()) {
        return {};
    }
    // TODO(b/238756380): YUVA not supported yet
    if (as_IB(image)->isYUVA()) {
        return {};
    }

    auto gi = reinterpret_cast<const skgpu::graphite::Image*>(image);

    if (gi->dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    if (mipmapped == skgpu::Mipmapped::kYes &&
        gi->textureProxyView().proxy()->mipmapped() != skgpu::Mipmapped::kYes) {
        SKGPU_LOG_W("Graphite does not auto-generate mipmap levels");
        return {};
    }

    SkColorType ct = gi->colorType();
    return {gi->textureProxyView(), ct};
}

} // namespace skgpu::graphite

namespace skif {

Context MakeGraphiteContext(skgpu::graphite::Recorder* recorder,
                            const ContextInfo& info) {
    SkASSERT(recorder);
    SkASSERT(!info.fSource.image() || info.fSource.image()->isGraphiteBacked());

    auto makeSurfaceFunctor = [recorder](const SkImageInfo& imageInfo,
                                         const SkSurfaceProps* props) {
        return SkSpecialSurfaces::MakeGraphite(recorder, imageInfo, *props);
    };
    auto makeImageCallback = [recorder](const SkIRect& subset,
                                sk_sp<SkImage> image,
                                const SkSurfaceProps& props) {
        // This just makes a raster image, but it could maybe call MakeFromGraphite
        return SkSpecialImages::MakeGraphite(recorder, subset, image, props);
    };
    auto makeCachedBitmapCallback = [recorder](const SkBitmap& data) -> sk_sp<SkImage> {
        auto proxy = skgpu::graphite::RecorderPriv::CreateCachedProxy(recorder, data);
        if (!proxy) {
            return nullptr;
        }

        const SkColorInfo& colorInfo = data.info().colorInfo();
        skgpu::Swizzle swizzle = recorder->priv().caps()->getReadSwizzle(colorInfo.colorType(),
                                                                         proxy->textureInfo());
        return sk_make_sp<skgpu::graphite::Image>(
                data.getGenerationID(),
                skgpu::graphite::TextureProxyView(std::move(proxy), swizzle),
                colorInfo);
    };

    return Context(info, nullptr, makeSurfaceFunctor, makeImageCallback, makeCachedBitmapCallback);
}
}  // namespace skif
