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
#include "src/gpu/graphite/Image_Base_Graphite.h"
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

// We choose a fallback color type that will map to the same texture format as `dstCT` but will be
// considered renderable (assuming the format is supported). This allows the content to be
// rendered and then we "cast" back to the original requested color type for the image view of the
// scratch surface.
//
// This function takes in the `srcCT` as well because it can enable some fallbacks that are not
// otherwise possible (e.g. gray -> red -> gray).
SkColorType renderable_colortype(SkColorType srcCT, SkColorType dstCT) {
    // This mapping only includes color types that are often deemed non-renderable because of
    // semantics (e.g. can't blend into an alpha channel that is meant to be masked during sampling,
    // or can't render gray from an arbitrary RGB source).
    //
    // We intentionally don't support falling back from one color type to another if it changes
    // the underlying data type. It is better to fail the operation to signal to the client that
    // the action isn't possible and let them choose what sort of fallback should happen.
    switch (dstCT) {
        // For these types we can always render with the alpha channel and not worry about
        // blending because every draw operation uses kSrc and we're filling all pixels. The
        // image view will then still use an rgb1 swizzle to hide any bad alpha data from the
        // original image.
        case kRGB_101010x_SkColorType:    return kRGBA_1010102_SkColorType;
        case kBGR_101010x_SkColorType:    return kBGRA_1010102_SkColorType;
        case kRGB_F16F16F16x_SkColorType: return kRGBA_F16_SkColorType;
        case kRGBA_F16Norm_SkColorType:   return kRGBA_F16_SkColorType;

        // While it is the case that a BGRA format can be used with a kRGB_888x colortype, we
        // don't look at srcCT to guess the format and switch to kBGRA_8888. In the event that
        // this copied image will be read back to the CPU, it's best to match the color type's
        // channel ordering.
        case kRGB_888x_SkColorType:       return kRGBA_8888_SkColorType;

        // Normally kGray is never renderable from arbitrary RGB color data because calculating the
        // luminance/gray level is a dot product. However, if the source color type is also gray,
        // then the only channel we care about is R, which is renderable. After rendering to R,
        // the image view's swizzle can splat that out to produce grayscale.
        case kGray_8_SkColorType:
            if (srcCT == kGray_8_SkColorType) {
                return kR8_unorm_SkColorType;
            } else {
                return kUnknown_SkColorType;
            }

        default:
            // If this color type isn't renderable or doesn't map to a supported format, then there
            // isn't any other fallback that can happen.
            return dstCT;
    }
}

SkAlphaType renderable_alphatype(SkAlphaType srcAT, SkAlphaType dstAT) {
    switch (srcAT) {
        case kUnknown_SkAlphaType:
            // The src image will be forced opaque as part of sampling, so the output pixels will
            // be guaranteed opaque (can upgrade requested kPremul or kUnpremul to kOpaque since the
            // RGB values are unchanged).
            return kOpaque_SkAlphaType;
        case kOpaque_SkAlphaType:
            // The src image claims to be opaque, so the output pixels should be opaque.
            return kOpaque_SkAlphaType;
        case kPremul_SkAlphaType:
            // Always render to kPremul, regardless of the requested dst AT. If the dst AT was
            // kPremul this is a no-op. If it was kOpaque, SkColorSpaceXformSteps treats it as the
            // src AT, so it's also a no-op. If it was kUnknown, the image view will presumably be
            // sampled with a masked opaque alpha channel, so producing premul RGB values emulates
            // having blended with solid black. If it is kUnpremul, there is no current way to
            // render to a kUnpremul render target; using kPremul here allows the copy to proceed
            // and then any unpremul math will happen during sampling or readback conversion.
            return kPremul_SkAlphaType;
        case kUnpremul_SkAlphaType:
            // If the requested dst AT is kPremul, then keep that so the premultiply is performed
            // during the copy conversion. In all other cases, switch to kOpaque so that we are
            // deemed renderable and color conversion in SkColorSpaceXformSteps produces a no-op.
            // Since the only actual rendering to this surface will be pixel-filling with kSrc
            // blending, this alpha type manipulation is valid.
            return dstAT == kPremul_SkAlphaType ? kPremul_SkAlphaType : kOpaque_SkAlphaType;
    }
    SkUNREACHABLE;
}

SkAlphaType final_alphatype(SkAlphaType srcAT, SkAlphaType renderedAT) {
    // Assuming `renderedAT` was the result of calling `renderable_alphatype` for `srcAT` and some
    // `dstAT`, the final AT to use for the image view is almost always `renderedAT` because it is
    // either more accurate (propogates src opaque-ness into the copy's alpha type), a no-op (it
    // was premul), or unpremul output was requested for premul input and that's not supported so
    // we need to reflect the copy as premul still in its image info.
    //
    // The only exception is for when both srcAT and dstAT were unpremul, in which case
    // `renderedAT` is manipulated to be kOpaque for rendering but in actuality the output remains
    // unpremul and that should be reflected in the final image info as well.
    return (srcAT == kUnpremul_SkAlphaType && renderedAT == kOpaque_SkAlphaType) ?
            kUnpremul_SkAlphaType : renderedAT;
}

SkColorInfo make_renderable(const SkColorInfo& srcInfo, const SkColorInfo& dstInfo) {
    return dstInfo.makeColorType(renderable_colortype(srcInfo.colorType(), dstInfo.colorType()))
                  .makeAlphaType(renderable_alphatype(srcInfo.alphaType(), dstInfo.alphaType()));
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

TextureProxyView MakeBitmapProxyView(Recorder* recorder,
                                     const SkBitmap& bitmap,
                                     sk_sp<SkMipmap> mipmapsIn,
                                     Mipmapped mipmapped,
                                     Budgeted budgeted,
                                     std::string_view label) {
    // Adjust params based on input and Caps
    const Caps* caps = recorder->priv().caps();
    const SkColorType ct = bitmap.info().colorType();

    if (bitmap.dimensions().area() <= 1) {
        mipmapped = Mipmapped::kNo;
    }

    Protected isProtected = recorder->priv().isProtected();
    auto textureInfo = caps->getDefaultSampledTextureInfo(ct, mipmapped, isProtected,
                                                          Renderable::kNo);
    if (!textureInfo.isValid()) {
        return {};
    }
    if (!SkImageInfoIsValid(bitmap.info())) {
        return {};
    }

    int mipLevelCount = (mipmapped == Mipmapped::kYes) ?
            SkMipmap::ComputeLevelCount(bitmap.width(), bitmap.height()) + 1 : 1;

    // setup MipLevels
    sk_sp<SkMipmap> mipmaps;
    std::vector<MipLevel> texels;
    if (mipLevelCount == 1) {
        texels.resize(mipLevelCount);
        texels[0].fPixels = bitmap.getPixels();
        texels[0].fRowBytes = bitmap.rowBytes();
    } else {
        mipmaps = SkToBool(mipmapsIn)
                          ? mipmapsIn
                          : sk_sp<SkMipmap>(SkMipmap::Build(bitmap.pixmap(), nullptr));
        if (!mipmaps) {
            return {};
        }

        SkASSERT(mipLevelCount == mipmaps->countLevels() + 1);
        texels.resize(mipLevelCount);

        texels[0].fPixels = bitmap.getPixels();
        texels[0].fRowBytes = bitmap.rowBytes();

        for (int i = 1; i < mipLevelCount; ++i) {
            SkMipmap::Level generatedMipLevel;
            mipmaps->getLevel(i - 1, &generatedMipLevel);
            texels[i].fPixels = generatedMipLevel.fPixmap.addr();
            texels[i].fRowBytes = generatedMipLevel.fPixmap.rowBytes();
            SkASSERT(texels[i].fPixels);
            SkASSERT(generatedMipLevel.fPixmap.colorType() == bitmap.colorType());
        }
    }

    // Create proxy
    sk_sp<TextureProxy> proxy = TextureProxy::Make(caps,
                                                   recorder->priv().resourceProvider(),
                                                   bitmap.dimensions(),
                                                   textureInfo,
                                                   label,
                                                   budgeted);
    if (!proxy) {
        return {};
    }
    SkASSERT(caps->areColorTypeAndTextureInfoCompatible(ct, proxy->textureInfo()));
    SkASSERT(mipmapped == Mipmapped::kNo || proxy->mipmapped() == Mipmapped::kYes);

    // Src and dst colorInfo are the same
    const SkColorInfo& colorInfo = bitmap.info().colorInfo();
    // Add upload to the root upload list. These bitmaps are uploaded to unique textures so there is
    // no need to coordinate resource sharing. It is better to then group them into a single task
    // at the start of the Recording.
    const SkIRect dimensions = SkIRect::MakeSize(bitmap.dimensions());
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

    return {std::move(proxy), caps->getReadSwizzle(ct, textureInfo)};
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
                                            fulfillContext, textureReleaseProc, label};
    // Proxies for promise images are assumed to always be destined for a client's SkImage so
    // are never considered budgeted.
    return TextureProxy::MakeLazy(caps, dimensions, textureInfo, Budgeted::kNo, isVolatile,
                                  std::move(callback));
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

    size_t finalSize = colorSize * (uint8_t) info.sampleCount();

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
    // NOTE: This info may not exactly match `dstColorInfo` but will be castable back to
    // `dstColorInfo` when we create the Image view.
    SkImageInfo dstInfo = SkImageInfo::Make(
            subset.size(), make_renderable(image->imageInfo().colorInfo(), dstColorInfo));

    // The surface goes out of scope when we return, so it can be scratch, but it may or may
    // not be budgeted depending on how the copied image is used (or returned to the client).
    sk_sp<Surface> surface =
            Surface::MakeScratch(recorder, dstInfo, label, budgeted, mipmapped, backingFit);
    if (!surface) {
        return nullptr;
    }

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    surface->getCanvas()->drawImage(image, -subset.left(), -subset.top(),
                                    SkFilterMode::kNearest, &paint);
    surface->flushToDrawContext(drawContext);
    // Get the image with the actual requested color type and the final alpha type.
    return surface->asImage(dstColorInfo.colorType(),
                            final_alphatype(image->alphaType(), dstInfo.alphaType()));
}

sk_sp<Image> RescaleImage(Recorder* recorder,
                          const Image_Base* srcImage,
                          SkIRect srcIRect,
                          const SkImageInfo& dstInfo,
                          SkImage::RescaleGamma rescaleGamma,
                          SkImage::RescaleMode rescaleMode) {
    TRACE_EVENT0("skia.gpu", TRACE_FUNC);
    TRACE_EVENT_INSTANT2("skia.gpu", "RescaleImage Src", TRACE_EVENT_SCOPE_THREAD,
                         "width", srcIRect.width(), "height", srcIRect.height());
    TRACE_EVENT_INSTANT2("skia.gpu", "RescaleImage Dst", TRACE_EVENT_SCOPE_THREAD,
                         "width", dstInfo.width(), "height", dstInfo.height());

    // Other than the final step, rescaling will be performed in the source color type and color
    // space, possibly with a linear gamma adjustment (although changing the SkColorSpace can be
    // applied to the result of make_renderable).
    SkColorInfo stepInfo = make_renderable(srcImage->imageInfo().colorInfo(),
                                           srcImage->imageInfo().colorInfo());

    // Make a Surface *exactly* (barring renderable adjustments) matching dstInfo for the final
    // scaling step.
    SkColorInfo renderableDstInfo =
            make_renderable(srcImage->imageInfo().colorInfo(), dstInfo.colorInfo());
    sk_sp<Surface> dst = Surface::MakeScratch(
            recorder,
            SkImageInfo::Make(dstInfo.dimensions(), renderableDstInfo),
            "RescaleDstTexture",
            Budgeted::kYes,
            Mipmapped::kNo,
            SkBackingFit::kExact);
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
    sk_sp<Image_Base> tempInput = sk_ref_sp(srcImage);
    sk_sp<Surface> tempOutput;

    // Assume we should ignore the rescale linear request if the surface has no color space since
    // it's unclear how we'd linearize from an unknown color space.
    if (rescaleGamma == Image::RescaleGamma::kLinear &&
        stepInfo.colorSpace() &&
        !stepInfo.colorSpace()->gammaIsLinear()) {
        // Draw the src image into a new surface with linear gamma, and make that the new tempInput
        sk_sp<SkColorSpace> linearGamma = stepInfo.colorSpace()->makeLinearGamma();
        SkImageInfo gammaDstInfo = SkImageInfo::Make(srcIRect.size(),
                                                     stepInfo.makeColorSpace(linearGamma));
        tempOutput = Surface::MakeScratch(recorder,
                                          gammaDstInfo,
                                          "RescaleLinearGammaTexture");

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
        tempInput = tempOutput->asImage();
        srcRect = gammaDstRect;
        stepInfo = gammaDstInfo.colorInfo(); // remaining steps output linear gamma too
    }

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
            // The final surface's color info is `renderableDstInfo`
            tempOutput = dst;
            stepDstRect = dstRect;
        } else {
            tempOutput = Surface::MakeScratch(recorder,
                                              SkImageInfo::Make(nextDims, stepInfo),
                                              "RescaleImageTempTexture");
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

        tempInput = tempOutput->asImage();
        srcRect = SkRect::Make(nextDims);
    } while (srcRect.width() != finalSize.width() || srcRect.height() != finalSize.height());

    // Cast back to the requested color type and final alpha type now that rendering is finished.
    return dst->asImage(dstInfo.colorType(),
                        final_alphatype(srcImage->alphaType(), renderableDstInfo.alphaType()));
}

bool GenerateMipmaps(Recorder* recorder, DrawContext* drawContext, sk_sp<TextureProxy> texture) {
    SkASSERT(texture->mipmapped() == Mipmapped::kYes);

    // GenerateMipmaps uses Surface and Image to generate mipmaps by drawing each level at 1/2
    // scale compared to the last level and then copying from the scratch surface into `texture`.
    // Surface and Image require SkColorInfo but it does not matter what is chosen so long as the
    // final color management results in the identity function (including read/write swizzles).
    // This ensures that whatever the original color handling of `texture` is, the regenerated
    // levels will match. To do this, we use these settings for both surface and images
    //   - the default color type for the texture's format
    //   - kOpaque_SkAlphaType to disable all premul/unpremul conversions and remain renderable,
    //     (which is okay since all our drawing uses kSrc blending anyways)
    //   - provide no SkColorSpace
    //
    // NOTE: In the future, if we were generate mipmaps in linear gamma, we would need to know about
    // the original image's color space and alpha type.  We would also have to implement a custom
    // filtering shader that sampled the base level several times with nearest filtering, convert
    // each sample to linear+premul space, average them, and then convert that to the source color
    // space and alpha type.
    SkColorType colorType = recorder->priv().caps()->getDefaultColorType(texture->textureInfo());
    SkColorInfo colorInfo{colorType, kOpaque_SkAlphaType, /*cs=*/nullptr};
    // Since we are creating the color info from the default color type for the texture format,
    // it should match what we'd expect from make_renderable already.
    SkASSERT(make_renderable(colorInfo, colorInfo) == colorInfo);

    // Configure swizzle for the initial image to match what happens in Surface::asImage()
    auto imgSwizzle = recorder->priv().caps()->getReadSwizzle(colorInfo.colorType(),
                                                              texture->textureInfo());
    sk_sp<SkImage> scratchImg(new Image(TextureProxyView(texture, imgSwizzle), colorInfo));

    // Alternate between two scratch surfaces to avoid reading from and writing to a texture in the
    // same pass. The dimensions of the first usages of the two scratch textures will be 1/2 and 1/4
    // those of the original texture, respectively.
    SkISize srcSize = texture->dimensions();
    sk_sp<Surface> scratchSurfaces[2];
    for (int i = 0; i < 2; ++i) {
        scratchSurfaces[i] = Surface::MakeScratch(
                recorder,
                SkImageInfo::Make(SkISize::Make(std::max(1, srcSize.width() >> (i + 1)),
                                                std::max(1, srcSize.height() >> (i + 1))),
                                  colorInfo),
                "GenerateMipmapsScratchTexture",
                Budgeted::kYes,
                Mipmapped::kNo,
                SkBackingFit::kApprox);
        if (!scratchSurfaces[i]) {
            return false;
        }
    }

    // Within a rescaling pass scratchImg is read from and a scratch surface is written to.
    // At the end of the pass the scratch surface's texture is wrapped and assigned to scratchImg.
    for (int mipLevel = 1; srcSize.width() > 1 || srcSize.height() > 1; ++mipLevel) {
        const SkISize dstSize = SkISize::Make(std::max(srcSize.width() >> 1, 1),
                                              std::max(srcSize.height() >> 1, 1));

        Surface* scratchSurface = scratchSurfaces[(mipLevel - 1) & 1].get();

        SkPaint paint;
        paint.setBlendMode(SkBlendMode::kSrc);
        scratchSurface->getCanvas()->drawImageRect(scratchImg,
                                                   SkRect::Make(srcSize),
                                                   SkRect::Make(dstSize),
                                                   SkFilterMode::kLinear,
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
