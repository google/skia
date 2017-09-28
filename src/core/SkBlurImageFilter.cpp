/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurImageFilter.h"

#include "SkAutoPixmapStorage.h"
#include "SkColorData.h"
#include "SkColorSpaceXformer.h"
#include "SkTFitsIn.h"
#include "SkGpuBlurUtils.h"
#include "SkOpts.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTextureProxy.h"
#include "SkGr.h"
#endif

class SkBlurImageFilterImpl final : public SkImageFilter {
public:
    SkBlurImageFilterImpl(SkScalar sigmaX,
                      SkScalar sigmaY,
                      sk_sp<SkImageFilter> input,
                      const CropRect* cropRect,
                      SkBlurImageFilter::TileMode tileMode);

    SkRect computeFastBounds(const SkRect&) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurImageFilterImpl)

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    sk_sp<SkImageFilter> onMakeColorSpace(SkColorSpaceXformer*) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix&, MapDirection) const override;

private:
    typedef SkImageFilter INHERITED;
    friend class SkImageFilter;

    #if SK_SUPPORT_GPU
    sk_sp<SkSpecialImage> gpuFilter(
            SkSpecialImage *source,
            SkVector sigma, const sk_sp<SkSpecialImage> &input,
            SkIRect inputBounds, SkIRect dstBounds) const;
    #endif

    sk_sp<SkSpecialImage> cpuFilter(
            SkSpecialImage *source,
            SkVector sigma, const sk_sp<SkSpecialImage> &input,
            SkIRect inputBounds, SkIRect dstBounds) const;

    SkSize                      fSigma;
    SkBlurImageFilter::TileMode fTileMode;
};

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkImageFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurImageFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkBlurImageFilter::Make(SkScalar sigmaX, SkScalar sigmaY,
                                             sk_sp<SkImageFilter> input,
                                             const SkImageFilter::CropRect* cropRect,
                                             TileMode tileMode) {
    if (0 == sigmaX && 0 == sigmaY && !cropRect) {
        return input;
    }
    return sk_sp<SkImageFilter>(
          new SkBlurImageFilterImpl(sigmaX, sigmaY, input, cropRect, tileMode));
}

// This rather arbitrary-looking value results in a maximum box blur kernel size
// of 1000 pixels on the raster path, which matches the WebKit and Firefox
// implementations. Since the GPU path does not compute a box blur, putting
// the limit on sigma ensures consistent behaviour between the GPU and
// raster paths.
#define MAX_SIGMA SkIntToScalar(532)

static SkVector map_sigma(const SkSize& localSigma, const SkMatrix& ctm) {
    SkVector sigma = SkVector::Make(localSigma.width(), localSigma.height());
    ctm.mapVectors(&sigma, 1);
    sigma.fX = SkMinScalar(SkScalarAbs(sigma.fX), MAX_SIGMA);
    sigma.fY = SkMinScalar(SkScalarAbs(sigma.fY), MAX_SIGMA);
    return sigma;
}

SkBlurImageFilterImpl::SkBlurImageFilterImpl(SkScalar sigmaX,
                                             SkScalar sigmaY,
                                             sk_sp<SkImageFilter> input,
                                             const CropRect* cropRect,
                                             SkBlurImageFilter::TileMode tileMode)
        : INHERITED(&input, 1, cropRect), fSigma{sigmaX, sigmaY}, fTileMode(tileMode) {}

sk_sp<SkFlattenable> SkBlurImageFilterImpl::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkScalar sigmaX = buffer.readScalar();
    SkScalar sigmaY = buffer.readScalar();
    SkBlurImageFilter::TileMode tileMode;
    if (buffer.isVersionLT(SkReadBuffer::kTileModeInBlurImageFilter_Version)) {
        tileMode = SkBlurImageFilter::kClampToBlack_TileMode;
    } else {
        tileMode = static_cast<SkBlurImageFilter::TileMode>(buffer.readInt());
    }

    static_assert(SkBlurImageFilter::kMax_TileMode == 2, "CreateProc");
    SkASSERT(tileMode <= SkBlurImageFilter::kMax_TileMode);

    return SkBlurImageFilter::Make(
          sigmaX, sigmaY, common.getInput(0), &common.cropRect(), tileMode);
}

void SkBlurImageFilterImpl::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSigma.fWidth);
    buffer.writeScalar(fSigma.fHeight);

    static_assert(SkBlurImageFilter::kMax_TileMode == 2, "flatten");
    SkASSERT(fTileMode <= SkBlurImageFilter::kMax_TileMode);

    buffer.writeInt(static_cast<int>(fTileMode));
}

#if SK_SUPPORT_GPU
static GrTextureDomain::Mode to_texture_domain_mode(SkBlurImageFilter::TileMode tileMode) {
    switch (tileMode) {
      case SkBlurImageFilter::TileMode::kClamp_TileMode:
        return GrTextureDomain::kClamp_Mode;
      case SkBlurImageFilter::TileMode::kClampToBlack_TileMode:
        return GrTextureDomain::kDecal_Mode;
      case SkBlurImageFilter::TileMode::kRepeat_TileMode:
        return GrTextureDomain::kRepeat_Mode;
      default:
        SK_ABORT("Unsupported tile mode.");
        return GrTextureDomain::kDecal_Mode;
    }
}
#endif

static void get_box3_params(SkScalar s, int *kernelSize, int* kernelSize3, int *lowOffset,
                            int *highOffset) {
    float pi = SkScalarToFloat(SK_ScalarPI);
    int d = static_cast<int>(floorf(SkScalarToFloat(s) * 3.0f * sqrtf(2.0f * pi) / 4.0f + 0.5f));
    *kernelSize = d;
    if (d % 2 == 1) {
        *lowOffset = *highOffset = (d - 1) / 2;
        *kernelSize3 = d;
    } else {
        *highOffset = d / 2;
        *lowOffset = *highOffset - 1;
        *kernelSize3 = d + 1;
    }
}

sk_sp<SkSpecialImage> SkBlurImageFilterImpl::onFilterImage(SkSpecialImage* source,
                                                           const Context& ctx,
                                                           SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);

    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    SkIRect inputBounds = SkIRect::MakeXYWH(inputOffset.fX, inputOffset.fY,
                                            input->width(), input->height());

    // Calculate the destination bounds.
    SkIRect dstBounds;
    if (!this->applyCropRect(this->mapContext(ctx), inputBounds, &dstBounds)) {
        return nullptr;
    }
    if (!inputBounds.intersect(dstBounds)) {
        return nullptr;
    }

    // Save the offset in preparation to make all rectangles relative to the inputOffset.
    SkIPoint resultOffset = SkIPoint::Make(dstBounds.fLeft, dstBounds.fTop);

    // Make all bounds relative to the inputOffset.
    inputBounds.offset(-inputOffset);
    dstBounds.offset(-inputOffset);

    const SkVector sigma = map_sigma(fSigma, ctx.ctm());
    if (sigma.x() < 0 || sigma.y() < 0) {
        return nullptr;
    }

    sk_sp<SkSpecialImage> result;
#if SK_SUPPORT_GPU
    if (source->isTextureBacked()) {
        // Ensure the input is in the destination's gamut. This saves us from having to do the
        // xform during the filter itself.
        input = ImageToColorSpace(input.get(), ctx.outputProperties());

        result = this->gpuFilter(source, sigma, input, inputBounds, dstBounds);
    } else
    #endif
    {
        #if defined(SK_SUPPORT_LEGACY_BLUR_IMAGE)
        result = this->cpuFilter(source, sigma, input, inputBounds, dstBounds);
        #else
        // The new code will go here.
        result = this->cpuFilter(source, sigma, input, inputBounds, dstBounds);
        #endif
    }

    // Return the resultOffset if the blur succeeded.
    if (result != nullptr) {
        *offset = resultOffset;
    }
    return result;
}

#if SK_SUPPORT_GPU
sk_sp<SkSpecialImage> SkBlurImageFilterImpl::gpuFilter(
        SkSpecialImage *source,
        SkVector sigma, const sk_sp<SkSpecialImage> &input,
        SkIRect inputBounds, SkIRect dstBounds) const
{
    // If both sigmas produce arms of the cross that are less than 1/2048, then they
    // do not contribute to the sum of the filter in a way to change a gamma corrected result.
    // Let s = 1/(2*sigma^2)
    // The normalizing value   n = 1 + 4*E^(-s) + 4*E^(-2s)
    // The raw cross arm value c = E^-s
    // The normalized cross arm value = c/n
    // N[Solve[{c/n == 1/2048, sigma > 0}, sigma], 16]
    static constexpr double kCrossTooSmall = 0.2561130112451658;
    if (sigma.x() < kCrossTooSmall && sigma.y() < kCrossTooSmall) {
        return input->makeSubset(inputBounds);
    }

    GrContext* context = source->getContext();

    sk_sp<GrTextureProxy> inputTexture(input->asTextureProxyRef(context));
    if (!inputTexture) {
        return nullptr;
    }

    // Typically, we would create the RTC with the output's color space (from ctx), but we
    // always blur in the PixelConfig of the *input*. Those might not be compatible (if they
    // have different transfer functions). We've already guaranteed that those color spaces
    // have the same gamut, so in this case, we do everything in the input's color space.
    sk_sp<GrRenderTargetContext> renderTargetContext(SkGpuBlurUtils::GaussianBlur(
        context,
        std::move(inputTexture),
        sk_ref_sp(input->getColorSpace()),
        dstBounds,
        inputBounds,
        sigma.x(),
        sigma.y(),
        to_texture_domain_mode(fTileMode)));
    if (!renderTargetContext) {
        return nullptr;
    }

    return SkSpecialImage::MakeDeferredFromGpu(context,
                                               SkIRect::MakeWH(dstBounds.width(),
                                                               dstBounds.height()),
                                               kNeedNewImageUniqueID_SpecialImage,
                                               renderTargetContext->asTextureProxyRef(),
                                               renderTargetContext->refColorSpace(),
                                               &source->props());
}
#endif

// TODO: Implement CPU backend for different fTileMode.
sk_sp<SkSpecialImage> SkBlurImageFilterImpl::cpuFilter(
        SkSpecialImage *source,
        SkVector sigma, const sk_sp<SkSpecialImage> &input,
        SkIRect inputBounds, SkIRect dstBounds) const
{
    // If both sigmas will result in a zero width window, there is nothing to do.
    // N[Solve[sigma*3*Sqrt[2 Pi]/4 == 1/2, sigma], 16]
    static constexpr double kZeroWindow = 0.2659615202676218;
    if (sigma.x() < kZeroWindow && sigma.y() < kZeroWindow) {
        return input->makeSubset(inputBounds);
    }

    int kernelSizeX, kernelSizeX3, lowOffsetX, highOffsetX;
    int kernelSizeY, kernelSizeY3, lowOffsetY, highOffsetY;
    get_box3_params(sigma.x(), &kernelSizeX, &kernelSizeX3, &lowOffsetX, &highOffsetX);
    get_box3_params(sigma.y(), &kernelSizeY, &kernelSizeY3, &lowOffsetY, &highOffsetY);

    SkBitmap inputBM;

    if (!input->getROPixels(&inputBM) && inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    SkImageInfo info = SkImageInfo::Make(dstBounds.width(), dstBounds.height(),
                                         inputBM.colorType(), inputBM.alphaType());

    SkBitmap tmp, dst;
    if (!tmp.tryAllocPixels(info) || !dst.tryAllocPixels(info)) {
        return nullptr;
    }

    // Get ready to blur.
    const SkPMColor* s = inputBM.getAddr32(inputBounds.x(), inputBounds.y());
          SkPMColor* t = tmp.getAddr32(0, 0);
          SkPMColor* d = dst.getAddr32(0, 0);

    // Shift everything from being relative to the orignal input bounds to the destination bounds.
    inputBounds.offset(-dstBounds.x(), -dstBounds.y());
    dstBounds.offset(-dstBounds.x(), -dstBounds.y());

    int w  = dstBounds.width(),
        h  = dstBounds.height(),
        sw = inputBM.rowBytesAsPixels();

    SkIRect inputBoundsT = SkIRect::MakeLTRB(inputBounds.top(), inputBounds.left(),
                                             inputBounds.bottom(), inputBounds.right());
    SkIRect dstBoundsT = SkIRect::MakeWH(dstBounds.height(), dstBounds.width());

    /**
     *
     * In order to make memory accesses cache-friendly, we reorder the passes to
     * use contiguous memory reads wherever possible.
     *
     * For example, the 6 passes of the X-and-Y blur case are rewritten as
     * follows. Instead of 3 passes in X and 3 passes in Y, we perform
     * 2 passes in X, 1 pass in X transposed to Y on write, 2 passes in X,
     * then 1 pass in X transposed to Y on write.
     *
     * +----+       +----+       +----+        +---+       +---+       +---+        +----+
     * + AB + ----> | AB | ----> | AB | -----> | A | ----> | A | ----> | A | -----> | AB |
     * +----+ blurX +----+ blurX +----+ blurXY | B | blurX | B | blurX | B | blurXY +----+
     *                                         +---+       +---+       +---+
     *
     * In this way, two of the y-blurs become x-blurs applied to transposed
     * images, and all memory reads are contiguous.
     */
    if (kernelSizeX > 0 && kernelSizeY > 0) {
        SkOpts::box_blur_xx(s, sw,  inputBounds,  t, kernelSizeX,  lowOffsetX,  highOffsetX, w, h);
        SkOpts::box_blur_xx(t,  w,  dstBounds,    d, kernelSizeX,  highOffsetX, lowOffsetX,  w, h);
        SkOpts::box_blur_xy(d,  w,  dstBounds,    t, kernelSizeX3, highOffsetX, highOffsetX, w, h);
        SkOpts::box_blur_xx(t,  h,  dstBoundsT,   d, kernelSizeY,  lowOffsetY,  highOffsetY, h, w);
        SkOpts::box_blur_xx(d,  h,  dstBoundsT,   t, kernelSizeY,  highOffsetY, lowOffsetY,  h, w);
        SkOpts::box_blur_xy(t,  h,  dstBoundsT,   d, kernelSizeY3, highOffsetY, highOffsetY, h, w);
    } else if (kernelSizeX > 0) {
        SkOpts::box_blur_xx(s, sw,  inputBounds,  d, kernelSizeX,  lowOffsetX,  highOffsetX, w, h);
        SkOpts::box_blur_xx(d,  w,  dstBounds,    t, kernelSizeX,  highOffsetX, lowOffsetX,  w, h);
        SkOpts::box_blur_xx(t,  w,  dstBounds,    d, kernelSizeX3, highOffsetX, highOffsetX, w, h);
    } else if (kernelSizeY > 0) {
        SkOpts::box_blur_yx(s, sw,  inputBoundsT, d, kernelSizeY,  lowOffsetY,  highOffsetY, h, w);
        SkOpts::box_blur_xx(d,  h,  dstBoundsT,   t, kernelSizeY,  highOffsetY, lowOffsetY,  h, w);
        SkOpts::box_blur_xy(t,  h,  dstBoundsT,   d, kernelSizeY3, highOffsetY, highOffsetY, h, w);
    }

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeSize(dstBounds.size()),
                                          dst, &source->props());
}

sk_sp<SkImageFilter> SkBlurImageFilterImpl::onMakeColorSpace(SkColorSpaceXformer* xformer)
const {
    SkASSERT(1 == this->countInputs());

    auto input = xformer->apply(this->getInput(0));
    if (this->getInput(0) != input.get()) {
        return SkBlurImageFilter::Make(fSigma.width(), fSigma.height(), std::move(input),
                                       this->getCropRectIfSet(), fTileMode);
    }
    return this->refMe();
}

SkRect SkBlurImageFilterImpl::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    bounds.outset(fSigma.width() * 3, fSigma.height() * 3);
    return bounds;
}

SkIRect SkBlurImageFilterImpl::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                              MapDirection) const {
    SkVector sigma = map_sigma(fSigma, ctm);
    return src.makeOutset(SkScalarCeilToInt(sigma.x() * 3), SkScalarCeilToInt(sigma.y() * 3));
}

#ifndef SK_IGNORE_TO_STRING
void SkBlurImageFilterImpl::toString(SkString* str) const {
    str->appendf("SkBlurImageFilterImpl: (");
    str->appendf("sigma: (%f, %f) tileMode: %d input (", fSigma.fWidth, fSigma.fHeight,
                 static_cast<int>(fTileMode));

    if (this->getInput(0)) {
        this->getInput(0)->toString(str);
    }

    str->append("))");
}
#endif
