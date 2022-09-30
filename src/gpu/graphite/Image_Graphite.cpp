/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/Image_Graphite.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureUtils.h"

#if SK_SUPPORT_GPU
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#endif

namespace skgpu::graphite {

Image::Image(uint32_t uniqueID,
             TextureProxyView view,
             const SkColorInfo& info)
    : SkImage_Base(SkImageInfo::Make(view.proxy()->dimensions(), info), uniqueID)
    , fTextureProxyView(std::move(view)) {
}

Image::Image(TextureProxyView view,
             const SkColorInfo& info)
    : SkImage_Base(SkImageInfo::Make(view.proxy()->dimensions(), info), kNeedNewImageUniqueID)
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

sk_sp<SkImage> Image::onMakeColorTypeAndColorSpace(SkColorType,
                                                   sk_sp<SkColorSpace>,
                                                   GrDirectContext*) const {
    return nullptr;
}

sk_sp<SkImage> Image::onReinterpretColorSpace(sk_sp<SkColorSpace>) const {
    return nullptr;
}

void Image::onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                        SkIRect srcRect,
                                        RescaleGamma rescaleGamma,
                                        RescaleMode rescaleMode,
                                        ReadPixelsCallback callback,
                                        ReadPixelsContext context) const {
    // TODO
    callback(context, nullptr);
}

void Image::onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                              sk_sp<SkColorSpace> dstColorSpace,
                                              const SkIRect srcRect,
                                              const SkISize dstSize,
                                              RescaleGamma rescaleGamma,
                                              RescaleMode rescaleMode,
                                              ReadPixelsCallback callback,
                                              ReadPixelsContext context) const {
    // TODO
    callback(context, nullptr);
}

#if SK_SUPPORT_GPU
std::unique_ptr<GrFragmentProcessor> Image::onAsFragmentProcessor(
        GrRecordingContext*,
        SkSamplingOptions,
        const SkTileMode[2],
        const SkMatrix&,
        const SkRect* subset,
        const SkRect* domain) const {
    return nullptr;
}
#endif

sk_sp<SkImage> Image::onMakeTextureImage(Recorder*, RequiredImageProperties requiredProps) const {
    SkASSERT(requiredProps.fMipmapped == Mipmapped::kYes && !this->hasMipmaps());
    // TODO: copy the base layer into a new image that has mip levels. For now we just return
    // the un-mipmapped version and allow the sampling to be downgraded to linear
    SKGPU_LOG_W("Graphite does not yet allow explicit mipmap level addition");
    return sk_ref_sp(this);
}

} // namespace skgpu::graphite

using namespace skgpu::graphite;

namespace {

bool validate_backend_texture(const Caps* caps,
                              const BackendTexture& texture,
                              const SkColorInfo& info) {
    if (!texture.isValid() ||
        texture.dimensions().width() <= 0 ||
        texture.dimensions().height() <= 0) {
        return false;
    }

    if (!SkColorInfoIsValid(info)) {
        return false;
    }

    if (!caps->isTexturable(texture.info())) {
        return false;
    }

    return caps->areColorTypeAndTextureInfoCompatible(info.colorType(), texture.info());
}

} // anonymous namespace

sk_sp<SkImage> SkImage::makeTextureImage(Recorder* recorder,
                                         RequiredImageProperties requiredProps) const {
    if (!recorder) {
        return nullptr;
    }
    if (this->dimensions().area() <= 1) {
        requiredProps.fMipmapped = Mipmapped::kNo;
    }

    if (as_IB(this)->isGraphiteBacked()) {
        if (requiredProps.fMipmapped == Mipmapped::kNo || this->hasMipmaps()) {
            const SkImage* image = this;
            return sk_ref_sp(const_cast<SkImage*>(image));
        }
    }
    return as_IB(this)->onMakeTextureImage(recorder, requiredProps);
}

sk_sp<SkImage> SkImage::MakeGraphiteFromBackendTexture(Recorder* recorder,
                                                       const BackendTexture& backendTex,
                                                       SkColorType ct,
                                                       SkAlphaType at,
                                                       sk_sp<SkColorSpace> cs) {
    if (!recorder) {
        return nullptr;
    }

    const Caps* caps = recorder->priv().caps();

    SkColorInfo info(ct, at, std::move(cs));

    if (!validate_backend_texture(caps, backendTex, info)) {
        return nullptr;
    }

    sk_sp<Texture> texture = recorder->priv().resourceProvider()->createWrappedTexture(backendTex);
    if (!texture) {
        return nullptr;
    }

    sk_sp<TextureProxy> proxy(new TextureProxy(std::move(texture)));

    skgpu::Swizzle swizzle = caps->getReadSwizzle(ct, backendTex.info());
    TextureProxyView view(std::move(proxy), swizzle);
    return sk_make_sp<Image>(view, info);
}
