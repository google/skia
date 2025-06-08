/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/imagefilters/SkMatrixConvolutionImageFilter.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkM44.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkMath.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkWriteBuffer.h"

#include <cstdint>
#include <cstring>
#include <optional>
#include <utility>

using namespace skia_private;
using namespace MatrixConvolutionImageFilter;

namespace {

static_assert(kLargeKernelSize % 4 == 0, "Must be a multiple of 4");
static_assert(kSmallKernelSize <= kLargeKernelSize, "Small kernel size must be <= max size");
// The uniform array storing the kernel is packed into half4's so that we don't waste space
// forcing array elements out to 16-byte alignment when using std140.
static_assert(kMaxUniformKernelSize % 4 == 0, "Must be a multiple of 4");

SkBitmap create_kernel_bitmap(const SkISize& kernelSize, const float* kernel,
                              float* innerGain, float* innerBias);

class SkMatrixConvolutionImageFilter final : public SkImageFilter_Base {
public:
    SkMatrixConvolutionImageFilter(const SkISize& kernelSize, const SkScalar* kernel,
                                   SkScalar gain, SkScalar bias, const SkIPoint& kernelOffset,
                                   bool convolveAlpha, sk_sp<SkImageFilter> const* input)
            : SkImageFilter_Base(input, 1)
            , fKernel(kernel, kernelSize.width() * kernelSize.height())
            , fKernelSize(kernelSize)
            , fKernelOffset({kernelOffset.fX, kernelOffset.fY})
            , fGain(gain)
            , fBias(bias)
            , fConvolveAlpha(convolveAlpha) {
        // The public factory should have ensured these before creating this object.
        SkASSERT(SkSafeMath::Mul(kernelSize.fWidth, kernelSize.fHeight) <= kLargeKernelSize);
        SkASSERT(kernelSize.fWidth >= 1 && kernelSize.fHeight >= 1);
        SkASSERT(kernelOffset.fX >= 0 && kernelOffset.fX < kernelSize.fWidth);
        SkASSERT(kernelOffset.fY >= 0 && kernelOffset.fY < kernelSize.fHeight);

        // Does nothing for small kernels, otherwise encodes kernel into an A8 image.
        fKernelBitmap = create_kernel_bitmap(kernelSize, kernel, &fInnerGain, &fInnerBias);
    }

    SkRect computeFastBounds(const SkRect& bounds) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterMatrixConvolutionImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkMatrixConvolutionImageFilter)

    bool onAffectsTransparentBlack() const override {
        // affectsTransparentBlack() is conflated with "canComputeFastBounds" and MatrixConvolution
        // is unique in that it might not produce unbounded output, but we can't calculate the
        // fast bounds because the kernel is applied in device space and no transform is provided
        // with that API.
        // TODO(skbug.com/40045519): Accept a matrix in computeFastBounds() so that we can handle the
        // layer-space kernel case.

        // That issue aside, a matrix convolution can affect transparent black when it has a
        // non-zero bias and convolves alpha (if it doesn't convolve the alpha channel then the bias
        // applied to RGB doesn't matter for transparent black pixels).
        // NOTE: The crop image filters that wrap the matrix convolution to apply tile modes will
        // reset this property when possible.
        return true;
    }

    skif::FilterResult onFilterImage(const skif::Context& context) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    // Helper functions to adjust 'bounds' by the kernel size and offset, either for what would be
    // sampled when covering 'bounds', or what could produce values when applied to 'bounds'.
    skif::LayerSpace<SkIRect> boundsSampledByKernel(const skif::LayerSpace<SkIRect>& bounds) const;
    skif::LayerSpace<SkIRect> boundsAffectedByKernel(const skif::LayerSpace<SkIRect>& bounds) const;

    sk_sp<SkShader> createShader(const skif::Context& ctx, sk_sp<SkShader> input) const;

    // Original kernel data, preserved for serialization even if it was encoded into fKernelBitmap
    TArray<float> fKernel;

    // Unlike the majority of image filters, the kernel is applied as-is to the layer-space pixels.
    // This means that the kernel size and offset are always in the layer coordinate system.
    skif::LayerSpace<SkISize>  fKernelSize;
    skif::LayerSpace<skif::IVector> fKernelOffset;

    float fGain;
    float fBias; // NOTE: This is assumed to be in [0-255] for historical reasons
    bool  fConvolveAlpha;

    // Derived from fKernel when larger than what we will upload as uniforms; fInnerBias and
    // fInnerGain  reconstruct the original coefficient from unorm8 data as (a+innerBias)*innerGain
    // Since these are derived, they are not serialized.
    SkBitmap fKernelBitmap;
    float fInnerBias;
    float fInnerGain;
};

// LayerSpace doesn't have a clean type to represent 4 separate edge deltas, but the result
// is a valid layer-space rectangle, so just go back to the underlying SkIRect temporarily.
skif::LayerSpace<SkIRect> adjust(const skif::LayerSpace<SkIRect>& rect,
                                 int dl, int dt, int dr, int db) {
    SkIRect adjusted = SkIRect(rect);
    adjusted.adjust(dl, dt, dr, db);
    return skif::LayerSpace<SkIRect>(adjusted);
}

std::pair<int, SkKnownRuntimeEffects::StableKey> quantize_by_kernel_size(int kernelSize) {
    if (kernelSize < kMaxUniformKernelSize) {
        return { kMaxUniformKernelSize, SkKnownRuntimeEffects::StableKey::kMatrixConvUniforms };
    } else if (kernelSize <= kSmallKernelSize) {
        return { kSmallKernelSize, SkKnownRuntimeEffects::StableKey::kMatrixConvTexSm };
    }

    return { kLargeKernelSize, SkKnownRuntimeEffects::StableKey::kMatrixConvTexLg };
}

SkBitmap create_kernel_bitmap(const SkISize& kernelSize, const float* kernel,
                              float* innerGain, float* innerBias) {
    int length = kernelSize.fWidth * kernelSize.fHeight;
    auto [quantizedKernelSize, key] = quantize_by_kernel_size(length);
    if (key == SkKnownRuntimeEffects::StableKey::kMatrixConvUniforms) {
        // No bitmap is needed to store the kernel on the GPU
        *innerGain = 1.f;
        *innerBias = 0.f;
        return {};
    }


    // The convolution kernel is "big". The SVG spec has no upper limit on what's supported so
    // store the kernel in a SkBitmap that will be uploaded to a data texture. We could
    // implement a more straight forward evaluation loop for the CPU backend, but kernels of
    // this size are already going to be very slow so we accept the extra indirection to
    // keep the code paths consolidated.
    //
    // We store the data in A8 for universal support, but this requires normalizing the values
    // and adding an extra inner bias operation to the shader. We could store values in A16 or
    // A32 for improved accuracy but that would require querying GPU capabilities, which
    // prevents creating the bitmap once during initialization. Even on the GPU, kernels larger
    // than 5x5 quickly exceed realtime capabilities, so the loss of precision isn't a great
    // concern either.
    float min = kernel[0];
    float max = kernel[0];
    for (int i = 1; i < length; ++i) {
        if (kernel[i] < min) {
            min = kernel[i];
        }
        if (kernel[i] > max) {
            max = kernel[i];
        }
    }

    *innerGain = max - min;
    *innerBias = min;
    // Treat a near-0 gain (i.e. box blur) as 1 and let innerBias move everything to final value.
    if (SkScalarNearlyZero(*innerGain)) {
        *innerGain = 1.f;
    }

    SkBitmap kernelBM;
    if (!kernelBM.tryAllocPixels(SkImageInfo::Make({ quantizedKernelSize, 1 },
                                                   kAlpha_8_SkColorType,
                                                   kPremul_SkAlphaType))) {
        // OOM so return an empty bitmap, which will be detected later on in onFilterImage().
        return {};
    }

    for (int i = 0; i < length; ++i) {
        *kernelBM.getAddr8(i, 0) = SkScalarRoundToInt(255 * (kernel[i] - min) / *innerGain);
    }
    for (int i = length; i < quantizedKernelSize; ++i) {
        *kernelBM.getAddr8(i, 0) = 0;
    }

    kernelBM.setImmutable();
    return kernelBM;
}

} // anonymous namespace

sk_sp<SkImageFilter> SkImageFilters::MatrixConvolution(const SkISize& kernelSize,
                                                       const SkScalar kernel[],
                                                       SkScalar gain,
                                                       SkScalar bias,
                                                       const SkIPoint& kernelOffset,
                                                       SkTileMode tileMode,
                                                       bool convolveAlpha,
                                                       sk_sp<SkImageFilter> input,
                                                       const CropRect& cropRect) {
    if (kernelSize.width() < 1 || kernelSize.height() < 1) {
        return nullptr;
    }
    if (SkSafeMath::Mul(kernelSize.width(), kernelSize.height()) > kLargeKernelSize) {
        return nullptr;
    }
    if (!kernel) {
        return nullptr;
    }
    if ((kernelOffset.fX < 0) || (kernelOffset.fX >= kernelSize.fWidth) ||
        (kernelOffset.fY < 0) || (kernelOffset.fY >= kernelSize.fHeight)) {
        return nullptr;
    }

    // The 'tileMode' behavior is not well-defined if there is no crop, so we only apply it if
    // there is a provided 'cropRect'.
    sk_sp<SkImageFilter> filter = std::move(input);
    if (cropRect && tileMode != SkTileMode::kDecal) {
        // Historically the input image was restricted to the cropRect when tiling was not kDecal
        // so that the kernel evaluated the tiled edge conditions, while a kDecal crop only affected
        // the output.
        filter = SkImageFilters::Crop(*cropRect, tileMode, std::move(filter));
    }
    filter = sk_sp<SkImageFilter>(new SkMatrixConvolutionImageFilter(
            kernelSize, kernel, gain, bias, kernelOffset, convolveAlpha, &filter));
    if (cropRect) {
        // But regardless of the tileMode, the output is decal cropped.
        filter = SkImageFilters::Crop(*cropRect, SkTileMode::kDecal, std::move(filter));
    }
    return filter;
}

void SkRegisterMatrixConvolutionImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkMatrixConvolutionImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkMatrixConvolutionImageFilterImpl",
                            SkMatrixConvolutionImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkMatrixConvolutionImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    SkISize kernelSize;
    kernelSize.fWidth = buffer.readInt();
    kernelSize.fHeight = buffer.readInt();
    const int count = buffer.getArrayCount();

    const int64_t kernelArea = sk_64_mul(kernelSize.width(), kernelSize.height());
    if (!buffer.validate(kernelArea == count)) {
        return nullptr;
    }
    if (!buffer.validateCanReadN<SkScalar>(count)) {
        return nullptr;
    }
    AutoSTArray<16, SkScalar> kernel(count);
    if (!buffer.readScalarArray(kernel)) {
        return nullptr;
    }
    SkScalar gain = buffer.readScalar();
    SkScalar bias = buffer.readScalar();
    SkIPoint kernelOffset;
    kernelOffset.fX = buffer.readInt();
    kernelOffset.fY = buffer.readInt();

    SkTileMode tileMode = SkTileMode::kDecal;
    if (buffer.isVersionLT(SkPicturePriv::kConvolutionImageFilterTilingUpdate)) {
        tileMode = buffer.read32LE(SkTileMode::kLastTileMode);
    } // else SkCropImageFilter handles the tile mode (if any)

    bool convolveAlpha = buffer.readBool();

    if (!buffer.isValid()) {
        return nullptr;
    }
    // NOTE: For SKPs with version >= kConvolutionImageFilterTilingUpdate, tileMode will be kDecal
    // and common.cropRect() will be null (so the factory also ignores tileMode). Any
    // cropping/tiling will have been handled by the deserialized input/output Crop image filters.
    return SkImageFilters::MatrixConvolution(
                kernelSize, kernel.get(), gain, bias, kernelOffset, tileMode,
                convolveAlpha, common.getInput(0), common.cropRect());
}

void SkMatrixConvolutionImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeInt(fKernelSize.width());
    buffer.writeInt(fKernelSize.height());
    buffer.writeScalarArray(fKernel);
    buffer.writeScalar(fGain);
    buffer.writeScalar(fBias);
    buffer.writeInt(fKernelOffset.x());
    buffer.writeInt(fKernelOffset.y());
    buffer.writeBool(fConvolveAlpha);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::LayerSpace<SkIRect> SkMatrixConvolutionImageFilter::boundsSampledByKernel(
        const skif::LayerSpace<SkIRect>& bounds) const {
    return adjust(bounds,
                  -fKernelOffset.x(),
                  -fKernelOffset.y(),
                  fKernelSize.width() - fKernelOffset.x() - 1,
                  fKernelSize.height() - fKernelOffset.y() - 1);
}

skif::LayerSpace<SkIRect> SkMatrixConvolutionImageFilter::boundsAffectedByKernel(
        const skif::LayerSpace<SkIRect>& bounds) const {
    return adjust(bounds,
                  fKernelOffset.x() - fKernelSize.width() + 1,
                  fKernelOffset.y() - fKernelSize.height() + 1,
                  fKernelOffset.x(),
                  fKernelOffset.y());
}



sk_sp<SkShader> SkMatrixConvolutionImageFilter::createShader(const skif::Context& ctx,
                                                             sk_sp<SkShader> input) const {
    const int kernelLength = fKernelSize.width() * fKernelSize.height();
    auto [_, key] = quantize_by_kernel_size(kernelLength);
    const bool useTextureShader = (key != SkKnownRuntimeEffects::StableKey::kMatrixConvUniforms);
    if (useTextureShader && fKernelBitmap.empty()) {
        return nullptr; // No actual kernel data to work with from a prior OOM
    }

    const SkRuntimeEffect* matrixConvEffect = GetKnownRuntimeEffect(key);

    SkRuntimeShaderBuilder builder(sk_ref_sp(matrixConvEffect));
    builder.child("child") = std::move(input);

    if (useTextureShader) {
        sk_sp<SkImage> cachedKernel = ctx.backend()->getCachedBitmap(fKernelBitmap);
        if (!cachedKernel) {
            return nullptr;
        }
        builder.child("kernel") = cachedKernel->makeRawShader(SkFilterMode::kNearest);
        builder.uniform("innerGainAndBias") = SkV2{fInnerGain, fInnerBias};
    } else {
        float paddedKernel[kMaxUniformKernelSize];
        memcpy(paddedKernel, fKernel.data(), kernelLength*sizeof(float));
        memset(paddedKernel+kernelLength, 0, (kMaxUniformKernelSize - kernelLength)*sizeof(float));

        builder.uniform("kernel").set(paddedKernel, kMaxUniformKernelSize);
    }

    builder.uniform("size") = SkISize(fKernelSize);
    builder.uniform("offset") = skif::IVector(fKernelOffset);
    // Scale the user-provided bias by 1/255 to match the [0,1] color channel range
    builder.uniform("gainAndBias") = SkV2{fGain, fBias / 255.f};
    builder.uniform("convolveAlpha") = fConvolveAlpha ? 1 : 0;

    return builder.makeShader();
}

skif::FilterResult SkMatrixConvolutionImageFilter::onFilterImage(
        const skif::Context& context) const {
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    skif::LayerSpace<SkIRect> requiredInput = this->boundsSampledByKernel(context.desiredOutput());
    skif::FilterResult childOutput =
            this->getChildOutput(0, context.withNewDesiredOutput(requiredInput));

    skif::LayerSpace<SkIRect> outputBounds;
    if (fConvolveAlpha && fBias != 0.f) {
        // The convolution will produce a non-trivial value for every pixel so fill desired output.
        outputBounds = context.desiredOutput();
    } else {
        // Calculate the possible extent of the convolution given what was actually produced by the
        // child filter and then intersect that with the desired output.
        outputBounds = this->boundsAffectedByKernel(childOutput.layerBounds());
        if (!outputBounds.intersect(context.desiredOutput())) {
            return {};
        }
    }

    skif::FilterResult::Builder builder{context};
    builder.add(childOutput,
                this->boundsSampledByKernel(outputBounds),
                ShaderFlags::kSampledRepeatedly);
    return builder.eval([&](SkSpan<sk_sp<SkShader>> inputs) {
        return this->createShader(context, inputs[0]);
    }, outputBounds);
}

skif::LayerSpace<SkIRect> SkMatrixConvolutionImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // Adjust the desired output bounds by the kernel size to avoid evaluating edge conditions, and
    // then recurse to the child filter.
    skif::LayerSpace<SkIRect> requiredInput = this->boundsSampledByKernel(desiredOutput);
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

std::optional<skif::LayerSpace<SkIRect>> SkMatrixConvolutionImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    if (fConvolveAlpha && fBias != 0.f) {
        // Applying the kernel as a convolution to fully transparent black will result in 0 for
        // each channel, unless the bias itself shifts this "zero-point". However, when the alpha
        // channel is not convolved, the original a=0 is preserved and producing a premul color
        // discards the non-zero bias. Convolving the alpha channel and a non-zero bias can mean
        // the transparent black pixels outside of any input image become non-transparent black.
        return skif::LayerSpace<SkIRect>::Unbounded();
    }

    // Otherwise apply the kernel to the output bounds of the child filter.
    auto outputBounds = this->getChildOutputLayerBounds(0, mapping, contentBounds);
    if (outputBounds) {
        return this->boundsAffectedByKernel(*outputBounds);
    } else {
        return skif::LayerSpace<SkIRect>::Unbounded();
    }
}

SkRect SkMatrixConvolutionImageFilter::computeFastBounds(const SkRect& bounds) const {
    // See onAffectsTransparentBlack(), but without knowing the local-to-device transform, we don't
    // know how many pixels will be sampled by the kernel. Return unbounded to match the
    // expectations of an image filter that "affects" transparent black.
    return SkRectPriv::MakeLargeS32();
}
