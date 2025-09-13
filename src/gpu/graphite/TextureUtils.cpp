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
#include "include/effects/SkRuntimeEffect.h"
#include "src/core/SkBlurEngine.h"
#include "src/core/SkCompressedDataUtils.h"
#include "src/core/SkDevice.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/image/SkImage_Base.h"

#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/BlurUtils.h"
#include "src/gpu/RefCntedCallback.h"
#include "src/gpu/SkBackingFit.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/RuntimeEffectDictionary.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureInfoPriv.h"
#include "src/gpu/graphite/task/CopyTask.h"
#include "src/gpu/graphite/task/SynchronizeToCpuTask.h"
#include "src/gpu/graphite/task/UploadTask.h"

#include <array>


using SkImages::GraphitePromiseTextureFulfillProc;
using SkImages::GraphitePromiseTextureFulfillContext;
using SkImages::GraphitePromiseTextureReleaseProc;

namespace skgpu::graphite {

namespace {

sk_sp<Surface> make_renderable_scratch_surface(
        Recorder* recorder,
        const SkImageInfo& info,
        SkBackingFit backingFit,
        std::string_view label,
        const SkSurfaceProps* surfaceProps = nullptr) {
    SkColorType ct = recorder->priv().caps()->getRenderableColorType(info.colorType());
    if (ct == kUnknown_SkColorType) {
        return nullptr;
    }

    // TODO(b/323886870): Historically the scratch surfaces used here were exact-fit but they should
    // be able to be approx-fit and uninstantiated.
    return Surface::MakeScratch(recorder,
                                info.makeColorType(ct),
                                std::move(label),
                                Budgeted::kYes,
                                Mipmapped::kNo,
                                backingFit);
}

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

    // We require provided images to have a TopLeft origin
    auto graphiteImage = static_cast<const Image*>(clientProvided);
    if (graphiteImage->textureProxyView().origin() != Origin::kTopLeft) {
        SKGPU_LOG_E("Client provided image must have a TopLeft origin.");
        return false;
    }

    return true;
}

// This class is the lazy instantiation callback for promise images. It manages calling the
// client's Fulfill, ImageRelease, and TextureRelease procs.
class PromiseLazyInstantiateCallback {
public:
    PromiseLazyInstantiateCallback(sk_sp<RefCntedCallback> releaseHelper,
                                   GraphitePromiseTextureFulfillProc fulfillProc,
                                   GraphitePromiseTextureFulfillContext fulfillContext,
                                   GraphitePromiseTextureReleaseProc textureReleaseProc,
                                   std::string_view label)
            : fReleaseHelper(std::move(releaseHelper))
            , fFulfillProc(fulfillProc)
            , fFulfillContext(fulfillContext)
            , fTextureReleaseProc(textureReleaseProc)
            , fLabel(label) {
    }
    PromiseLazyInstantiateCallback(PromiseLazyInstantiateCallback&&) = default;
    PromiseLazyInstantiateCallback(const PromiseLazyInstantiateCallback&) {
        // Because we get wrapped in std::function we must be copyable. But we should never
        // be copied.
        SkASSERT(false);
    }
    PromiseLazyInstantiateCallback& operator=(PromiseLazyInstantiateCallback&&) = default;
    PromiseLazyInstantiateCallback& operator=(const PromiseLazyInstantiateCallback&) {
        SkASSERT(false);
        return *this;
    }

    sk_sp<Texture> operator()(ResourceProvider* resourceProvider) {
        // Invoke the fulfill proc to get the promised backend texture.
        auto [ backendTexture, textureReleaseCtx ] = fFulfillProc(fFulfillContext);
        if (!backendTexture.isValid()) {
            SKGPU_LOG_W("FulfillProc returned an invalid backend texture");
            return nullptr;
        }

        sk_sp<RefCntedCallback> textureReleaseCB = RefCntedCallback::Make(fTextureReleaseProc,
                                                                          textureReleaseCtx);

        sk_sp<Texture> texture = resourceProvider->createWrappedTexture(backendTexture,
                                                                        std::move(fLabel));
        if (!texture) {
            SKGPU_LOG_W("Failed to wrap BackendTexture returned by fulfill proc");
            return nullptr;
        }
        texture->setReleaseCallback(std::move(textureReleaseCB));
        return texture;
    }

private:
    sk_sp<RefCntedCallback> fReleaseHelper;
    GraphitePromiseTextureFulfillProc fFulfillProc;
    GraphitePromiseTextureFulfillContext fFulfillContext;
    GraphitePromiseTextureReleaseProc fTextureReleaseProc;
    std::string fLabel;
};

} // anonymous namespace

std::tuple<TextureProxyView, SkColorType> MakeBitmapProxyView(Recorder* recorder,
                                                              const SkBitmap& bitmap,
                                                              sk_sp<SkMipmap> mipmapsIn,
                                                              Mipmapped mipmapped,
                                                              Budgeted budgeted,
                                                              std::string_view label) {
    // Adjust params based on input and Caps
    const Caps* caps = recorder->priv().caps();
    SkColorType ct = bitmap.info().colorType();

    if (bitmap.dimensions().area() <= 1) {
        mipmapped = Mipmapped::kNo;
    }

    Protected isProtected = recorder->priv().isProtected();
    auto textureInfo = caps->getDefaultSampledTextureInfo(ct, mipmapped, isProtected,
                                                          Renderable::kNo);
    if (!textureInfo.isValid()) {
        ct = kRGBA_8888_SkColorType;
        textureInfo = caps->getDefaultSampledTextureInfo(ct, mipmapped, isProtected,
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
        mipmaps = SkToBool(mipmapsIn)
                          ? mipmapsIn
                          : sk_sp<SkMipmap>(SkMipmap::Build(bmpToUpload.pixmap(), nullptr));
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
    sk_sp<TextureProxy> proxy = TextureProxy::Make(caps,
                                                   recorder->priv().resourceProvider(),
                                                   bmpToUpload.dimensions(),
                                                   textureInfo,
                                                   std::move(label),
                                                   budgeted);
    if (!proxy) {
        return {};
    }
    SkASSERT(caps->areColorTypeAndTextureInfoCompatible(ct, proxy->textureInfo()));
    SkASSERT(mipmapped == Mipmapped::kNo || proxy->mipmapped() == Mipmapped::kYes);

    // Src and dst colorInfo are the same
    const SkColorInfo& colorInfo = bmpToUpload.info().colorInfo();
    // Add upload to the root upload list. These bitmaps are uploaded to unique textures so there is
    // no need to coordinate resource sharing. It is better to then group them into a single task
    // at the start of the Recording.
    const SkIRect dimensions = SkIRect::MakeSize(bmpToUpload.dimensions());
    UploadSource uploadSource = UploadSource::Make(
            recorder->priv().caps(), *proxy, colorInfo, colorInfo, texels, dimensions);
    if (!uploadSource.isValid()) {
        SKGPU_LOG_E("MakeBitmapProxyView: Could not create UploadSource");
        return {};
    }
    if (!recorder->priv().rootUploadList()->recordUpload(recorder,
                                                         proxy,
                                                         colorInfo,
                                                         colorInfo,
                                                         uploadSource,
                                                         dimensions,
                                                         std::make_unique<ImageUploadContext>())) {
        SKGPU_LOG_E("MakeBitmapProxyView: Could not create UploadInstance");
        return {};
    }

    Swizzle swizzle = caps->getReadSwizzle(ct, textureInfo);
    // If the color type is alpha-only, propagate the alpha value to the other channels.
    if (SkColorTypeIsAlphaOnly(colorInfo.colorType())) {
        swizzle = Swizzle::Concat(swizzle, Swizzle("aaaa"));
    }
    return {{std::move(proxy), swizzle}, ct};
}

sk_sp<TextureProxy> MakePromiseImageLazyProxy(
        const Caps* caps,
        SkISize dimensions,
        TextureInfo textureInfo,
        Volatile isVolatile,
        sk_sp<RefCntedCallback> releaseHelper,
        GraphitePromiseTextureFulfillProc fulfillProc,
        GraphitePromiseTextureFulfillContext fulfillContext,
        GraphitePromiseTextureReleaseProc textureReleaseProc,
        std::string_view label) {
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(releaseHelper);

    if (!fulfillProc) {
        return nullptr;
    }

    PromiseLazyInstantiateCallback callback{std::move(releaseHelper), fulfillProc,
                                            fulfillContext, textureReleaseProc, std::move(label)};
    // Proxies for promise images are assumed to always be destined for a client's SkImage so
    // are never considered budgeted.
    return TextureProxy::MakeLazy(caps, dimensions, textureInfo, Budgeted::kNo, isVolatile,
                                  std::move(callback));
}

sk_sp<SkImage> MakeFromBitmap(Recorder* recorder,
                              const SkColorInfo& colorInfo,
                              const SkBitmap& bitmap,
                              sk_sp<SkMipmap> mipmaps,
                              Budgeted budgeted,
                              SkImage::RequiredProperties requiredProps,
                              std::string_view label) {
    auto mm = requiredProps.fMipmapped ? Mipmapped::kYes : Mipmapped::kNo;
    auto [view, ct] = MakeBitmapProxyView(recorder,
                                          bitmap,
                                          std::move(mipmaps),
                                          mm,
                                          budgeted,
                                          std::move(label));
    if (!view) {
        return nullptr;
    }

    SkASSERT(!requiredProps.fMipmapped || view.proxy()->mipmapped() == Mipmapped::kYes);
    return sk_make_sp<skgpu::graphite::Image>(std::move(view), colorInfo.makeColorType(ct));
}

size_t ComputeSize(SkISize dimensions, const TextureInfo& info) {
    TextureFormat format = TextureInfoPriv::ViewFormat(info);
    SkTextureCompressionType compression = TextureFormatCompressionType(format);

    size_t colorSize = 0;

    if (compression != SkTextureCompressionType::kNone) {
        colorSize =  SkCompressedFormatDataSize(compression,
                                                dimensions,
                                                info.mipmapped() == Mipmapped::kYes);
    } else {
        // TODO(b/401016699): Add logic to handle multiplanar formats
        size_t bytesPerPixel = TextureFormatBytesPerBlock(format);

        colorSize = (size_t)dimensions.width() * dimensions.height() * bytesPerPixel;
    }

    size_t finalSize = colorSize * info.numSamples();

    if (info.mipmapped() == Mipmapped::kYes) {
        finalSize += colorSize/3;
    }
    return finalSize;
}

sk_sp<Image> CopyAsDraw(Recorder* recorder,
                        DrawContext* drawContext,
                        const SkImage* image,
                        const SkIRect& subset,
                        const SkColorInfo& dstColorInfo,
                        Budgeted budgeted,
                        Mipmapped mipmapped,
                        SkBackingFit backingFit,
                        std::string_view label) {
    SkColorType ct = recorder->priv().caps()->getRenderableColorType(dstColorInfo.colorType());
    if (ct == kUnknown_SkColorType) {
        return nullptr;
    }
    SkImageInfo dstInfo = SkImageInfo::Make(subset.size(),
                                            dstColorInfo.makeColorType(ct)
                                                        .makeAlphaType(kPremul_SkAlphaType));
    // The surface goes out of scope when we return, so it can be scratch, but it may or may
    // not be budgeted depending on how the copied image is used (or returned to the client).
    sk_sp<Surface> surface = Surface::MakeScratch(recorder,
                                                  dstInfo,
                                                  std::move(label),
                                                  budgeted,
                                                  mipmapped,
                                                  backingFit);
    if (!surface) {
        return nullptr;
    }

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    surface->getCanvas()->drawImage(image, -subset.left(), -subset.top(),
                                    SkFilterMode::kNearest, &paint);
    surface->flushToDrawContext(drawContext);
    return surface->asImage();
}

sk_sp<SkImage> RescaleImage(Recorder* recorder,
                            const SkImage* srcImage,
                            SkIRect srcIRect,
                            const SkImageInfo& dstInfo,
                            SkImage::RescaleGamma rescaleGamma,
                            SkImage::RescaleMode rescaleMode) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    TRACE_EVENT_INSTANT2("skia.gpu", "RescaleImage Src", TRACE_EVENT_SCOPE_THREAD,
                         "width", srcIRect.width(), "height", srcIRect.height());
    TRACE_EVENT_INSTANT2("skia.gpu", "RescaleImage Dst", TRACE_EVENT_SCOPE_THREAD,
                         "width", dstInfo.width(), "height", dstInfo.height());

    // RescaleImage() should only be called when we already know that srcImage is graphite-backed
    SkASSERT(srcImage && as_IB(srcImage)->isGraphiteBacked());

    // For now this needs to be texturable because we can't depend on copies to scale.
    // NOTE: srcView may be empty if srcImage is YUVA.
    const TextureProxyView srcView = AsView(srcImage);
    if (srcView && !recorder->priv().caps()->isTexturable(srcView.proxy()->textureInfo())) {
        // With the current definition of SkImage, this shouldn't happen. If we allow non-texturable
        // formats for compute, we'll need to copy to a texturable format.
        SkASSERT(false);
        return nullptr;
    }

    // make a Surface *exactly* matching dstInfo to rescale into
    SkSurfaceProps surfaceProps = {};
    sk_sp<SkSurface> dst = make_renderable_scratch_surface(recorder,
                                                           dstInfo,
                                                           SkBackingFit::kExact,
                                                           "RescaleDstTexture",
                                                           &surfaceProps);
    if (!dst) {
        return nullptr;
    }

    SkRect srcRect = SkRect::Make(srcIRect);
    SkRect dstRect = SkRect::Make(dstInfo.dimensions());

    SkISize finalSize = SkISize::Make(dstRect.width(), dstRect.height());
    if (finalSize == srcIRect.size()) {
        rescaleGamma = Image::RescaleGamma::kSrc;
        rescaleMode = Image::RescaleMode::kNearest;
    }

    // Within a rescaling pass tempInput is read from and tempOutput is written to.
    // At the end of the pass tempOutput's texture is wrapped and assigned to tempInput.
    sk_sp<SkImage> tempInput = sk_ref_sp(srcImage);
    sk_sp<SkSurface> tempOutput;

    // Assume we should ignore the rescale linear request if the surface has no color space since
    // it's unclear how we'd linearize from an unknown color space.
    const SkImageInfo& srcImageInfo = srcImage->imageInfo();
    if (rescaleGamma == Image::RescaleGamma::kLinear &&
        srcImageInfo.colorSpace() &&
        !srcImageInfo.colorSpace()->gammaIsLinear()) {
        // Draw the src image into a new surface with linear gamma, and make that the new tempInput
        sk_sp<SkColorSpace> linearGamma = srcImageInfo.colorSpace()->makeLinearGamma();
        SkImageInfo gammaDstInfo = SkImageInfo::Make(srcIRect.size(),
                                                     tempInput->imageInfo().colorType(),
                                                     kPremul_SkAlphaType,
                                                     std::move(linearGamma));
        tempOutput = make_renderable_scratch_surface(recorder, gammaDstInfo, SkBackingFit::kApprox,
                                                     "RescaleLinearGammaTexture", &surfaceProps);
        if (!tempOutput) {
            return nullptr;
        }
        SkCanvas* gammaDst = tempOutput->getCanvas();
        SkRect gammaDstRect = SkRect::Make(srcIRect.size());

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        gammaDst->drawImageRect(tempInput, srcRect, gammaDstRect,
                                SkSamplingOptions(SkFilterMode::kNearest), &paint,
                                SkCanvas::kStrict_SrcRectConstraint);
        tempInput = SkSurfaces::AsImage(std::move(tempOutput));
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

        SkRect stepDstRect;
        if (nextDims == finalSize) {
            tempOutput = dst;
            stepDstRect = dstRect;
        } else {
            SkImageInfo nextInfo = outImageInfo.makeDimensions(nextDims);
            tempOutput = make_renderable_scratch_surface(recorder, nextInfo, SkBackingFit::kApprox,
                                                         "RescaleImageTempTexture", &surfaceProps);
            if (!tempOutput) {
                return nullptr;
            }
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
        paint.setBlendMode(SkBlendMode::kSrc);
        tempOutput->getCanvas()->drawImageRect(tempInput, srcRect, stepDstRect, samplingOptions,
                                               &paint, SkCanvas::kStrict_SrcRectConstraint);

        tempInput = SkSurfaces::AsImage(std::move(tempOutput));
        srcRect = SkRect::Make(nextDims);
    } while (srcRect.width() != finalSize.width() || srcRect.height() != finalSize.height());

    return SkSurfaces::AsImage(std::move(dst));
}

bool GenerateMipmaps(Recorder* recorder,
                     DrawContext* drawContext,
                     sk_sp<TextureProxy> texture,
                     const SkColorInfo& colorInfo) {
    constexpr SkSamplingOptions kSamplingOptions = SkSamplingOptions(SkFilterMode::kLinear);

    SkASSERT(texture->mipmapped() == Mipmapped::kYes);

    // Within a rescaling pass scratchImg is read from and a scratch surface is written to.
    // At the end of the pass the scratch surface's texture is wrapped and assigned to scratchImg.

    // The scratch surface we create below will use a write swizzle derived from SkColorType and
    // pixel format. We have to be consistent and swizzle on the read.
    auto imgSwizzle = recorder->priv().caps()->getReadSwizzle(colorInfo.colorType(),
                                                              texture->textureInfo());
    sk_sp<SkImage> scratchImg(new Image(TextureProxyView(texture, imgSwizzle), colorInfo));

    SkISize srcSize = texture->dimensions();
    const SkColorInfo outColorInfo = colorInfo.makeAlphaType(kPremul_SkAlphaType);

    // Alternate between two scratch surfaces to avoid reading from and writing to a texture in the
    // same pass. The dimensions of the first usages of the two scratch textures will be 1/2 and 1/4
    // those of the original texture, respectively.
    sk_sp<Surface> scratchSurfaces[2];
    for (int i = 0; i < 2; ++i) {
        scratchSurfaces[i] = make_renderable_scratch_surface(
                recorder,
                SkImageInfo::Make(SkISize::Make(std::max(1, srcSize.width() >> (i + 1)),
                                                std::max(1, srcSize.height() >> (i + 1))),
                                  outColorInfo),
                SkBackingFit::kApprox,
                "GenerateMipmapsScratchTexture");
        if (!scratchSurfaces[i]) {
            return false;
        }
    }

    for (int mipLevel = 1; srcSize.width() > 1 || srcSize.height() > 1; ++mipLevel) {
        const SkISize dstSize = SkISize::Make(std::max(srcSize.width() >> 1, 1),
                                              std::max(srcSize.height() >> 1, 1));

        Surface* scratchSurface = scratchSurfaces[(mipLevel - 1) & 1].get();

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        scratchSurface->getCanvas()->drawImageRect(scratchImg,
                                                   SkRect::Make(srcSize),
                                                   SkRect::Make(dstSize),
                                                   kSamplingOptions,
                                                   &paint,
                                                   SkCanvas::kStrict_SrcRectConstraint);

        // Make sure the rescaling draw finishes before copying the results.
        scratchSurface->flushToDrawContext(drawContext);

        sk_sp<CopyTextureToTextureTask> copyTask = CopyTextureToTextureTask::Make(
                static_cast<const Surface*>(scratchSurface)->readSurfaceView().refProxy(),
                SkIRect::MakeSize(dstSize),
                texture,
                {0, 0},
                mipLevel);
        if (!copyTask) {
            return false;
        }

        if (drawContext) {
            drawContext->recordDependency(std::move(copyTask));
        } else {
            recorder->priv().add(std::move(copyTask));
        }

        scratchImg = scratchSurface->asImage();
        srcSize = dstSize;
    }

    return true;
}

std::pair<sk_sp<SkImage>, SkSamplingOptions> GetGraphiteBacked(Recorder* recorder,
                                                               const SkImage* imageIn,
                                                               SkSamplingOptions sampling) {
    Mipmapped mipmapped = (sampling.mipmap != SkMipmapMode::kNone)
                                     ? Mipmapped::kYes : Mipmapped::kNo;

    if (imageIn->dimensions().area() <= 1 && mipmapped == Mipmapped::kYes) {
        mipmapped = Mipmapped::kNo;
        sampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
    }

    sk_sp<SkImage> result;
    if (as_IB(imageIn)->isGraphiteBacked()) {
        result = sk_ref_sp(imageIn);

        // If the preexisting Graphite-backed image doesn't have the required mipmaps we will drop
        // down the sampling
        if (mipmapped == Mipmapped::kYes && !result->hasMipmaps()) {
            mipmapped = Mipmapped::kNo;
            sampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
        }
    } else {
        auto clientImageProvider = recorder->clientImageProvider();
        result = clientImageProvider->findOrCreate(
                recorder, imageIn, {mipmapped == Mipmapped::kYes});

        if (!valid_client_provided_image(
                    result.get(), imageIn, {mipmapped == Mipmapped::kYes})) {
            // The client did not fulfill the ImageProvider contract so drop the image.
            result = nullptr;
        }
    }

    if (sampling.isAniso() && result) {
        sampling = SkSamplingPriv::AnisoFallback(result->hasMipmaps());
    }

    return { result, sampling };
}

TextureProxyView AsView(const SkImage* image) {
    if (!image) {
        return {};
    }
    if (!as_IB(image)->isGraphiteBacked()) {
        return {};
    }
    // A YUVA image (even if backed by graphite textures) is not a single texture
    if (as_IB(image)->isYUVA()) {
        return {};
    }

    auto gi = reinterpret_cast<const Image*>(image);
    return gi->textureProxyView();
}

SkColorType ComputeShaderCoverageMaskTargetFormat(const Caps* caps) {
    // GPU compute coverage mask renderers need to bind the mask texture as a storage binding, which
    // support a limited set of color formats. In general, we use RGBA8 if Alpha8 can't be
    // supported.
    if (caps->isStorage(caps->getDefaultStorageTextureInfo(kAlpha_8_SkColorType))) {
        return kAlpha_8_SkColorType;
    }
    return kRGBA_8888_SkColorType;
}

} // namespace skgpu::graphite

namespace skif {

namespace {

// TODO(michaelludwig): The skgpu::BlurUtils effects will be migrated to src/core to implement a
// shader BlurEngine that can be shared by rastr, Ganesh, and Graphite. This is blocked by having
// skif::FilterResult handle the resizing to the max supported sigma.
class GraphiteBackend :
        public Backend,
        private SkShaderBlurAlgorithm,
        private SkBlurEngine {
public:

    GraphiteBackend(skgpu::graphite::Recorder* recorder,
                    const SkSurfaceProps& surfaceProps,
                    SkColorType colorType)
            : Backend(SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize),
                      surfaceProps, colorType)
            , fRecorder(recorder) {}

    // Backend
    sk_sp<SkDevice> makeDevice(SkISize size,
                               sk_sp<SkColorSpace> colorSpace,
                               const SkSurfaceProps* props) const override {
        SkImageInfo imageInfo = SkImageInfo::Make(size,
                                                  this->colorType(),
                                                  kPremul_SkAlphaType,
                                                  std::move(colorSpace));
        return skgpu::graphite::Device::Make(fRecorder,
                                             imageInfo,
                                             skgpu::Budgeted::kYes,
                                             skgpu::Mipmapped::kNo,
                                             SkBackingFit::kApprox,
                                             props ? *props : this->surfaceProps(),
                                             skgpu::graphite::LoadOp::kDiscard,
                                             "ImageFilterResult");
    }

    sk_sp<SkSpecialImage> makeImage(const SkIRect& subset, sk_sp<SkImage> image) const override {
        return SkSpecialImages::MakeGraphite(fRecorder, subset, image, this->surfaceProps());
    }

    sk_sp<SkImage> getCachedBitmap(const SkBitmap& data) const override {
        auto proxy = skgpu::graphite::RecorderPriv::CreateCachedProxy(fRecorder, data,
                                                                      "ImageFilterCachedBitmap");
        if (!proxy) {
            return nullptr;
        }

        const SkColorInfo& colorInfo = data.info().colorInfo();
        skgpu::Swizzle swizzle = fRecorder->priv().caps()->getReadSwizzle(colorInfo.colorType(),
                                                                          proxy->textureInfo());
        return sk_make_sp<skgpu::graphite::Image>(
                skgpu::graphite::TextureProxyView(std::move(proxy), swizzle),
                colorInfo);
    }

    const SkBlurEngine* getBlurEngine() const override { return this; }

    // SkBlurEngine
    const SkBlurEngine::Algorithm* findAlgorithm(SkSize sigma,
                                                 SkColorType colorType) const override {
        // The runtime effect blurs handle all tilemodes and color types
        return this;
    }

    // SkShaderBlurAlgorithm
    sk_sp<SkDevice> makeDevice(const SkImageInfo& imageInfo) const override {
        return skgpu::graphite::Device::Make(fRecorder,
                                             imageInfo,
                                             skgpu::Budgeted::kYes,
                                             skgpu::Mipmapped::kNo,
                                             SkBackingFit::kApprox,
                                             this->surfaceProps(),
                                             skgpu::graphite::LoadOp::kDiscard,
                                             "EvalBlurTexture");
    }

private:
    skgpu::graphite::Recorder* fRecorder;
};

} // anonymous namespace

sk_sp<Backend> MakeGraphiteBackend(skgpu::graphite::Recorder* recorder,
                                   const SkSurfaceProps& surfaceProps,
                                   SkColorType colorType) {
    SkASSERT(recorder);
    return sk_make_sp<GraphiteBackend>(recorder, surfaceProps, colorType);
}

}  // namespace skif
