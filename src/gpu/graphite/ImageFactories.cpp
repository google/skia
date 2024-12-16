/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkSurface.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Surface.h"
#include "include/gpu/graphite/YUVABackendTextures.h"
#include "include/private/base/SkMutex.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/Image_Base_Graphite.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Image_YUVA_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/TextureProxyView.h"
#include "src/gpu/graphite/TextureUtils.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Lazy.h"
#include "src/image/SkImage_Picture.h"
#include "src/image/SkImage_Raster.h"

#include <string>

namespace SkImages {

using namespace skgpu::graphite;

static bool validate_backend_texture(const skgpu::graphite::Caps* caps,
                                     const skgpu::graphite::BackendTexture& texture,
                                     const SkColorInfo& info) {
    if (!texture.isValid() || texture.dimensions().width() <= 0 ||
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

sk_sp<SkImage> WrapTexture(Recorder* recorder,
                           const BackendTexture& backendTex,
                           SkColorType ct,
                           SkAlphaType at,
                           sk_sp<SkColorSpace> cs,
                           skgpu::Origin origin,
                           GenerateMipmapsFromBase genMipmaps,
                           TextureReleaseProc releaseP,
                           ReleaseContext releaseC,
                           std::string_view label) {
    auto releaseHelper = skgpu::RefCntedCallback::Make(releaseP, releaseC);

    if (!recorder) {
        return nullptr;
    }

    const Caps* caps = recorder->priv().caps();

    SkColorInfo info(ct, at, std::move(cs));

    if (!validate_backend_texture(caps, backendTex, info)) {
        return nullptr;
    }

    if (label.empty()) {
        label = "WrappedImage";
    }

    sk_sp<Texture> texture =
            recorder->priv().resourceProvider()->createWrappedTexture(backendTex, std::move(label));
    if (!texture) {
        SKGPU_LOG_W("Texture creation failed");
        return nullptr;
    }
    texture->setReleaseCallback(std::move(releaseHelper));

    sk_sp<TextureProxy> proxy = TextureProxy::Wrap(std::move(texture));
    SkASSERT(proxy);

    skgpu::Swizzle swizzle = caps->getReadSwizzle(ct, backendTex.info());
    TextureProxyView view(std::move(proxy), swizzle, origin);

    if (genMipmaps == GenerateMipmapsFromBase::kYes) {
        if (view.proxy()->mipmapped() == skgpu::Mipmapped::kNo) {
            SKGPU_LOG_W("Failed SkImage:::WrapTexture because asked to generate mipmaps for "
                        "nonmipmapped texture");
            return nullptr;
        }
        if (!GenerateMipmaps(recorder, view.refProxy(), info)) {
            SKGPU_LOG_W("Failed SkImage::WrapTexture. Could not generate mipmaps.");
            return nullptr;
        }
    }

    return sk_make_sp<skgpu::graphite::Image>(view, info);
}

sk_sp<SkImage> WrapTexture(Recorder* recorder,
                           const BackendTexture& backendTex,
                           SkColorType ct,
                           SkAlphaType at,
                           sk_sp<SkColorSpace> cs,
                           skgpu::Origin origin,
                           TextureReleaseProc releaseP,
                           ReleaseContext releaseC,
                           std::string_view label) {
    return WrapTexture(recorder,
                       backendTex,
                       ct,
                       at,
                       std::move(cs),
                       origin,
                       SkImages::GenerateMipmapsFromBase::kNo,
                       releaseP,
                       releaseC,
                       std::move(label));
}

sk_sp<SkImage> WrapTexture(Recorder* recorder,
                           const BackendTexture& backendTex,
                           SkColorType ct,
                           SkAlphaType at,
                           sk_sp<SkColorSpace> cs,
                           TextureReleaseProc releaseP,
                           ReleaseContext releaseC,
                           std::string_view label) {
    return WrapTexture(recorder,
                       backendTex,
                       ct,
                       at,
                       std::move(cs),
                       skgpu::Origin::kTopLeft,
                       SkImages::GenerateMipmapsFromBase::kNo,
                       releaseP,
                       releaseC,
                       std::move(label));
}

sk_sp<SkImage> PromiseTextureFrom(Recorder* recorder,
                                  SkISize dimensions,
                                  const TextureInfo& textureInfo,
                                  const SkColorInfo& colorInfo,
                                  skgpu::Origin origin,
                                  Volatile isVolatile,
                                  GraphitePromiseTextureFulfillProc fulfillProc,
                                  GraphitePromiseImageReleaseProc imageReleaseProc,
                                  GraphitePromiseTextureReleaseProc textureReleaseProc,
                                  GraphitePromiseImageContext imageContext,
                                  std::string_view label) {
    // Our contract is that we will always call the _image_ release proc even on failure.
    // We use the helper to convey the imageContext, so we need to ensure Make doesn't fail.
    imageReleaseProc = imageReleaseProc ? imageReleaseProc : [](void*) {};
    auto releaseHelper = skgpu::RefCntedCallback::Make(imageReleaseProc, imageContext);

    if (!recorder) {
        SKGPU_LOG_W("Null Recorder");
        return nullptr;
    }

    const Caps* caps = recorder->priv().caps();

    SkImageInfo info = SkImageInfo::Make(dimensions, colorInfo);
    if (!SkImageInfoIsValid(info)) {
        SKGPU_LOG_W("Invalid SkImageInfo");
        return nullptr;
    }

    if (!caps->areColorTypeAndTextureInfoCompatible(colorInfo.colorType(), textureInfo)) {
        SKGPU_LOG_W("Incompatible SkColorType and TextureInfo");
        return nullptr;
    }

    // Non-YUVA promise images use the 'imageContext' for both the release proc and fulfill proc.
    sk_sp<TextureProxy> proxy = MakePromiseImageLazyProxy(caps,
                                                          dimensions,
                                                          textureInfo,
                                                          isVolatile,
                                                          std::move(releaseHelper),
                                                          fulfillProc,
                                                          imageContext,
                                                          textureReleaseProc,
                                                          std::move(label));
    if (!proxy) {
        return nullptr;
    }

    skgpu::Swizzle swizzle = caps->getReadSwizzle(colorInfo.colorType(), textureInfo);
    TextureProxyView view(std::move(proxy), swizzle, origin);
    return sk_make_sp<Image>(view, colorInfo);
}

sk_sp<SkImage> PromiseTextureFrom(Recorder* recorder,
                                  SkISize dimensions,
                                  const TextureInfo& textureInfo,
                                  const SkColorInfo& colorInfo,
                                  Volatile isVolatile,
                                  GraphitePromiseTextureFulfillProc fulfillProc,
                                  GraphitePromiseImageReleaseProc imageReleaseProc,
                                  GraphitePromiseTextureReleaseProc textureReleaseProc,
                                  GraphitePromiseImageContext imageContext) {
    return PromiseTextureFrom(recorder,
                              dimensions,
                              textureInfo,
                              colorInfo,
                              skgpu::Origin::kTopLeft,
                              isVolatile,
                              fulfillProc,
                              imageReleaseProc,
                              textureReleaseProc,
                              imageContext);
}

sk_sp<SkImage> PromiseTextureFromYUVA(skgpu::graphite::Recorder* recorder,
                                      const YUVABackendTextureInfo& backendTextureInfo,
                                      sk_sp<SkColorSpace> imageColorSpace,
                                      skgpu::graphite::Volatile isVolatile,
                                      GraphitePromiseTextureFulfillProc fulfillProc,
                                      GraphitePromiseImageReleaseProc imageReleaseProc,
                                      GraphitePromiseTextureReleaseProc textureReleaseProc,
                                      GraphitePromiseImageContext imageContext,
                                      GraphitePromiseTextureFulfillContext planeContexts[],
                                      std::string_view label) {
    // Our contract is that we will always call the _image_ release proc even on failure.
    // We use the helper to convey the imageContext, so we need to ensure Make doesn't fail.
    auto releaseHelper = skgpu::RefCntedCallback::Make(imageReleaseProc, imageContext);
    if (!recorder) {
        return nullptr;
    }
    // Precompute the dimensions for all promise texture planes
    SkISize planeDimensions[SkYUVAInfo::kMaxPlanes];
    if (!backendTextureInfo.yuvaInfo().planeDimensions(planeDimensions)) {
        return nullptr;
    }

    std::string labelStr(label);
    if (labelStr.empty()) {
        labelStr = "Wrapped_PromiseYUVPlane";
    } else {
        labelStr += "_PromiseYUVPlane";
    }

    TextureProxyView planes[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < backendTextureInfo.numPlanes(); ++i) {
        sk_sp<TextureProxy> lazyProxy = MakePromiseImageLazyProxy(
                recorder->priv().caps(),
                planeDimensions[i],
                backendTextureInfo.planeTextureInfo(i),
                isVolatile,
                releaseHelper,
                fulfillProc,
                planeContexts[i],
                textureReleaseProc,
                labelStr);
        // Promise YUVA images assume the default rgba swizzle.
        planes[i] = TextureProxyView(std::move(lazyProxy));
    }
    return Image_YUVA::Make(recorder->priv().caps(), backendTextureInfo.yuvaInfo(),
                            SkSpan(planes), std::move(imageColorSpace));
}

sk_sp<SkImage> SubsetTextureFrom(skgpu::graphite::Recorder* recorder,
                                 const SkImage* img,
                                 const SkIRect& subset,
                                 SkImage::RequiredProperties props) {
    if (!recorder || !img) {
        return nullptr;
    }
    auto subsetImg = img->makeSubset(recorder, subset, props);
    return SkImages::TextureFromImage(recorder, subsetImg, props);
}

sk_sp<SkImage> MakeWithFilter(skgpu::graphite::Recorder* recorder,
                              sk_sp<SkImage> src,
                              const SkImageFilter* filter,
                              const SkIRect& subset,
                              const SkIRect& clipBounds,
                              SkIRect* outSubset,
                              SkIPoint* offset) {
    if (!recorder || !src || !filter) {
        return nullptr;
    }

    sk_sp<skif::Backend> backend = skif::MakeGraphiteBackend(recorder, {}, src->colorType());
    return as_IFB(filter)->makeImageWithFilter(std::move(backend),
                                               std::move(src),
                                               subset,
                                               clipBounds,
                                               outSubset,
                                               offset);
}

static sk_sp<SkImage> generate_picture_texture(skgpu::graphite::Recorder* recorder,
                                               const SkImage_Picture* img,
                                               const SkImageInfo& info,
                                               SkImage::RequiredProperties requiredProps) {
    auto mm = requiredProps.fMipmapped ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    // Use a non-budgeted surface since the image wrapping the surface's texture will be owned by
    // the client.
    sk_sp<Surface> surface = Surface::Make(recorder,
                                           info,
                                           "LazySkImagePictureTexture",
                                           skgpu::Budgeted::kNo,
                                           mm,
                                           SkBackingFit::kExact,
                                           img->props());

    if (!surface) {
        SKGPU_LOG_E("Failed to create Surface");
        return nullptr;
    }

    img->replay(surface->getCanvas());
    // If the surface was created with mipmaps, they will be automatically generated when flushing
    // the tasks when 'surface' goes out of scope.
    return surface->asImage();
}

/*
 *  We only have 2 ways to create a Graphite-backed image.
 *
 *  1. Ask the generator to natively create one
 *  2. Ask the generator to return RGB(A) data, which the GPU can convert
 */
static sk_sp<SkImage> make_texture_image_from_lazy(skgpu::graphite::Recorder* recorder,
                                                   const SkImage_Lazy* img,
                                                   SkImage::RequiredProperties requiredProps) {
    // 1. Ask the generator to natively create one.
    {
        if (img->type() == SkImage_Base::Type::kLazyPicture) {
            sk_sp<SkImage> newImage =
                    generate_picture_texture(recorder,
                                             static_cast<const SkImage_Picture*>(img),
                                             img->imageInfo(),
                                             requiredProps);
            if (newImage) {
                SkASSERT(as_IB(newImage)->isGraphiteBacked());
                return newImage;
            }
            // The fallback for this would be to generate a bitmap, but some picture-backed
            // images can only be played back on the GPU.
            return nullptr;
        }
        // There is not an analog to GrTextureGenerator for Graphite yet, but if there was,
        // we would want to call it here.
    }

    // 2. Ask the generator to return a bitmap, which the GPU can convert.
    {
        SkBitmap bitmap;
        if (img->getROPixels(nullptr, &bitmap, SkImage_Lazy::CachingHint::kDisallow_CachingHint)) {
            return skgpu::graphite::MakeFromBitmap(recorder,
                                                   img->imageInfo().colorInfo(),
                                                   bitmap,
                                                   nullptr,
                                                   skgpu::Budgeted::kNo,
                                                   requiredProps,
                                                   "LazySkImageBitmapTexture");
        }
    }

    return nullptr;
}

sk_sp<SkImage> TextureFromImage(skgpu::graphite::Recorder* recorder,
                                const SkImage* image,
                                SkImage::RequiredProperties requiredProps) {
    if (!recorder || !image) {
        return nullptr;
    }
    if (image->dimensions().area() <= 1) {
        requiredProps.fMipmapped = false;
    }

    auto ib = as_IB(image);
    SkASSERT(!ib->isGaneshBacked());

    if (ib->isRasterBacked()) {
        auto raster = static_cast<const SkImage_Raster*>(ib);
        return skgpu::graphite::MakeFromBitmap(recorder,
                                               raster->imageInfo().colorInfo(),
                                               raster->bitmap(),
                                               raster->refMips(),
                                               skgpu::Budgeted::kNo,
                                               requiredProps,
                                               "RasterBitmapTexture");
    }
    if (ib->isLazyGenerated()) {
        return make_texture_image_from_lazy(
                recorder, static_cast<const SkImage_Lazy*>(ib), requiredProps);
    }
    SkASSERT(ib->isGraphiteBacked());
    return ib->makeSubset(recorder, ib->bounds(), requiredProps);
}

sk_sp<SkImage> TextureFromYUVAPixmaps(Recorder* recorder,
                                      const SkYUVAPixmaps& pixmaps,
                                      SkImage::RequiredProperties requiredProps,
                                      bool limitToMaxTextureSize,
                                      sk_sp<SkColorSpace> imageColorSpace,
                                      std::string_view label) {
    if (!recorder) {
        return nullptr;
    }

    // Determine if we have to resize the pixmaps
    const int maxTextureSize = recorder->priv().caps()->maxTextureSize();
    const int maxDim = std::max(pixmaps.yuvaInfo().width(), pixmaps.yuvaInfo().height());

    SkYUVAPixmapInfo finalInfo = pixmaps.pixmapsInfo();
    if (maxDim > maxTextureSize) {
        if (!limitToMaxTextureSize) {
            return nullptr;
        }
        float scale = static_cast<float>(maxTextureSize) / maxDim;
        SkISize newDimensions = {
                std::min(static_cast<int>(pixmaps.yuvaInfo().width() * scale), maxTextureSize),
                std::min(static_cast<int>(pixmaps.yuvaInfo().height() * scale), maxTextureSize)};
        finalInfo = SkYUVAPixmapInfo(pixmaps.yuvaInfo().makeDimensions(newDimensions),
                                     pixmaps.dataType(),
                                     /*rowBytes=*/nullptr);
    }

    std::string labelStr(label);
    if (labelStr.empty()) {
        labelStr = "YUVRasterBitmapPlane";
    } else {
        labelStr += "_YUVBitmapPlane";
    }

    auto mipmapped = requiredProps.fMipmapped ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    TextureProxyView planes[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < finalInfo.yuvaInfo().numPlanes(); ++i) {
        SkBitmap bmp;
        if (maxDim > maxTextureSize) {
            // Rescale the data before uploading
            if (!bmp.tryAllocPixels(finalInfo.planeInfo(i)) ||
                !pixmaps.plane(i).scalePixels(bmp.pixmap(), SkFilterMode::kLinear)) {
                return nullptr;
            }
        } else {
            // Use original data to upload
            if (!bmp.installPixels(pixmaps.plane(i))) {
                return nullptr;
            }
        }

        auto [view, _] = MakeBitmapProxyView(recorder, bmp, /*mipmapsIn=*/nullptr,
                                             mipmapped,  skgpu::Budgeted::kNo,
                                             labelStr);
        planes[i] = std::move(view);
    }
    return Image_YUVA::Make(recorder->priv().caps(), finalInfo.yuvaInfo(),
                            SkSpan(planes), std::move(imageColorSpace));
}

sk_sp<SkImage> TextureFromYUVATextures(Recorder* recorder,
                                       const YUVABackendTextures& yuvaTextures,
                                       sk_sp<SkColorSpace> imageColorSpace,
                                       TextureReleaseProc releaseP,
                                       ReleaseContext releaseC,
                                       std::string_view label) {
    auto releaseHelper = skgpu::RefCntedCallback::Make(releaseP, releaseC);
    if (!recorder) {
        return nullptr;
    }

    std::string labelStr(label);
    if (labelStr.empty()) {
        labelStr = "Wrapped_YUVPlane";
    } else {
        labelStr += "_YUVPlane";
    }

    TextureProxyView planes[SkYUVAInfo::kMaxPlanes];
    for (int i = 0; i < yuvaTextures.yuvaInfo().numPlanes(); ++i) {
        sk_sp<Texture> texture = recorder->priv().resourceProvider()->createWrappedTexture(
                yuvaTextures.planeTexture(i), labelStr);
        if (!texture) {
            SKGPU_LOG_W("Failed to wrap backend texture for YUVA plane %d", i);
            return nullptr;
        }
        texture->setReleaseCallback(releaseHelper);
        planes[i] = TextureProxyView(TextureProxy::Wrap(std::move(texture)));
    }

    return Image_YUVA::Make(recorder->priv().caps(), yuvaTextures.yuvaInfo(),
                            SkSpan(planes), std::move(imageColorSpace));
}

sk_sp<SkImage> TextureFromYUVAImages(Recorder* recorder,
                                     const SkYUVAInfo& yuvaInfo,
                                     SkSpan<const sk_sp<SkImage>> images,
                                     sk_sp<SkColorSpace> imageColorSpace) {
    // This factory is just a view of the images, so does not actually trigger any work on the
    // recorder. It is just used to provide the Caps.
    return Image_YUVA::WrapImages(recorder->priv().caps(), yuvaInfo, images, imageColorSpace);
}

}  // namespace SkImages
