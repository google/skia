/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurImageFilter.h"

#include <algorithm>

#include "SkArenaAlloc.h"
#include "SkAutoPixmapStorage.h"
#include "SkBitmap.h"
#include "SkColorData.h"
#include "SkImageFilterPriv.h"
#include "SkTFitsIn.h"
#include "SkGpuBlurUtils.h"
#include "SkNx.h"
#include "SkOpts.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTextureProxy.h"
#include "SkGr.h"
#endif

static constexpr double kPi = 3.14159265358979323846264338327950288;

class SkBlurImageFilterImpl final : public SkImageFilter {
public:
    SkBlurImageFilterImpl(SkScalar sigmaX,
                          SkScalar sigmaY,
                          sk_sp<SkImageFilter> input,
                          const CropRect* cropRect,
                          SkBlurImageFilter::TileMode tileMode);

    SkRect computeFastBounds(const SkRect&) const override;

protected:
    void flatten(SkWriteBuffer&) const override;
    sk_sp<SkSpecialImage> onFilterImage(SkSpecialImage* source, const Context&,
                                        SkIPoint* offset) const override;
    SkIRect onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                               MapDirection, const SkIRect* inputRect) const override;

private:
    SK_FLATTENABLE_HOOKS(SkBlurImageFilterImpl)

    typedef SkImageFilter INHERITED;
    friend class SkImageFilter;

#if SK_SUPPORT_GPU
    sk_sp<SkSpecialImage> gpuFilter(
            SkSpecialImage *source, SkVector sigma, const sk_sp<SkSpecialImage> &input,
            SkIRect inputBounds, SkIRect dstBounds, SkIPoint inputOffset,
            const OutputProperties& outProps, SkIPoint* offset) const;
#endif

    SkSize                      fSigma;
    SkBlurImageFilter::TileMode fTileMode;
};

void SkImageFilter::RegisterFlattenables() { SK_REGISTER_FLATTENABLE(SkBlurImageFilterImpl); }

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkBlurImageFilter::Make(SkScalar sigmaX, SkScalar sigmaY,
                                             sk_sp<SkImageFilter> input,
                                             const SkImageFilter::CropRect* cropRect,
                                             TileMode tileMode) {
    if (sigmaX < SK_ScalarNearlyZero && sigmaY < SK_ScalarNearlyZero && !cropRect) {
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
        tileMode = buffer.read32LE(SkBlurImageFilter::kLast_TileMode);
    }

    static_assert(SkBlurImageFilter::kLast_TileMode == 2, "CreateProc");

    return SkBlurImageFilter::Make(
          sigmaX, sigmaY, common.getInput(0), &common.cropRect(), tileMode);
}

void SkBlurImageFilterImpl::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeScalar(fSigma.fWidth);
    buffer.writeScalar(fSigma.fHeight);

    static_assert(SkBlurImageFilter::kLast_TileMode == 2, "flatten");
    SkASSERT(fTileMode <= SkBlurImageFilter::kLast_TileMode);

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

// This is defined by the SVG spec:
// https://drafts.fxtf.org/filter-effects/#feGaussianBlurElement
static int calculate_window(double sigma) {
    // NB 136 is the largest sigma that will not cause a buffer full of 255 mask values to overflow
    // using the Gauss filter. It also limits the size of buffers used hold intermediate values.
    // Explanation of maximums:
    //   sum0 = window * 255
    //   sum1 = window * sum0 -> window * window * 255
    //   sum2 = window * sum1 -> window * window * window * 255 -> window^3 * 255
    //
    //   The value window^3 * 255 must fit in a uint32_t. So,
    //      window^3 < 2^32. window = 255.
    //
    //   window = floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5)
    //   For window <= 255, the largest value for sigma is 136.
    sigma = SkTPin(sigma, 0.0, 136.0);
    auto possibleWindow = static_cast<int>(floor(sigma * 3 * sqrt(2 * kPi) / 4 + 0.5));
    return std::max(1, possibleWindow);
}

// Calculating the border is tricky. The border is the distance in pixels between the first dst
// pixel and the first src pixel (or the last src pixel and the last dst pixel).
// I will go through the odd case which is simpler, and then through the even case. Given a
// stack of filters seven wide for the odd case of three passes.
//
//        S
//     aaaAaaa
//     bbbBbbb
//     cccCccc
//        D
//
// The furthest changed pixel is when the filters are in the following configuration.
//
//                 S
//           aaaAaaa
//        bbbBbbb
//     cccCccc
//        D
//
//  The A pixel is calculated using the value S, the B uses A, and the C uses B, and
// finally D is C. So, with a window size of seven the border is nine. In the odd case, the
// border is 3*((window - 1)/2).
//
// For even cases the filter stack is more complicated. The spec specifies two passes
// of even filters and a final pass of odd filters. A stack for a width of six looks like
// this.
//
//       S
//    aaaAaa
//     bbBbbb
//    cccCccc
//       D
//
// The furthest pixel looks like this.
//
//               S
//          aaaAaa
//        bbBbbb
//    cccCccc
//       D
//
// For a window of six, the border value is eight. In the even case the border is 3 *
// (window/2) - 1.
static int calculate_border(int window) {
    return (window & 1) == 1 ? 3 * ((window - 1) / 2) : 3 * (window / 2) - 1;
}

static int calculate_buffer(int window) {
    int bufferSize = window - 1;
    return (window & 1) == 1 ? 3 * bufferSize : 3 * bufferSize + 1;
}

// blur_one_direction implements the common three pass box filter approximation of Gaussian blur,
// but combines all three passes into a single pass. This approach is facilitated by three circular
// buffers the width of the window which track values for trailing edges of each of the three
// passes. This allows the algorithm to use more precision in the calculation because the values
// are not rounded each pass. And this implementation also avoids a trap that's easy to fall
// into resulting in blending in too many zeroes near the edge.
//
//  In general, a window sum has the form:
//     sum_n+1 = sum_n + leading_edge - trailing_edge.
//  If instead we do the subtraction at the end of the previous iteration, we can just
// calculate the sums instead of having to do the subtractions too.
//
//      In previous iteration:
//      sum_n+1 = sum_n - trailing_edge.
//
//      In this iteration:
//      sum_n+1 = sum_n + leading_edge.
//
//  Now we can stack all three sums and do them at once. Sum0 gets its leading edge from the
// actual data. Sum1's leading edge is just Sum0, and Sum2's leading edge is Sum1. So, doing the
// three passes at the same time has the form:
//
//    sum0_n+1 = sum0_n + leading edge
//    sum1_n+1 = sum1_n + sum0_n+1
//    sum2_n+1 = sum2_n + sum1_n+1
//
//    sum2_n+1 / window^3 is the new value of the destination pixel.
//
//    Reduce the sums by the trailing edges which were stored in the circular buffers,
// for the next go around. This is the case for odd sized windows, even windows the the third
// circular buffer is one larger then the first two circular buffers.
//
//    sum2_n+2 = sum2_n+1 - buffer2[i];
//    buffer2[i] = sum1;
//    sum1_n+2 = sum1_n+1 - buffer1[i];
//    buffer1[i] = sum0;
//    sum0_n+2 = sum0_n+1 - buffer0[i];
//    buffer0[i] = leading edge
//
//   This is all encapsulated in the processValue function below.
//
using Pass0And1 = Sk4u[2];
// The would be dLeft parameter is assumed to be 0.
static void blur_one_direction(Sk4u* buffer, int window,
                               int srcLeft, int srcRight, int dstRight,
                               const uint32_t* src, int srcXStride, int srcYStride, int srcH,
                                     uint32_t* dst, int dstXStride, int dstYStride) {

    // The circular buffers are one less than the window.
    auto pass0Count = window - 1,
         pass1Count = window - 1,
         pass2Count = (window & 1) == 1 ? window - 1 : window;

    Pass0And1* buffer01Start = (Pass0And1*)buffer;
    Sk4u*      buffer2Start  = buffer + pass0Count + pass1Count;
    Pass0And1* buffer01End   = (Pass0And1*)buffer2Start;
    Sk4u*      buffer2End    = buffer2Start + pass2Count;

    // If the window is odd then the divisor is just window ^ 3 otherwise,
    // it is window * window * (window + 1) = window ^ 3 + window ^ 2;
    auto window2 = window * window;
    auto window3 = window2 * window;
    auto divisor = (window & 1) == 1 ? window3 : window3 + window2;

    // NB the sums in the blur code use the following technique to avoid
    // adding 1/2 to round the divide.
    //
    //   Sum/d + 1/2 == (Sum + h) / d
    //   Sum + d(1/2) ==  Sum + h
    //     h == (1/2)d
    //
    // But the d/2 it self should be rounded.
    //    h == d/2 + 1/2 == (d + 1) / 2
    //
    // weight = 1 / d * 2 ^ 32
    auto weight = static_cast<uint32_t>(round(1.0 / divisor * (1ull << 32)));
    auto half = static_cast<uint32_t>((divisor + 1) / 2);

    auto border = calculate_border(window);

    // Calculate the start and end of the source pixels with respect to the destination start.
    auto srcStart = srcLeft - border,
         srcEnd   = srcRight - border,
         dstEnd   = dstRight;

    for (auto y = 0; y < srcH; y++) {
        auto buffer01Cursor = buffer01Start;
        auto buffer2Cursor  = buffer2Start;

        Sk4u sum0{0u};
        Sk4u sum1{0u};
        Sk4u sum2{half};

        sk_bzero(buffer01Start, (buffer2End - (Sk4u *) (buffer01Start)) * sizeof(*buffer2Start));

        // Given an expanded input pixel, move the window ahead using the leadingEdge value.
        auto processValue = [&](const Sk4u& leadingEdge) -> Sk4u {
            sum0 += leadingEdge;
            sum1 += sum0;
            sum2 += sum1;

            Sk4u value = sum2.mulHi(weight);

            sum2 -= *buffer2Cursor;
            *buffer2Cursor = sum1;
            buffer2Cursor = (buffer2Cursor + 1) < buffer2End ? buffer2Cursor + 1 : buffer2Start;

            sum1 -= (*buffer01Cursor)[1];
            (*buffer01Cursor)[1] = sum0;
            sum0 -= (*buffer01Cursor)[0];
            (*buffer01Cursor)[0] = leadingEdge;
            buffer01Cursor =
                    (buffer01Cursor + 1) < buffer01End ? buffer01Cursor + 1 : buffer01Start;

            return value;
        };

        auto srcIdx = srcStart;
        auto dstIdx = 0;
        const uint32_t* srcCursor = src;
              uint32_t* dstCursor = dst;

        // The destination pixels are not effected by the src pixels,
        // change to zero as per the spec.
        // https://drafts.fxtf.org/filter-effects/#FilterPrimitivesOverviewIntro
        while (dstIdx < srcIdx) {
            *dstCursor = 0;
            dstCursor += dstXStride;
            SK_PREFETCH(dstCursor);
            dstIdx++;
        }

        // The edge of the source is before the edge of the destination. Calculate the sums for
        // the pixels before the start of the destination.
        while (dstIdx > srcIdx) {
            Sk4u leadingEdge = srcIdx < srcEnd ? SkNx_cast<uint32_t>(Sk4b::Load(srcCursor)) : 0;
            (void) processValue(leadingEdge);
            srcCursor += srcXStride;
            srcIdx++;
        }

        // The dstIdx and srcIdx are in sync now; the code just uses the dstIdx for both now.
        // Consume the source generating pixels to dst.
        auto loopEnd = std::min(dstEnd, srcEnd);
        while (dstIdx < loopEnd) {
            Sk4u leadingEdge = SkNx_cast<uint32_t>(Sk4b::Load(srcCursor));
            SkNx_cast<uint8_t>(processValue(leadingEdge)).store(dstCursor);
            srcCursor += srcXStride;
            dstCursor += dstXStride;
            SK_PREFETCH(dstCursor);
            dstIdx++;
        }

        // The leading edge is beyond the end of the source. Assume that the pixels
        // are now 0x0000 until the end of the destination.
        loopEnd = dstEnd;
        while (dstIdx < loopEnd) {
            SkNx_cast<uint8_t>(processValue(0u)).store(dstCursor);
            dstCursor += dstXStride;
            SK_PREFETCH(dstCursor);
            dstIdx++;
        }

        src += srcYStride;
        dst += dstYStride;
    }
}

static sk_sp<SkSpecialImage> copy_image_with_bounds(
        SkSpecialImage *source, const sk_sp<SkSpecialImage> &input,
        SkIRect srcBounds, SkIRect dstBounds) {
    SkBitmap inputBM;
    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if (inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    SkBitmap src;
    inputBM.extractSubset(&src, srcBounds);

    // Make everything relative to the destination bounds.
    srcBounds.offset(-dstBounds.x(), -dstBounds.y());
    dstBounds.offset(-dstBounds.x(), -dstBounds.y());

    auto srcW = srcBounds.width(),
         dstW = dstBounds.width(),
         dstH = dstBounds.height();

    SkImageInfo dstInfo = SkImageInfo::Make(dstW, dstH, inputBM.colorType(), inputBM.alphaType());

    SkBitmap dst;
    if (!dst.tryAllocPixels(dstInfo)) {
        return nullptr;
    }

    // There is no blurring to do, but we still need to copy the source while accounting for the
    // dstBounds. Remember that the src was intersected with the dst.
    int y = 0;
    size_t dstWBytes = dstW * sizeof(uint32_t);
    for (;y < srcBounds.top(); y++) {
        sk_bzero(dst.getAddr32(0, y), dstWBytes);
    }

    for (;y < srcBounds.bottom(); y++) {
        int x = 0;
        uint32_t* dstPtr = dst.getAddr32(0, y);
        for (;x < srcBounds.left(); x++) {
            *dstPtr++ = 0;
        }

        memcpy(dstPtr, src.getAddr32(x - srcBounds.left(), y - srcBounds.top()),
               srcW * sizeof(uint32_t));

        dstPtr += srcW;
        x += srcW;

        for (;x < dstBounds.right(); x++) {
            *dstPtr++ = 0;
        }
    }

    for (;y < dstBounds.bottom(); y++) {
        sk_bzero(dst.getAddr32(0, y), dstWBytes);
    }

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(dstBounds.width(),
                                                          dstBounds.height()),
                                          dst, &source->props());
}

// TODO: Implement CPU backend for different fTileMode.
static sk_sp<SkSpecialImage> cpu_blur(
        SkVector sigma,
        SkSpecialImage *source, const sk_sp<SkSpecialImage> &input,
        SkIRect srcBounds, SkIRect dstBounds) {
    auto windowW = calculate_window(sigma.x()),
         windowH = calculate_window(sigma.y());

    if (windowW <= 1 && windowH <= 1) {
        return copy_image_with_bounds(source, input, srcBounds, dstBounds);
    }

    SkBitmap inputBM;

    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if (inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    SkBitmap src;
    inputBM.extractSubset(&src, srcBounds);

    // Make everything relative to the destination bounds.
    srcBounds.offset(-dstBounds.x(), -dstBounds.y());
    dstBounds.offset(-dstBounds.x(), -dstBounds.y());

    auto srcW = srcBounds.width(),
         srcH = srcBounds.height(),
         dstW = dstBounds.width(),
         dstH = dstBounds.height();

    SkImageInfo dstInfo = inputBM.info().makeWH(dstW, dstH);

    SkBitmap dst;
    if (!dst.tryAllocPixels(dstInfo)) {
        return nullptr;
    }

    auto bufferSizeW = calculate_buffer(windowW),
         bufferSizeH = calculate_buffer(windowH);

    // The amount 1024 is enough for buffers up to 10 sigma. The tmp bitmap will be
    // allocated on the heap.
    SkSTArenaAlloc<1024> alloc;
    Sk4u* buffer = alloc.makeArrayDefault<Sk4u>(std::max(bufferSizeW, bufferSizeH));

    // Basic Plan: The three cases to handle
    // * Horizontal and Vertical - blur horizontally while copying values from the source to
    //     the destination. Then, do an in-place vertical blur.
    // * Horizontal only - blur horizontally copying values from the source to the destination.
    // * Vertical only - blur vertically copying values from the source to the destination.

    // Default to vertical only blur case. If a horizontal blur is needed, then these values
    // will be adjusted while doing the horizontal blur.
    auto intermediateSrc = static_cast<uint32_t *>(src.getPixels());
    auto intermediateRowBytesAsPixels = src.rowBytesAsPixels();
    auto intermediateWidth = srcW;

    // Because the border is calculated before the fork of the GPU/CPU path. The border is
    // the maximum of the two rendering methods. In the case where sigma is zero, then the
    // src and dst left values are the same. If sigma is small resulting in a window size of
    // 1, then border calculations add some pixels which will always be zero. Inset the
    // destination by those zero pixels. This case is very rare.
    auto intermediateDst = dst.getAddr32(srcBounds.left(), 0);

    // The following code is executed very rarely, I have never seen it in a real web
    // page. If sigma is small but not zero then shared GPU/CPU border calculation
    // code adds extra pixels for the border. Just clear everything to clear those pixels.
    // This solution is overkill, but very simple.
    if (windowW == 1 || windowH == 1) {
        dst.eraseColor(0);
    }

    if (windowW > 1) {
        // Make int64 to avoid overflow in multiplication below.
        int64_t shift = srcBounds.top() - dstBounds.top();

        // For the horizontal blur, starts part way down in anticipation of the vertical blur.
        // For a vertical sigma of zero shift should be zero. But, for small sigma,
        // shift may be > 0 but the vertical window could be 1.
        intermediateSrc = static_cast<uint32_t *>(dst.getPixels())
                          + (shift > 0 ? shift * dst.rowBytesAsPixels() : 0);
        intermediateRowBytesAsPixels = dst.rowBytesAsPixels();
        intermediateWidth = dstW;
        intermediateDst = static_cast<uint32_t *>(dst.getPixels());

        blur_one_direction(
                buffer, windowW,
                srcBounds.left(), srcBounds.right(), dstBounds.right(),
                static_cast<uint32_t *>(src.getPixels()), 1, src.rowBytesAsPixels(), srcH,
                intermediateSrc, 1, intermediateRowBytesAsPixels);
    }

    if (windowH > 1) {
        blur_one_direction(
                buffer, windowH,
                srcBounds.top(), srcBounds.bottom(), dstBounds.bottom(),
                intermediateSrc, intermediateRowBytesAsPixels, 1, intermediateWidth,
                intermediateDst, dst.rowBytesAsPixels(), 1);
    }

    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(dstBounds.width(),
                                                          dstBounds.height()),
                                          dst, &source->props());
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

        result = this->gpuFilter(source, sigma, input, inputBounds, dstBounds, inputOffset,
                                 ctx.outputProperties(), &resultOffset);
    } else
#endif
    {
        result = cpu_blur(sigma, source, input, inputBounds, dstBounds);
    }

    // Return the resultOffset if the blur succeeded.
    if (result != nullptr) {
        *offset = resultOffset;
    }
    return result;
}

#if SK_SUPPORT_GPU
sk_sp<SkSpecialImage> SkBlurImageFilterImpl::gpuFilter(
        SkSpecialImage *source, SkVector sigma, const sk_sp<SkSpecialImage> &input,
        SkIRect inputBounds, SkIRect dstBounds, SkIPoint inputOffset,
        const OutputProperties& outProps, SkIPoint* offset) const
{
    if (0 == sigma.x() && 0 == sigma.y()) {
        offset->fX = inputBounds.x() + inputOffset.fX;
        offset->fY = inputBounds.y() + inputOffset.fY;
        return input->makeSubset(inputBounds);
    }

    auto context = source->getContext();

    sk_sp<GrTextureProxy> inputTexture(input->asTextureProxyRef(context));
    if (!inputTexture) {
        return nullptr;
    }

    sk_sp<GrRenderTargetContext> renderTargetContext(SkGpuBlurUtils::GaussianBlur(
                            context,
                            std::move(inputTexture),
                            outProps.colorSpace() ? sk_ref_sp(input->getColorSpace()) : nullptr,
                            dstBounds,
                            inputBounds,
                            sigma.x(),
                            sigma.y(),
                            to_texture_domain_mode(fTileMode),
                            input->alphaType()));
    if (!renderTargetContext) {
        return nullptr;
    }

    return SkSpecialImage::MakeDeferredFromGpu(
            context,
            SkIRect::MakeWH(dstBounds.width(), dstBounds.height()),
            kNeedNewImageUniqueID_SpecialImage,
            renderTargetContext->asTextureProxyRef(),
            sk_ref_sp(input->getColorSpace()),
            &source->props());
}
#endif

SkRect SkBlurImageFilterImpl::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    bounds.outset(fSigma.width() * 3, fSigma.height() * 3);
    return bounds;
}

SkIRect SkBlurImageFilterImpl::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                  MapDirection, const SkIRect* inputRect) const {
    SkVector sigma = map_sigma(fSigma, ctm);
    return src.makeOutset(SkScalarCeilToInt(sigma.x() * 3), SkScalarCeilToInt(sigma.y() * 3));
}
