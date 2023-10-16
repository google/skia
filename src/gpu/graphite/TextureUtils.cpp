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
#include "src/core/SkDevice.h"
#include "src/core/SkImageFilterCache.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkTraceEvent.h"
#include "src/image/SkImage_Base.h"

#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/GraphiteTypes.h"
#include "include/gpu/graphite/ImageProvider.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/gpu/BlurUtils.h"
#include "src/gpu/graphite/Buffer.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/CommandBuffer.h"
#include "src/gpu/graphite/CopyTask.h"
#include "src/gpu/graphite/Device.h"
#include "src/gpu/graphite/Image_Graphite.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/ResourceProvider.h"
#include "src/gpu/graphite/SpecialImage_Graphite.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/SynchronizeToCpuTask.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/UploadTask.h"

#include <array>

namespace {

sk_sp<SkSurface> make_surface_with_fallback(skgpu::graphite::Recorder* recorder,
                                            const SkImageInfo& info,
                                            skgpu::Mipmapped mipmapped,
                                            const SkSurfaceProps* surfaceProps) {
    SkColorType ct = recorder->priv().caps()->getRenderableColorType(info.colorType());
    if (ct == kUnknown_SkColorType) {
        return nullptr;
    }

    return SkSurfaces::RenderTarget(recorder, info.makeColorType(ct), mipmapped, surfaceProps);
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
    auto graphiteImage = static_cast<const skgpu::graphite::Image*>(clientProvided);
    if (graphiteImage->textureProxyView().origin() != skgpu::Origin::kTopLeft) {
        SKGPU_LOG_E("Client provided image must have a TopLeft origin.");
        return false;
    }

    return true;
}

sk_sp<SkSpecialImage> eval_blur(skgpu::graphite::Recorder* recorder,
                                sk_sp<SkShader> blurEffect,
                                const SkIRect& dstRect,
                                SkColorType colorType,
                                sk_sp<SkColorSpace> outCS,
                                const SkSurfaceProps& outProps) {
    SkImageInfo outII = SkImageInfo::Make({dstRect.width(), dstRect.height()},
                                          colorType, kPremul_SkAlphaType, std::move(outCS));
    auto device = skgpu::graphite::Device::Make(recorder,
                                                outII,
                                                skgpu::Budgeted::kYes,
                                                skgpu::Mipmapped::kNo,
                                                outProps,
                                                /*addInitialClear=*/false);
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

sk_sp<SkSpecialImage> blur_2d(skgpu::graphite::Recorder* recorder,
                              SkSize sigma,
                              SkISize radii,
                              sk_sp<SkSpecialImage> input,
                              const SkIRect& srcRect,
                              const SkIRect& dstRect,
                              sk_sp<SkColorSpace> outCS,
                              const SkSurfaceProps& outProps) {
    std::array<SkV4, skgpu::kMaxBlurSamples/4> kernel;
    std::array<SkV4, skgpu::kMaxBlurSamples/2> offsets;
    skgpu::Compute2DBlurKernel(sigma, radii, kernel);
    skgpu::Compute2DBlurOffsets(radii, offsets);

    SkRuntimeShaderBuilder builder{sk_ref_sp(skgpu::GetBlur2DEffect(radii))};
    builder.uniform("kernel") = kernel;
    builder.uniform("offsets") = offsets;
    // TODO(b/294102201): This is very much like FilterResult::asShader()...
    builder.child("child") =
            input->makeSubset(srcRect)->asShader(SkTileMode::kDecal,
                                                 SkFilterMode::kNearest,
                                                 SkMatrix::Translate(srcRect.left(),srcRect.top()));

    return eval_blur(recorder, builder.makeShader(), dstRect,
                     input->colorType(), std::move(outCS), outProps);
}

sk_sp<SkSpecialImage> blur_1d(skgpu::graphite::Recorder* recorder,
                              float sigma,
                              int radius,
                              SkV2 dir,
                              sk_sp<SkSpecialImage> input,
                              SkIRect srcRect,
                              SkIRect dstRect,
                              sk_sp<SkColorSpace> outCS,
                              const SkSurfaceProps& outProps) {
    std::array<SkV4, skgpu::kMaxBlurSamples/2> offsetsAndKernel;
    skgpu::Compute1DBlurLinearKernel(sigma, radius, offsetsAndKernel);

    SkRuntimeShaderBuilder builder{sk_ref_sp(skgpu::GetLinearBlur1DEffect(radius))};
    builder.uniform("offsetsAndKernel") = offsetsAndKernel;
    builder.uniform("dir") = dir;
    // TODO(b/294102201): This is very much like FilterResult::asShader()...
    builder.child("child") =
            input->makeSubset(srcRect)->asShader(SkTileMode::kDecal,
                                                 SkFilterMode::kLinear,
                                                 SkMatrix::Translate(srcRect.left(),srcRect.top()));

    return eval_blur(recorder, builder.makeShader(), dstRect,
                     input->colorType(), std::move(outCS), outProps);
}

sk_sp<SkSpecialImage> blur_impl(skgpu::graphite::Recorder* recorder,
                                SkSize sigma,
                                sk_sp<SkSpecialImage> input,
                                SkIRect srcRect,
                                SkIRect dstRect,
                                sk_sp<SkColorSpace> outCS,
                                const SkSurfaceProps& outProps) {
    // See if we can do a blur on the original resolution image
    if (sigma.width() <= skgpu::kMaxLinearBlurSigma &&
        sigma.height() <= skgpu::kMaxLinearBlurSigma) {
        int radiusX = skgpu::BlurSigmaRadius(sigma.width());
        int radiusY = skgpu::BlurSigmaRadius(sigma.height());
        const int kernelArea = skgpu::BlurKernelWidth(radiusX) * skgpu::BlurKernelWidth(radiusY);
        if (kernelArea <= skgpu::kMaxBlurSamples && radiusX > 0 && radiusY > 0) {
            // Use a single-pass 2D kernel if it fits and isn't just 1D already
            return blur_2d(recorder, sigma, {radiusX, radiusY}, std::move(input), srcRect, dstRect,
                           std::move(outCS), outProps);
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
                                std::move(input), srcRect, intermediateDstRect, outCS, outProps);
                if (!input) {
                    return nullptr;
                }
                srcRect = SkIRect::MakeWH(input->width(), input->height());
                dstRect.offset(-intermediateDstRect.left(), -intermediateDstRect.top());
            }

            if (radiusY > 0) {
                input = blur_1d(recorder, sigma.height(), radiusY, {0.f, 1.f},
                                std::move(input), srcRect, dstRect, outCS, outProps);
            }

            return input;
        }
    } else {
        // Rescale the source image, blur that with a reduced sigma, and then upscale back to the
        // dstRect dimensions.
        // TODO(b/294102201): Share rescaling logic with GrBlurUtils::GaussianBlur.
        float sx = sigma.width() > skgpu::kMaxLinearBlurSigma
                ? (skgpu::kMaxLinearBlurSigma / sigma.width()) : 1.f;
        float sy = sigma.height() > skgpu::kMaxLinearBlurSigma
                ? (skgpu::kMaxLinearBlurSigma / sigma.height()) : 1.f;

        int targetSrcWidth = sk_float_ceil2int(srcRect.width() * sx);
        int targetSrcHeight = sk_float_ceil2int(srcRect.height() * sy);

        auto inputImage = input->asImage();
        // TODO(b/288902559): Support approx fit backings for the target of a rescale
        // TODO(b/294102201): Be smarter about downscaling when there are actual tilemodes to apply
        // to the image.
        auto scaledInput = skgpu::graphite::RescaleImage(
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
                {std::min(sigma.width(), skgpu::kMaxLinearBlurSigma),
                 std::min(sigma.height(), skgpu::kMaxLinearBlurSigma)},
                SkSpecialImages::MakeGraphite(recorder,
                                              targetSrcRect,
                                              std::move(scaledInput),
                                              outProps),
                targetSrcRect,
                targetDstRect,
                outCS,
                outProps);
        if (!scaledOutput) {
            return nullptr;
        }

        // TODO: Pass out the upscaling transform for skif::FilterResult to hold on to.
        auto scaledOutputImage = scaledOutput->asImage();
        auto outputImage = skgpu::graphite::RescaleImage(
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

} // anonymous namespace

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

    const TextureProxyView& imageView = srcGraphiteImage->textureProxyView();
    if (!imageView.proxy()) {
        // With the current definition of SkImage, this shouldn't happen.
        // If we allow non-texturable formats for compute, we'll need to
        // copy to a texturable format.
        SkASSERT(false);
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

bool GenerateMipmaps(Recorder* recorder,
                     sk_sp<TextureProxy> texture,
                     const SkColorInfo& colorInfo) {
    constexpr SkSamplingOptions kSamplingOptions = SkSamplingOptions(SkFilterMode::kLinear);

    SkASSERT(texture->mipmapped() == Mipmapped::kYes);

    // Within a rescaling pass tempInput is read from and tempOutput is written to.
    // At the end of the pass tempOutput's texture is wrapped and assigned to tempInput.
    sk_sp<SkImage> tempInput(new Image(kNeedNewImageUniqueID,
                                       TextureProxyView(texture),
                                       colorInfo));
    sk_sp<SkSurface> tempOutput;

    SkISize srcSize = texture->dimensions();
    const SkColorInfo outColorInfo = colorInfo.makeAlphaType(kPremul_SkAlphaType);

    for (int mipLevel = 1; srcSize.width() > 1 || srcSize.height() > 1; ++mipLevel) {
        SkISize stepSize = SkISize::Make(1, 1);
        if (srcSize.width() > 1) {
            stepSize.fWidth = srcSize.width() / 2;
        }
        if (srcSize.height() > 1) {
            stepSize.fHeight = srcSize.height() / 2;
        }

        tempOutput = make_surface_with_fallback(recorder,
                                                SkImageInfo::Make(stepSize, outColorInfo),
                                                Mipmapped::kNo,
                                                nullptr);
        if (!tempOutput) {
            return false;
        }
        SkCanvas* stepDst = tempOutput->getCanvas();
        SkRect stepDstRect = SkRect::Make(stepSize);

        SkPaint paint;
        stepDst->drawImageRect(tempInput, SkRect::Make(srcSize), stepDstRect, kSamplingOptions,
                               &paint, SkCanvas::kStrict_SrcRectConstraint);

        // Make sure the rescaling draw finishes before copying the results.
        sk_sp<SkSurface> stepDstSurface = sk_ref_sp(stepDst->getSurface());
        skgpu::graphite::Flush(stepDstSurface);

        sk_sp<CopyTextureToTextureTask> copyTask = CopyTextureToTextureTask::Make(
                static_cast<const Surface*>(stepDstSurface.get())->readSurfaceView().refProxy(),
                SkIRect::MakeSize(stepSize),
                texture,
                {0, 0},
                mipLevel);
        if (!copyTask) {
            return false;
        }
        recorder->priv().add(std::move(copyTask));

        tempInput = SkSurfaces::AsImage(tempOutput);
        srcSize = stepSize;
    }

    return true;
}

std::pair<sk_sp<SkImage>, SkSamplingOptions> GetGraphiteBacked(Recorder* recorder,
                                                               const SkImage* imageIn,
                                                               SkSamplingOptions sampling) {
    skgpu::Mipmapped mipmapped = (sampling.mipmap != SkMipmapMode::kNone)
                                     ? skgpu::Mipmapped::kYes : skgpu::Mipmapped::kNo;

    if (imageIn->dimensions().area() <= 1 && mipmapped == skgpu::Mipmapped::kYes) {
        mipmapped = skgpu::Mipmapped::kNo;
        sampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
    }

    sk_sp<SkImage> result;
    if (as_IB(imageIn)->isGraphiteBacked()) {
        result = sk_ref_sp(imageIn);

        // If the preexisting Graphite-backed image doesn't have the required mipmaps we will drop
        // down the sampling
        if (mipmapped == skgpu::Mipmapped::kYes && !result->hasMipmaps()) {
            mipmapped = skgpu::Mipmapped::kNo;
            sampling = SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
        }
    } else {
        auto clientImageProvider = recorder->clientImageProvider();
        result = clientImageProvider->findOrCreate(
                recorder, imageIn, {mipmapped == skgpu::Mipmapped::kYes});

        if (!valid_client_provided_image(
                    result.get(), imageIn, {mipmapped == skgpu::Mipmapped::kYes})) {
            // The client did not fulfill the ImageProvider contract so drop the image.
            result = nullptr;
        }
    }

    if (sampling.isAniso() && result) {
        sampling = SkSamplingPriv::AnisoFallback(result->hasMipmaps());
    }

    return { result, sampling };
}

std::tuple<skgpu::graphite::TextureProxyView, SkColorType> AsView(Recorder* recorder,
                                                                  const SkImage* image,
                                                                  skgpu::Mipmapped mipmapped) {
    if (!recorder || !image) {
        return {};
    }

    if (!as_IB(image)->isGraphiteBacked()) {
        return {};
    }
    // TODO(b/238756380): YUVA not supported yet
    if (as_IB(image)->isYUVA()) {
        return {};
    }

    auto gi = reinterpret_cast<const skgpu::graphite::Image*>(image);

    if (gi->dimensions().area() <= 1) {
        mipmapped = skgpu::Mipmapped::kNo;
    }

    if (mipmapped == skgpu::Mipmapped::kYes &&
        gi->textureProxyView().proxy()->mipmapped() != skgpu::Mipmapped::kYes) {
        SKGPU_LOG_W("Graphite does not auto-generate mipmap levels");
        return {};
    }

    SkColorType ct = gi->colorType();
    return {gi->textureProxyView(), ct};
}

SkColorType ComputeShaderCoverageMaskTargetFormat(const Caps* caps) {
    // GPU compute coverage mask renderers need to bind the mask texture as a storage binding, which
    // support a limited set of color formats. In general, we use RGBA8 if Alpha8 can't be
    // supported.
    // TODO(chromium:1856): In particular, WebGPU does not support the "storage binding" usage for
    // the R8Unorm texture format.
    if (caps->isStorage(caps->getDefaultStorageTextureInfo(kAlpha_8_SkColorType))) {
        return kAlpha_8_SkColorType;
    }
    return kRGBA_8888_SkColorType;
}

} // namespace skgpu::graphite

namespace skif {

namespace {

class GraphiteBackend : public Backend {
public:

    GraphiteBackend(skgpu::graphite::Recorder* recorder,
                    const SkSurfaceProps& surfaceProps,
                    SkColorType colorType)
            : Backend(SkImageFilterCache::Create(SkImageFilterCache::kDefaultTransientSize),
                      surfaceProps, colorType)
            , fRecorder(recorder) {}

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
                                             props ? *props : this->surfaceProps(),
                                             /*addInitialClear=*/false);
    }

    sk_sp<SkSpecialImage> makeImage(const SkIRect& subset, sk_sp<SkImage> image) const override {
        return SkSpecialImages::MakeGraphite(fRecorder, subset, image, this->surfaceProps());
    }

    sk_sp<SkImage> getCachedBitmap(const SkBitmap& data) const override {
        auto proxy = skgpu::graphite::RecorderPriv::CreateCachedProxy(fRecorder, data);
        if (!proxy) {
            return nullptr;
        }

        const SkColorInfo& colorInfo = data.info().colorInfo();
        skgpu::Swizzle swizzle = fRecorder->priv().caps()->getReadSwizzle(colorInfo.colorType(),
                                                                          proxy->textureInfo());
        return sk_make_sp<skgpu::graphite::Image>(
                data.getGenerationID(),
                skgpu::graphite::TextureProxyView(std::move(proxy), swizzle),
                colorInfo);
    }

    sk_sp<SkSpecialImage> blur(SkSize sigma,
                               sk_sp<SkSpecialImage> input,
                               SkIRect srcRect,
                               SkIRect dstRect,
                               sk_sp<SkColorSpace> cs) const override {
        TRACE_EVENT_INSTANT2("skia.gpu", "GaussianBlur", TRACE_EVENT_SCOPE_THREAD,
                             "sigmaX", sigma.width(), "sigmaY", sigma.height());
        return blur_impl(fRecorder, sigma, std::move(input), srcRect, dstRect,
                         std::move(cs), this->surfaceProps());
    }

    bool isBlurSupported() const override {
        return true;
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
