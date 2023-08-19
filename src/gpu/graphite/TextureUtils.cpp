/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "src/core/SkMipmap.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SynchronizeToCpuTask.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/UploadTask.h"

namespace skgpu::graphite {

std::tuple<TextureProxyView, SkColorType> MakeBitmapProxyView(Recorder* recorder,
                                                              const SkBitmap& bitmap,
                                                              sk_sp<SkMipmap> mipmapsIn,
                                                              Mipmapped mipmapped,
                                                              skgpu::Budgeted budgeted) {
    // Adjust params based on input and Caps
    const skgpu::graphite::Caps* caps = recorder->priv().caps();
    SkColorType ct = bitmap.info().colorType();

    if (bitmap.dimensions().area() <= 1) {
        mipmapped = Mipmapped::kNo;
    }

    auto textureInfo = caps->getDefaultSampledTextureInfo(ct, mipmapped, Protected::kNo,
                                                          Renderable::kNo);
    if (!textureInfo.isValid()) {
        ct = kRGBA_8888_SkColorType;
        textureInfo = caps->getDefaultSampledTextureInfo(ct, mipmapped, Protected::kNo,
                                                         Renderable::kNo);
    }
    SkASSERT(textureInfo.isValid());

    // Convert bitmap to texture colortype if necessary
    SkBitmap bmpToUpload;
    if (ct != bitmap.info().colorType()) {
        if (!bmpToUpload.tryAllocPixels(bitmap.info().makeColorType(ct)) ||
            !bitmap.readPixels(bmpToUpload.pixmap())) {
            return {};
        }
        bmpToUpload.setImmutable();
    } else {
        bmpToUpload = bitmap;
    }

    if (!SkImageInfoIsValid(bmpToUpload.info())) {
        return {};
    }

    int mipLevelCount = (mipmapped == Mipmapped::kYes) ?
            SkMipmap::ComputeLevelCount(bitmap.width(), bitmap.height()) + 1 : 1;


    // setup MipLevels
    sk_sp<SkMipmap> mipmaps;
    std::vector<MipLevel> texels;
    if (mipLevelCount == 1) {
        texels.resize(mipLevelCount);
        texels[0].fPixels = bmpToUpload.getPixels();
        texels[0].fRowBytes = bmpToUpload.rowBytes();
    } else {
        mipmaps = SkToBool(mipmapsIn) ? mipmapsIn
                                      : sk_ref_sp(SkMipmap::Build(bmpToUpload.pixmap(), nullptr));
        if (!mipmaps) {
            return {};
        }

        SkASSERT(mipLevelCount == mipmaps->countLevels() + 1);
        texels.resize(mipLevelCount);

        texels[0].fPixels = bmpToUpload.getPixels();
        texels[0].fRowBytes = bmpToUpload.rowBytes();

        for (int i = 1; i < mipLevelCount; ++i) {
            SkMipmap::Level generatedMipLevel;
            mipmaps->getLevel(i - 1, &generatedMipLevel);
            texels[i].fPixels = generatedMipLevel.fPixmap.addr();
            texels[i].fRowBytes = generatedMipLevel.fPixmap.rowBytes();
            SkASSERT(texels[i].fPixels);
            SkASSERT(generatedMipLevel.fPixmap.colorType() == bmpToUpload.colorType());
        }
    }

    // Create proxy
    sk_sp<TextureProxy> proxy =
            TextureProxy::Make(caps, bmpToUpload.dimensions(), textureInfo, budgeted);
    if (!proxy) {
        return {};
    }
    SkASSERT(caps->areColorTypeAndTextureInfoCompatible(ct, proxy->textureInfo()));
    SkASSERT(mipmapped == Mipmapped::kNo || proxy->mipmapped() == Mipmapped::kYes);

    // Src and dst colorInfo are the same
    const SkColorInfo& colorInfo = bmpToUpload.info().colorInfo();
    // Add UploadTask to Recorder
    UploadInstance upload = UploadInstance::Make(
            recorder, proxy, colorInfo, colorInfo, texels,
            SkIRect::MakeSize(bmpToUpload.dimensions()), std::make_unique<ImageUploadContext>());
    if (!upload.isValid()) {
        SKGPU_LOG_E("MakeBitmapProxyView: Could not create UploadInstance");
        return {};
    }
    recorder->priv().add(UploadTask::Make(std::move(upload)));

    Swizzle swizzle = caps->getReadSwizzle(ct, textureInfo);
    // If the color type is alpha-only, propagate the alpha value to the other channels.
    if (SkColorTypeIsAlphaOnly(colorInfo.colorType())) {
        swizzle = Swizzle::Concat(swizzle, Swizzle("aaaa"));
    }
    return {{std::move(proxy), swizzle}, ct};
}

sk_sp<SkImage> MakeFromBitmap(Recorder* recorder,
                              const SkColorInfo& colorInfo,
                              const SkBitmap& bitmap,
                              sk_sp<SkMipmap> mipmaps,
                              skgpu::Budgeted budgeted,
                              SkImage::RequiredProperties requiredProps) {
    auto mm = requiredProps.fMipmapped ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;
    auto [view, ct] = MakeBitmapProxyView(recorder, bitmap, std::move(mipmaps), mm, budgeted);
    if (!view) {
        return nullptr;
    }

    SkASSERT(!requiredProps.fMipmapped || view.proxy()->mipmapped() == skgpu::Mipmapped::kYes);
    return sk_make_sp<skgpu::graphite::Image>(kNeedNewImageUniqueID,
                                              std::move(view),
                                              colorInfo.makeColorType(ct));
}


// TODO: Make this computed size more generic to handle compressed textures
size_t ComputeSize(SkISize dimensions,
                   const TextureInfo& info) {
    // TODO: Should we make sure the backends return zero here if the TextureInfo is for a
    // memoryless texture?
    size_t bytesPerPixel = info.bytesPerPixel();

    size_t colorSize = (size_t)dimensions.width() * dimensions.height() * bytesPerPixel;

    size_t finalSize = colorSize * info.numSamples();

    if (info.mipmapped() == Mipmapped::kYes) {
        finalSize += colorSize/3;
    }
    return finalSize;
}

sk_sp<SkSurface> make_surface_with_fallback(Recorder* recorder,
                                            const SkImageInfo& info,
                                            Mipmapped mipmapped,
                                            const SkSurfaceProps* surfaceProps) {
    SkColorType ct = recorder->priv().caps()->getRenderableColorType(info.colorType());
    if (ct == kUnknown_SkColorType) {
        return nullptr;
    }

    return SkSurfaces::RenderTarget(recorder, info.makeColorType(ct), mipmapped, surfaceProps);
}

sk_sp<SkImage> RescaleImage(Recorder* recorder,
                            const SkImage* srcImage,
                            SkIRect srcIRect,
                            const SkImageInfo& dstInfo,
                            SkImage::RescaleGamma rescaleGamma,
                            SkImage::RescaleMode rescaleMode) {
    // make a Surface matching dstInfo to rescale into
    SkSurfaceProps surfaceProps = {};
    sk_sp<SkSurface> dst = make_surface_with_fallback(recorder,
                                                      dstInfo,
                                                      Mipmapped::kNo,
                                                      &surfaceProps);
    if (!dst) {
        return nullptr;
    }

    SkRect srcRect = SkRect::Make(srcIRect);
    SkRect dstRect = SkRect::Make(dstInfo.dimensions());

    // Get backing texture information for source Image.
    // For now this needs to be texturable because we can't depend on copies to scale.
    auto srcGraphiteImage = reinterpret_cast<const skgpu::graphite::Image*>(srcImage);

    TextureProxyView imageView = srcGraphiteImage->textureProxyView();
    if (!imageView.proxy()) {
        // TODO: if not texturable, copy to a texturable format
        return nullptr;
    }

    SkISize finalSize = SkISize::Make(dstRect.width(), dstRect.height());
    if (finalSize == srcIRect.size()) {
        rescaleGamma = Image::RescaleGamma::kSrc;
        rescaleMode = Image::RescaleMode::kNearest;
    }

    // Within a rescaling pass tempInput is read from and tempOutput is written to.
    // At the end of the pass tempOutput's texture is wrapped and assigned to tempInput.
    const SkImageInfo& srcImageInfo = srcImage->imageInfo();
    sk_sp<SkImage> tempInput(new Image(kNeedNewImageUniqueID,
                                       imageView,
                                       srcImageInfo.colorInfo()));
    sk_sp<SkSurface> tempOutput;

    // Assume we should ignore the rescale linear request if the surface has no color space since
    // it's unclear how we'd linearize from an unknown color space.
    if (rescaleGamma == Image::RescaleGamma::kLinear &&
        srcImageInfo.colorSpace() &&
        !srcImageInfo.colorSpace()->gammaIsLinear()) {
        // Draw the src image into a new surface with linear gamma, and make that the new tempInput
        sk_sp<SkColorSpace> linearGamma = srcImageInfo.colorSpace()->makeLinearGamma();
        SkImageInfo gammaDstInfo = SkImageInfo::Make(srcIRect.size(),
                                                     tempInput->imageInfo().colorType(),
                                                     kPremul_SkAlphaType,
                                                     std::move(linearGamma));
        tempOutput = make_surface_with_fallback(recorder,
                                                gammaDstInfo,
                                                Mipmapped::kNo,
                                                &surfaceProps);
        if (!tempOutput) {
            return nullptr;
        }
        SkCanvas* gammaDst = tempOutput->getCanvas();
        SkRect gammaDstRect = SkRect::Make(srcIRect.size());

        SkPaint paint;
        gammaDst->drawImageRect(tempInput, srcRect, gammaDstRect,
                                SkSamplingOptions(SkFilterMode::kNearest), &paint,
                                SkCanvas::kStrict_SrcRectConstraint);
        tempInput = SkSurfaces::AsImage(tempOutput);
        srcRect = gammaDstRect;
    }

    SkImageInfo outImageInfo = tempInput->imageInfo().makeAlphaType(kPremul_SkAlphaType);
    do {
        SkISize nextDims = finalSize;
        if (rescaleMode != Image::RescaleMode::kNearest &&
            rescaleMode != Image::RescaleMode::kLinear) {
            if (srcRect.width() > finalSize.width()) {
                nextDims.fWidth = std::max((srcRect.width() + 1)/2, (float)finalSize.width());
            } else if (srcRect.width() < finalSize.width()) {
                nextDims.fWidth = std::min(srcRect.width()*2, (float)finalSize.width());
            }
            if (srcRect.height() > finalSize.height()) {
                nextDims.fHeight = std::max((srcRect.height() + 1)/2, (float)finalSize.height());
            } else if (srcRect.height() < finalSize.height()) {
                nextDims.fHeight = std::min(srcRect.height()*2, (float)finalSize.height());
            }
        }

        SkCanvas* stepDst;
        SkRect stepDstRect;
        if (nextDims == finalSize) {
            stepDst = dst->getCanvas();
            stepDstRect = dstRect;
        } else {
            SkImageInfo nextInfo = outImageInfo.makeDimensions(nextDims);
            tempOutput = make_surface_with_fallback(recorder,
                                                    nextInfo,
                                                    Mipmapped::kNo,
                                                    &surfaceProps);
            if (!tempOutput) {
                return nullptr;
            }
            stepDst = tempOutput->getCanvas();
            stepDstRect = SkRect::Make(tempOutput->imageInfo().dimensions());
        }

        SkSamplingOptions samplingOptions;
        if (rescaleMode == Image::RescaleMode::kRepeatedCubic) {
            samplingOptions = SkSamplingOptions(SkCubicResampler::CatmullRom());
        } else {
            samplingOptions = (rescaleMode == Image::RescaleMode::kNearest) ?
                               SkSamplingOptions(SkFilterMode::kNearest) :
                               SkSamplingOptions(SkFilterMode::kLinear);
        }
        SkPaint paint;
        stepDst->drawImageRect(tempInput, srcRect, stepDstRect, samplingOptions, &paint,
                               SkCanvas::kStrict_SrcRectConstraint);

        tempInput = SkSurfaces::AsImage(tempOutput);
        srcRect = SkRect::Make(nextDims);
    } while (srcRect.width() != finalSize.width() || srcRect.height() != finalSize.height());

    return SkSurfaces::AsImage(dst);
}

} // namespace skgpu::graphite
