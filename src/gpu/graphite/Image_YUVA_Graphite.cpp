/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_YUVA_Graphite.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureUtils.h"

namespace {
constexpr auto kAssumedColorType = kRGBA_8888_SkColorType;
}

namespace skgpu::graphite {

Image_YUVA::Image_YUVA(uint32_t uniqueID,
                       YUVATextureProxies proxies,
                       sk_sp<SkColorSpace> imageColorSpace)
        : Image_Base(SkImageInfo::Make(proxies.yuvaInfo().dimensions(),
                                       kAssumedColorType,
                                       // If an alpha channel is present we always use kPremul. This
                                       // is because, although the planar data is always un-premul,
                                       // the final interleaved RGBA sample produced in the shader
                                       // is premul (and similar if flattened).
                                       proxies.yuvaInfo().hasAlpha() ? kPremul_SkAlphaType
                                                                     : kOpaque_SkAlphaType,
                                       std::move(imageColorSpace)),
                     uniqueID)
        , fYUVAProxies(std::move(proxies)) {
    // The caller should have checked this, just verifying.
    SkASSERT(fYUVAProxies.isValid());
}

}  // namespace skgpu::graphite

using namespace skgpu::graphite;

//////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImage> SkImage::MakeGraphiteFromYUVAPixmaps(Recorder* recorder,
                                                    const SkYUVAPixmaps& pixmaps,
                                                    RequiredImageProperties required,
                                                    bool limitToMaxTextureSize,
                                                    sk_sp<SkColorSpace> imageColorSpace) {
    if (!recorder) {
        return nullptr;  // until we impl this for raster backend
    }

    if (!pixmaps.isValid()) {
        return nullptr;
    }

    // Resize the pixmaps if necessary.
    int numPlanes = pixmaps.numPlanes();
    int maxTextureSize = recorder->priv().caps()->maxTextureSize();
    int maxDim = std::max(pixmaps.yuvaInfo().width(), pixmaps.yuvaInfo().height());

    SkYUVAPixmaps tempPixmaps;
    const SkYUVAPixmaps* pixmapsToUpload = &pixmaps;
    // We assume no plane is larger than the image size (and at least one plane is as big).
    if (maxDim > maxTextureSize) {
        if (!limitToMaxTextureSize) {
            return nullptr;
        }
        float scale = static_cast<float>(maxTextureSize)/maxDim;
        SkISize newDimensions = {
            std::min(static_cast<int>(pixmaps.yuvaInfo().width() *scale), maxTextureSize),
            std::min(static_cast<int>(pixmaps.yuvaInfo().height()*scale), maxTextureSize)
        };
        SkYUVAInfo newInfo = pixmaps.yuvaInfo().makeDimensions(newDimensions);
        SkYUVAPixmapInfo newPixmapInfo(newInfo, pixmaps.dataType(), /*rowBytes=*/nullptr);
        tempPixmaps = SkYUVAPixmaps::Allocate(newPixmapInfo);
        if (!tempPixmaps.isValid()) {
            return nullptr;
        }
        SkSamplingOptions sampling(SkFilterMode::kLinear);
        for (int i = 0; i < numPlanes; ++i) {
            if (!pixmaps.plane(i).scalePixels(tempPixmaps.plane(i), sampling)) {
                return nullptr;
            }
        }
        pixmapsToUpload = &tempPixmaps;
    }

    // Convert to texture proxies.
    TextureProxyView views[SkYUVAInfo::kMaxPlanes];
    SkColorType pixmapColorTypes[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < numPlanes; ++i) {
        // Turn the pixmap into a TextureProxy
        SkBitmap bmp;
        bmp.installPixels(pixmapsToUpload->plane(i));
        std::tie(views[i], std::ignore) = MakeBitmapProxyView(recorder,
                                                              bmp,
                                                              /*mipmapsIn=*/nullptr,
                                                              required.fMipmapped,
                                                              skgpu::Budgeted::kNo);
        if (!views[i]) {
            return nullptr;
        }
        pixmapColorTypes[i] = bmp.colorType();
    }

    YUVATextureProxies yuvaProxies(pixmapsToUpload->yuvaInfo(), views, pixmapColorTypes);
    SkASSERT(yuvaProxies.isValid());
    return sk_make_sp<Image_YUVA>(kNeedNewImageUniqueID,
                                  std::move(yuvaProxies),
                                  std::move(imageColorSpace));
}


