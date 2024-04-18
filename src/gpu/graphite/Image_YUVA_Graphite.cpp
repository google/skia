/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_YUVA_Graphite.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/shaders/SkImageShader.h"

namespace {
constexpr auto kAssumedColorType = kRGBA_8888_SkColorType;
}

namespace skgpu::graphite {

static SkAlphaType yuva_alpha_type(const SkYUVAInfo& yuvaInfo) {
    // If an alpha channel is present we always use kPremul. This is because, although the planar
    // data is always un-premul, the final interleaved RGBA sample produced in the shader is premul
    // (and similar if flattened).
    return yuvaInfo.hasAlpha() ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
}

Image_YUVA::Image_YUVA(YUVATextureProxies proxies,
                       sk_sp<SkColorSpace> imageColorSpace)
        : Image_Base(SkImageInfo::Make(proxies.yuvaInfo().dimensions(),
                                       kAssumedColorType,
                                       yuva_alpha_type(proxies.yuvaInfo()),
                                       std::move(imageColorSpace)),
                     kNeedNewImageUniqueID)
        , fYUVAProxies(std::move(proxies)) {
    // The caller should have checked this, just verifying.
    SkASSERT(fYUVAProxies.isValid());
}

Image_YUVA::~Image_YUVA() = default;

sk_sp<Image_YUVA> Image_YUVA::Make(const Caps* caps,
                                   const SkYUVAInfo& yuvaInfo,
                                   SkSpan<TextureProxyView> planes,
                                   sk_sp<SkColorSpace> imageColorSpace) {
    if (!yuvaInfo.isValid()) {
        return nullptr;
    }
    SkImageInfo info = SkImageInfo::Make(
            yuvaInfo.dimensions(), kAssumedColorType, yuva_alpha_type(yuvaInfo), imageColorSpace);
    if (!SkImageInfoIsValid(info)) {
        return nullptr;
    }

    YUVATextureProxies yuvaTextureProxies(caps, yuvaInfo, planes);
    if (!yuvaTextureProxies.isValid()) {
        return nullptr;
    }
    return sk_make_sp<Image_YUVA>(std::move(yuvaTextureProxies), std::move(imageColorSpace));
}

sk_sp<Image_YUVA> Image_YUVA::WrapImages(const Caps* caps,
                                         const SkYUVAInfo& yuvaInfo,
                                         SkSpan<const sk_sp<SkImage>> images,
                                         sk_sp<SkColorSpace> imageColorSpace) {
    if (SkTo<int>(images.size()) < yuvaInfo.numPlanes()) {
        return nullptr;
    }

    TextureProxyView planes[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < yuvaInfo.numPlanes(); ++i) {
        planes[i] = AsView(images[i]);
        if (!planes[i]) {
            // A null image, or not graphite-backed, or not backed by a single texture.
            return nullptr;
        }
        // YUVATextureProxies expects to sample from the red channel for single-channel
        // textures, so reset the swizzle for alpha-only textures to compensate for that
        if (images[i]->isAlphaOnly()) {
            planes[i] = planes[i].makeSwizzle(Swizzle("aaaa"));
        }
    }

    sk_sp<Image_YUVA> image = Make(caps, yuvaInfo, SkSpan(planes), std::move(imageColorSpace));
    if (image) {
        // Unlike the other factories, this YUVA image shares the texture proxies with each plane
        // Image, so if those are linked to Devices, it must inherit those same links.
        for (int plane = 0; plane < yuvaInfo.numPlanes(); ++plane) {
            SkASSERT(as_IB(images[plane])->isGraphiteBacked());
            image->linkDevices(static_cast<Image_Base*>(images[plane].get()));
        }
    }
    return image;
}

size_t Image_YUVA::textureSize() const {
    size_t size = 0;
    for (int i = 0; i < fYUVAProxies.numPlanes(); ++i) {
        if (fYUVAProxies.proxy(i)->texture()) {
            size += fYUVAProxies.proxy(i)->texture()->gpuMemorySize();
        }
    }
    return size;
}

sk_sp<SkImage> Image_YUVA::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    sk_sp<Image_YUVA> view = sk_make_sp<Image_YUVA>(fYUVAProxies, std::move(newCS));
    // The new Image object shares the same texture planes, so it should also share linked Devices
    view->linkDevices(this);
    return view;
}

}  // namespace skgpu::graphite
