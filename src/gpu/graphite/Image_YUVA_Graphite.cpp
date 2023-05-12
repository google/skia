/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_YUVA_Graphite.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/YUVABackendTextures.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
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

