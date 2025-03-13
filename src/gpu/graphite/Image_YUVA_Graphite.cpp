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
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "src/core/SkYUVAInfoLocation.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/shaders/SkImageShader.h"


namespace skgpu::graphite {

namespace {

constexpr auto kAssumedColorType = kRGBA_8888_SkColorType;

static constexpr int kY = static_cast<int>(SkYUVAInfo::kY);
static constexpr int kU = static_cast<int>(SkYUVAInfo::kU);
static constexpr int kV = static_cast<int>(SkYUVAInfo::kV);
static constexpr int kA = static_cast<int>(SkYUVAInfo::kA);

static SkAlphaType yuva_alpha_type(const SkYUVAInfo& yuvaInfo) {
    // If an alpha channel is present we always use kPremul. This is because, although the planar
    // data is always un-premul and the final interleaved RGBA sample produced in the shader is
    // unpremul (and similar if flattened), the client is expecting premul.
    return yuvaInfo.hasAlpha() ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
}

} // anonymous

Image_YUVA::Image_YUVA(const YUVAProxies& proxies,
                       const SkYUVAInfo& yuvaInfo,
                       sk_sp<SkColorSpace> imageColorSpace)
        : Image_Base(SkImageInfo::Make(yuvaInfo.dimensions(),
                                       kAssumedColorType,
                                       yuva_alpha_type(yuvaInfo),
                                       std::move(imageColorSpace)),
                     kNeedNewImageUniqueID)
        , fProxies(std::move(proxies))
        , fYUVAInfo(yuvaInfo)
        , fUVSubsampleFactors(SkYUVAInfo::SubsamplingFactors(yuvaInfo.subsampling())) {
    // The caller should have checked this, just verifying.
    SkASSERT(fYUVAInfo.isValid());
    for (int i = 0; i < SkYUVAInfo::kYUVAChannelCount; ++i) {
        if (!fProxies[i]) {
            SkASSERT(i == kA);
            continue;
        }
        if (fProxies[i].proxy()->mipmapped() == Mipmapped::kNo) {
            fMipmapped = Mipmapped::kNo;
        }
        if (fProxies[i].proxy()->isProtected() == Protected::kYes) {
            fProtected = Protected::kYes;
        }
    }
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

    // Invoke the PlaneProxyFactoryFn for each plane and validate it against the plane config
    const int numPlanes = yuvaInfo.numPlanes();
    SkISize planeDimensions[SkYUVAInfo::kMaxPlanes];
    if (numPlanes != yuvaInfo.planeDimensions(planeDimensions)) {
        return nullptr;
    }
    uint32_t pixmapChannelmasks[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < numPlanes; ++i) {
        if (!planes[i] || !caps->isTexturable(planes[i].proxy()->textureInfo())) {
            return nullptr;
        }
        if (planes[i].dimensions() != planeDimensions[i]) {
            return nullptr;
        }
        pixmapChannelmasks[i] = TextureInfoPriv::ChannelMask(planes[i].proxy()->textureInfo());
    }

    // Re-arrange the proxies from planes to channels
    SkYUVAInfo::YUVALocations locations = yuvaInfo.toYUVALocations(pixmapChannelmasks);
    int expectedPlanes;
    if (!SkYUVAInfo::YUVALocation::AreValidLocations(locations, &expectedPlanes) ||
        expectedPlanes != numPlanes) {
        return nullptr;
    }
    // Y channel should match the YUVAInfo dimensions
    if (planes[locations[kY].fPlane].dimensions() != yuvaInfo.dimensions()) {
        return nullptr;
    }
    // UV channels should have planes with the same dimensions and subsampling factor.
    if (planes[locations[kU].fPlane].dimensions() != planes[locations[kV].fPlane].dimensions()) {
        return nullptr;
    }
    // If A channel is present, it should match the Y channel
    if (locations[kA].fPlane >= 0 &&
        planes[locations[kA].fPlane].dimensions() != yuvaInfo.dimensions()) {
        return nullptr;
    }

    if (yuvaInfo.planeSubsamplingFactors(locations[kU].fPlane) !=
        yuvaInfo.planeSubsamplingFactors(locations[kV].fPlane)) {
        return nullptr;
    }

    // Re-arrange into YUVA channel order and apply the location to the swizzle
    YUVAProxies channelProxies;
    for (int i = 0; i < SkYUVAInfo::kYUVAChannelCount; ++i) {
        auto [plane, channel] = locations[i];
        if (plane >= 0) {
            // Compose the YUVA location with the data swizzle. replaceSwizzle() is used since
            // selectChannelInR() effectively does the composition (vs. Swizzle::Concat).
            Swizzle channelSwizzle = planes[plane].swizzle().selectChannelInR((int) channel);
            channelProxies[i] = planes[plane].replaceSwizzle(channelSwizzle);
        } else if (i == kA) {
            // The alpha channel is allowed to be not provided, set it to an empty view
            channelProxies[i] = {};
        } else {
            SKGPU_LOG_W("YUVA channel %d does not have a valid location", i);
            return nullptr;
        }
    }

    return sk_sp<Image_YUVA>(new Image_YUVA(std::move(channelProxies),
                                            yuvaInfo,
                                            std::move(imageColorSpace)));
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
        // The YUVA shader expects to sample from the red channel for single-channel textures, so
        // reset the swizzle for alpha-only textures to compensate for that
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
    // We could look at the plane config and plane count to determine how many different textures
    // to expect, but it's theoretically possible for an Image_YUVA to be constructed where the
    // same TextureProxy is aliased to both the U and the V planes (and similarly for the Y and A)
    // even when the plane config specifies that those channels are not packed into the same texture
    //
    // Given that it's simpler to just sum the total gpu memory of non-duplicate textures.
    size_t size = 0;
    for (int i = 0; i < SkYUVAInfo::kYUVAChannelCount; ++i) {
        if (!fProxies[i]) {
            continue; // Null channels (A) have no size.
        }
        bool repeat = false;
        for (int j = i - 1; j >= 0; --j) {
            if (fProxies[i].proxy() == fProxies[j].proxy()) {
                repeat = true;
                break;
            }
        }
        if (!repeat) {
            if (fProxies[i].proxy()->isInstantiated()) {
                size += fProxies[i].proxy()->texture()->gpuMemorySize();
            } else {
                size += fProxies[i].proxy()->uninstantiatedGpuMemorySize();
            }
        }
    }

    return size;
}

sk_sp<SkImage> Image_YUVA::onReinterpretColorSpace(sk_sp<SkColorSpace> newCS) const {
    sk_sp<Image_YUVA> view{new Image_YUVA(fProxies,
                                          fYUVAInfo,
                                          std::move(newCS))};
    // The new Image object shares the same texture planes, so it should also share linked Devices
    view->linkDevices(this);
    return view;
}

}  // namespace skgpu::graphite
