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
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/ResourceTypes.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/Texture.h"
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
                                SkBackingFit::kExact);
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

sk_sp<SkSpecialImage> eval_blur(Recorder* recorder,
                                sk_sp<SkShader> blurEffect,
                                const SkIRect& dstRect,
                                SkColorType colorType,
                                sk_sp<SkColorSpace> outCS,
                                const SkSurfaceProps& outProps) {
    SkImageInfo outII = SkImageInfo::Make({dstRect.width(), dstRect.height()},
                                          colorType, kPremul_SkAlphaType, std::move(outCS));
    // Protected-ness is pulled off of the recorder
    auto device = Device::Make(recorder,
                               outII,
                               Budgeted::kYes,
                               Mipmapped::kNo,
#if defined(GRAPHITE_USE_APPROX_FIT_FOR_FILTERS)
                               SkBackingFit::kApprox,
#else
                               SkBackingFit::kExact,
#endif
                               outProps,
                               LoadOp::kDiscard,
                               "EvalBlurTexture");
    if (!device) {
        return nullptr;
    }

    // TODO(b/294102201): This is very much like AutoSurface in SkImageFilterTypes.cpp
    SkIRect subset = SkIRect::MakeSize(dstRect.size());
    device->clipRect(SkRect::Make(subset), SkClipOp::kIntersect, /*aa=*/false);
    device->setLocalToDevice(SkM44::Translate(-dstRect.left(), -dstRect.top()));
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    paint.setShader(std::move(blurEffect));
    device->drawPaint(paint);
    return device->snapSpecial(subset);
}

sk_sp<SkSpecialImage> blur_2d(Recorder* recorder,
                              SkSize sigma,
                              SkISize radii,
                              sk_sp<SkSpecialImage> input,
                              const SkIRect& srcRect,
                              SkTileMode tileMode,
                              const SkIRect& dstRect,
                              sk_sp<SkColorSpace> outCS,
                              const SkSurfaceProps& outProps) {
    std::array<SkV4, kMaxBlurSamples/4> kernel;
    std::array<SkV4, kMaxBlurSamples/2> offsets;
    Compute2DBlurKernel(sigma, radii, kernel);
    Compute2DBlurOffsets(radii, offsets);

    SkRuntimeShaderBuilder builder{sk_ref_sp(GetBlur2DEffect(radii))};
    builder.uniform("kernel") = kernel;
    builder.uniform("offsets") = offsets;
    // TODO(b/294102201): This is very much like FilterResult::asShader()...
    builder.child("child") =
            input->makeSubset(srcRect)->asShader(tileMode,
                                                 SkFilterMode::kNearest,
                                                 SkMatrix::Translate(srcRect.left(),srcRect.top()));

    return eval_blur(recorder, builder.makeShader(), dstRect,
                     input->colorType(), std::move(outCS), outProps);
}

sk_sp<SkSpecialImage> blur_1d(Recorder* recorder,
                              float sigma,
                              int radius,
                              SkV2 dir,
                              sk_sp<SkSpecialImage> input,
                              SkIRect srcRect,
                              SkTileMode tileMode,
                              SkIRect dstRect,
                              sk_sp<SkColorSpace> outCS,
                              const SkSurfaceProps& outProps) {
    std::array<SkV4, kMaxBlurSamples/2> offsetsAndKernel;
    Compute1DBlurLinearKernel(sigma, radius, offsetsAndKernel);

    SkRuntimeShaderBuilder builder{sk_ref_sp(GetLinearBlur1DEffect(radius))};
    builder.uniform("offsetsAndKernel") = offsetsAndKernel;
    builder.uniform("dir") = dir;
    // TODO(b/294102201): This is very much like FilterResult::asShader()...
    builder.child("child") =
            input->makeSubset(srcRect)->asShader(tileMode,
                                                 SkFilterMode::kLinear,
                                                 SkMatrix::Translate(srcRect.left(),srcRect.top()));

    return eval_blur(recorder, builder.makeShader(), dstRect,
                     input->colorType(), std::move(outCS), outProps);
}

sk_sp<SkSpecialImage> blur_impl(Recorder* recorder,
                                SkSize sigma,
                                sk_sp<SkSpecialImage> input,
                                SkIRect srcRect,
                                SkTileMode tileMode,
                                SkIRect dstRect,
                                sk_sp<SkColorSpace> outCS,
                                const SkSurfaceProps& outProps) {
    // See if we can do a blur on the original resolution image
    if (sigma.width() <= kMaxLinearBlurSigma &&
        sigma.height() <= kMaxLinearBlurSigma) {
        int radiusX = BlurSigmaRadius(sigma.width());
        int radiusY = BlurSigmaRadius(sigma.height());
        const int kernelArea = BlurKernelWidth(radiusX) * BlurKernelWidth(radiusY);
        if (kernelArea <= kMaxBlurSamples && radiusX > 0 && radiusY > 0) {
            // Use a single-pass 2D kernel if it fits and isn't just 1D already
            return blur_2d(recorder, sigma, {radiusX, radiusY}, std::move(input), srcRect, tileMode,
                           dstRect, std::move(outCS), outProps);
        } else {
            // Use two passes of a 1D kernel (one per axis).
            if (radiusX > 0) {
                SkIRect intermediateDstRect = dstRect;
                if (radiusY > 0) {
                    // Outset the output size of dstRect by the radius required for the next Y pass
                    intermediateDstRect.outset(0, radiusY);
                    if (!intermediateDstRect.intersect(srcRect.makeOutset(radiusX, radiusY))) {
                        return nullptr;
                    }
                }

                input = blur_1d(recorder, sigma.width(), radiusX, {1.f, 0.f},
                                std::move(input), srcRect, tileMode, intermediateDstRect,
                                outCS, outProps);
                if (!input) {
                    return nullptr;
                }
                srcRect = SkIRect::MakeWH(input->width(), input->height());
                dstRect.offset(-intermediateDstRect.left(), -intermediateDstRect.top());
            }

            if (radiusY > 0) {
                input = blur_1d(recorder, sigma.height(), radiusY, {0.f, 1.f},
                                std::move(input), srcRect, tileMode, dstRect, outCS, outProps);
            }

            return input;
        }
    } else {
        // Rescale the source image, blur that with a reduced sigma, and then upscale back to the
        // dstRect dimensions.
        // TODO(b/294102201): Share rescaling logic with GrBlurUtils::GaussianBlur.
        float sx = sigma.width() > kMaxLinearBlurSigma
                ? (kMaxLinearBlurSigma / sigma.width()) : 1.f;
        float sy = sigma.height() > kMaxLinearBlurSigma
                ? (kMaxLinearBlurSigma / sigma.height()) : 1.f;

        int targetSrcWidth = sk_float_ceil2int(srcRect.width() * sx);
        int targetSrcHeight = sk_float_ceil2int(srcRect.height() * sy);

        auto inputImage = input->asImage();
        // TODO(b/288902559): Support approx fit backings for the target of a rescale
        // TODO(b/294102201): Be smarter about downscaling when there are actual tilemodes to apply
        // to the image.
        auto scaledInput = RescaleImage(
                recorder,
                inputImage.get(),
                srcRect.makeOffset(input->subset().topLeft()),
                inputImage->imageInfo().makeWH(targetSrcWidth, targetSrcHeight),
                SkImage::RescaleGamma::kLinear,
                SkImage::RescaleMode::kRepeatedLinear);
        if (!scaledInput) {
            return nullptr;
        }

        // Calculate a scaled dstRect to match (0,0,targetSrcWidth,targetSrcHeight) as srcRect.
        SkIRect targetDstRect = SkRect::MakeXYWH((dstRect.left() - srcRect.left()) * sx,
                                                 (dstRect.top() - srcRect.top()) * sy,
                                                 dstRect.width()*sx,
                                                 dstRect.height()*sy).roundOut();
        SkIRect targetSrcRect = SkIRect::MakeWH(targetSrcWidth, targetSrcHeight);
        // Blur with pinned sigmas. If the sigma was less than the max, that axis of the image was
        // not scaled so we can use the original. If it was greater than the max, the scale factor
        // should have taken it the max supported sigma (ignoring the effect of rounding out the
        // source bounds).
        auto scaledOutput = blur_impl(
                recorder,
                {std::min(sigma.width(), kMaxLinearBlurSigma),
                 std::min(sigma.height(), kMaxLinearBlurSigma)},
                SkSpecialImages::MakeGraphite(recorder,
                                              targetSrcRect,
                                              std::move(scaledInput),
                                              outProps),
                targetSrcRect,
                tileMode,
                targetDstRect,
                outCS,
                outProps);
        if (!scaledOutput) {
            return nullptr;
        }

        // TODO: Pass out the upscaling transform for skif::FilterResult to hold on to.
        auto scaledOutputImage = scaledOutput->asImage();
        auto outputImage = RescaleImage(
                recorder,
                scaledOutputImage.get(),
                scaledOutput->subset(),
                scaledOutputImage->imageInfo().makeWH(dstRect.width(), dstRect.height()),
                SkImage::RescaleGamma::kLinear,
                SkImage::RescaleMode::kLinear);
        if (!outputImage) {
            return nullptr;
        }

        SkIRect outputDstRect = outputImage->bounds();
        return SkSpecialImages::MakeGraphite(recorder,
                                             outputDstRect,
                                             std::move(outputImage),
                                             outProps);
    }
}

// This class is the lazy instantiation callback for promise images. It manages calling the
// client's Fulfill, ImageRelease, and TextureRelease procs.
class PromiseLazyInstantiateCallback {
public:
    PromiseLazyInstantiateCallback(sk_sp<RefCntedCallback> releaseHelper,
                                   GraphitePromiseTextureFulfillProc fulfillProc,
                                   GraphitePromiseTextureFulfillContext fulfillContext,
                                   GraphitePromiseTextureReleaseProc textureReleaseProc)
            : fReleaseHelper(std::move(releaseHelper))
            , fFulfillProc(fulfillProc)
            , fFulfillContext(fulfillContext)
            , fTextureReleaseProc(textureReleaseProc) {
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

        sk_sp<Texture> texture = resourceProvider->createWrappedTexture(backendTexture);
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

sk_sp<TextureProxy> MakePromiseImageLazyProxy(
        const Caps* caps,
        SkISize dimensions,
        TextureInfo textureInfo,
        Volatile isVolatile,
        sk_sp<RefCntedCallback> releaseHelper,
        GraphitePromiseTextureFulfillProc fulfillProc,
        GraphitePromiseTextureFulfillContext fulfillContext,
        GraphitePromiseTextureReleaseProc textureReleaseProc) {
    SkASSERT(!dimensions.isEmpty());
    SkASSERT(releaseHelper);

    if (!fulfillProc) {
        return nullptr;
    }

    PromiseLazyInstantiateCallback callback{std::move(releaseHelper), fulfillProc,
                                            fulfillContext, textureReleaseProc};
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

size_t ComputeSize(SkISize dimensions,
                   const TextureInfo& info) {

    SkTextureCompressionType compression = info.compressionType();

    size_t colorSize = 0;

    if (compression != SkTextureCompressionType::kNone) {
        colorSize =  SkCompressedFormatDataSize(compression,
                                                dimensions,
                                                info.mipmapped() == Mipmapped::kYes);
    } else {
        // TODO: Should we make sure the backends return zero here if the TextureInfo is for a
        // memoryless texture?
        size_t bytesPerPixel = info.bytesPerPixel();

        colorSize = (size_t)dimensions.width() * dimensions.height() * bytesPerPixel;
    }

    size_t finalSize = colorSize * info.numSamples();

    if (info.mipmapped() == Mipmapped::kYes) {
        finalSize += colorSize/3;
    }
    return finalSize;
}

sk_sp<Image> CopyAsDraw(Recorder* recorder,
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
    auto surface = Surface::MakeScratch(recorder,
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
    // And the image draw into `surface` is flushed when it goes out of scope
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

    // make a Surface matching dstInfo to rescale into
    SkSurfaceProps surfaceProps = {};
    sk_sp<SkSurface> dst = make_renderable_scratch_surface(recorder, dstInfo, "RescaleDstTexture",
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
        tempOutput = make_renderable_scratch_surface(recorder, gammaDstInfo,
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
            tempOutput = make_renderable_scratch_surface(recorder, nextInfo,
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
        Flush(scratchSurface);

        sk_sp<CopyTextureToTextureTask> copyTask = CopyTextureToTextureTask::Make(
                static_cast<const Surface*>(scratchSurface)->readSurfaceView().refProxy(),
                SkIRect::MakeSize(dstSize),
                texture,
                {0, 0},
                mipLevel);
        if (!copyTask) {
            return false;
        }
        recorder->priv().add(std::move(copyTask));

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
class GraphiteBackend : public Backend, private SkBlurEngine, private SkBlurEngine::Algorithm {
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
#if defined(GRAPHITE_USE_APPROX_FIT_FOR_FILTERS)
                                             SkBackingFit::kApprox,
#else
                                             SkBackingFit::kExact,
#endif
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

    // SkBlurEngine::Algorithm
    float maxSigma() const override {
        // TODO: When FilterResult handles rescaling externally, change this to
        // skgpu::kMaxLinearBlurSigma.
        return SK_ScalarInfinity;
    }

    bool supportsOnlyDecalTiling() const override { return false; }

    sk_sp<SkSpecialImage> blur(SkSize sigma,
                               sk_sp<SkSpecialImage> src,
                               const SkIRect& srcRect,
                               SkTileMode tileMode,
                               const SkIRect& dstRect) const override {
        TRACE_EVENT_INSTANT2("skia.gpu", "GaussianBlur", TRACE_EVENT_SCOPE_THREAD,
                             "sigmaX", sigma.width(), "sigmaY", sigma.height());

        SkColorSpace* cs = src->getColorSpace();
        return skgpu::graphite::blur_impl(fRecorder, sigma, std::move(src), srcRect, tileMode,
                                          dstRect, sk_ref_sp(cs), this->surfaceProps());
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
