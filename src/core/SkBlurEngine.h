/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurEngine_DEFINED
#define SkBlurEngine_DEFINED

#include "include/core/SkM44.h"  // IWYU pragma: keep
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/private/base/SkFloatingPoint.h"

#include <algorithm>
#include <array>
#include <cmath>

class SkDevice;
class SkRuntimeEffect;
class SkRuntimeEffectBuilder;
class SkSpecialImage;
struct SkImageInfo;
struct SkIRect;

enum class SkFilterMode;
enum class SkTileMode;
enum SkColorType : int;

/**
 * SkBlurEngine is a backend-agnostic provider of blur algorithms. Each Skia backend defines a blur
 * engine with a set of supported algorithms and/or implementations. A given implementation may be
 * optimized for a particular color type, sigma range, or available hardware. Each engine and its
 * algorithms are assumed to operate only on SkImages corresponding to its Skia backend, and will
 * produce output SkImages of the same type.
 *
 * Algorithms are allowed to specify a maximum supported sigma. If the desired sigma is higher than
 * this, the input image and output region must be downscaled by the caller before invoking the
 * algorithm. This is to provide the most flexibility for input representation (e.g. directly
 * rasterize at half resolution or apply deferred filter effects during the first downsample pass).
 *
 * skif::FilterResult::Builder::blur() is a convenient wrapper around the blur engine and
 * automatically handles resizing.
*/
class SkBlurEngine {
public:
    class Algorithm;

    virtual ~SkBlurEngine() = default;

    // Returns an Algorithm ideal for the requested 'sigma' that will support sampling an image of
    // the given 'colorType'. If the engine does not support the requested configuration, it returns
    // null. The engine maintains the lifetime of its algorithms, so the returned non-null
    // Algorithms live as long as the engine does.
    virtual const Algorithm* findAlgorithm(SkSize sigma,
                                           SkColorType colorType) const = 0;

    // TODO: Consolidate common utility functions from SkBlurMask.h into this header.

    // Any sigmas smaller than this are effectively an identity blur so can skip convolution at a
    // higher level. The value was chosen because it corresponds roughly to a radius of 1/10px, and
    // because 2*sigma^2 is slightly greater than SK_ScalarNearlyZero.
    static constexpr bool IsEffectivelyIdentity(float sigma) { return sigma <= 0.03f; }

    // Convert from a sigma Gaussian standard deviation to a pixel radius such that pixels outside
    // the radius would have an insignificant contribution to the final blurred value.
    static int SigmaToRadius(float sigma) {
        // sk_float_ceil2int is not constexpr
        return IsEffectivelyIdentity(sigma) ? 0 : sk_float_ceil2int(3.f * sigma);
    }

    // Get the default CPU-backed SkBlurEngine. This has specialized algorithms for 32-bit RGBA
    // and BGRA colors, and A8 alpha-only images when the sigma is large enough. For small blurs
    // and other color types, it uses SkShaderBlurAlgorithm backed by the raster pipeline.
    static const SkBlurEngine* GetRasterBlurEngine();

    // TODO: These are internal functions of the raster blur engine but need to be public for legacy
    // code paths to invoke them directly.

    // Calculate the successive box blur window for a given sigma. This is defined by the SVG spec:
    // https://drafts.fxtf.org/filter-effects/#feGaussianBlurElement
    //
    // NOTE: The successive box blur approximation is too inaccurate for cases where sigma < 2,
    // which works out to a window size of 4. If the window is smaller than this on both axes, the
    // successive box blur should not be used. If only one axis is this small, assume the
    // inaccuracies are hidden to avoid having to mix a shader-based blur and a box blur.
    static int BoxBlurWindow(float sigma) {
        int possibleWindow = sk_float_floor2int(sigma * 3 * sqrt(2 * SK_FloatPI) / 4 + 0.5f);
        return std::max(1, possibleWindow);
    }

    // TODO: Bring in anything needed for the single-channel box blur from SkMaskBlurFilter
};

class SkBlurEngine::Algorithm {
public:
    virtual ~Algorithm() = default;

    // The maximum sigma that can be passed to blur() in the X and/or Y sigma values. Larger
    // requested sigmas must manually downscale the input image and upscale the output image.
    virtual float maxSigma() const = 0;

    // Whether or not the SkTileMode can be passed to blur() must be SkTileMode::kDecal, or if any
    // tile mode is supported. If only kDecal is supported, then callers must manually apply the
    // tilemode and account for that in the src and dst bounds passed into blur(). If this returns
    // false, then the algorithm supports all SkTileModes.
    // TODO: Once CPU blurs support all tile modes, this API can go away.
    virtual bool supportsOnlyDecalTiling() const = 0;

    // Produce a blurred image that fills 'dstRect' (their dimensions will match). 'dstRect's top
    // left corner defines the output's location relative to the 'src' image. 'srcRect' restricts
    // the pixels that are included in the blur and is also relative to 'src'. The 'tileMode'
    // applies to the boundary of 'srcRect', which must be contained within 'src's dimensions.
    //
    // 'srcRect' and 'dstRect' may be different sizes and even be disjoint.
    //
    // The returned SkImage will have the same color type and colorspace as the input image. It will
    // be an SkImage type matching the underlying Skia backend. If the 'src' SkImage is not a
    // compatible SkImage type, null is returned.
    // TODO(b/299474380): This only takes SkSpecialImage to work with skif::FilterResult and
    // SkDevice::snapSpecial(); SkImage would be ideal.
    virtual sk_sp<SkSpecialImage> blur(SkSize sigma,
                                       sk_sp<SkSpecialImage> src,
                                       const SkIRect& srcRect,
                                       SkTileMode tileMode,
                                       const SkIRect& dstRect) const = 0;
};

/**
 * The default blur implementation uses internal runtime effects to evaluate either a single 2D
 * kernel within a shader, or performs two 1D blur passes. This algorithm is backend agnostic but
 * must be subclassed per backend to define the SkDevice creation function.
 */
class SkShaderBlurAlgorithm : public SkBlurEngine::Algorithm {
public:
    float maxSigma() const override { return kMaxLinearSigma; }
    bool supportsOnlyDecalTiling() const override { return false; }

    sk_sp<SkSpecialImage> blur(SkSize sigma,
                               sk_sp<SkSpecialImage> src,
                               const SkIRect& srcRect,
                               SkTileMode tileMode,
                               const SkIRect& dstRect) const override;

private:
    // Create a new surface, which can be approx-fit and have undefined contents.
    virtual sk_sp<SkDevice> makeDevice(const SkImageInfo&) const = 0;

    sk_sp<SkSpecialImage> renderBlur(SkRuntimeEffectBuilder* blurEffectBuilder,
                                     SkFilterMode filter,
                                     SkISize radii,
                                     sk_sp<SkSpecialImage> input,
                                     const SkIRect& srcRect,
                                     SkTileMode tileMode,
                                     const SkIRect& dstRect) const;
    sk_sp<SkSpecialImage> evalBlur2D(SkSize sigma,
                                     SkISize radii,
                                     sk_sp<SkSpecialImage> input,
                                     const SkIRect& srcRect,
                                     SkTileMode tileMode,
                                     const SkIRect& dstRect) const;
    sk_sp<SkSpecialImage> evalBlur1D(float sigma,
                                     int radius,
                                     SkV2 dir,
                                     sk_sp<SkSpecialImage> input,
                                     SkIRect srcRect,
                                     SkTileMode tileMode,
                                     SkIRect dstRect) const;

// TODO: These are internal details of the blur shaders, but are public for now because multiple
// backends invoke the blur shaders directly. Once everything just goes through this class, these
// can be hidden.
public:

    // The kernel width of a Gaussian blur of the given pixel radius, when all pixels are sampled.
    static constexpr int KernelWidth(int radius) { return 2 * radius + 1; }

    // The kernel width of a Gaussian blur of the given pixel radius, that relies on HW bilinear
    // filtering to combine adjacent pixels.
    static constexpr int LinearKernelWidth(int radius) { return radius + 1; }

    // The maximum sigma that can be computed without downscaling is based on the number of uniforms
    // and texture samples the effects will make in a single pass. For 1D passes, the number of
    // samples is equal to `LinearKernelWidth`; for 2D passes, it is equal to
    // `KernelWidth(radiusX)*KernelWidth(radiusY)`. This maps back to different maximum sigmas
    // depending on the approach used, as well as the ratio between the sigmas for the X and Y axes
    // if a 2D blur is performed.
    static constexpr int kMaxSamples = 28;

    // TODO(b/297393474): Update max linear sigma to 9; it had been 4 when a full 1D kernel was
    // used, but never updated after the linear filtering optimization reduced the number of
    // sample() calls required. Keep it at 4 for now to better isolate performance changes due to
    // switching to a runtime effect and constant loop structure.
    static constexpr float kMaxLinearSigma = 4.f; // -> radius = 27 -> linear kernel width = 28
    // NOTE: There is no defined kMaxBlurSigma for direct 2D blurs since it is entirely dependent on
    // the ratio between the two axes' sigmas, but generally it will be small on the order of a
    // 5x5 kernel.

    // Return a runtime effect that applies a 2D Gaussian blur in a single pass. The returned effect
    // can perform arbitrarily sized blur kernels so long as the kernel area is less than
    // kMaxSamples. An SkRuntimeEffect is returned to give flexibility for callers to convert it to
    // an SkShader or a GrFragmentProcessor. Callers are responsible for providing the uniform
    // values (using the appropriate API of the target effect type). The effect declares the
    // following uniforms:
    //
    //    uniform half4  kernel[7];
    //    uniform half4  offsets[14];
    //    uniform shader child;
    //
    // 'kernel' should be set to the output of Compute2DBlurKernel(). 'offsets' should be set to the
    // output of Compute2DBlurOffsets() with the same 'radii' passed to this function. 'child'
    // should be bound to whatever input is intended to be blurred, and can use nearest-neighbor
    // sampling (assuming it's an image).
    static const SkRuntimeEffect* GetBlur2DEffect(const SkISize& radii);

    // Return a runtime effect that applies a 1D Gaussian blur, taking advantage of HW linear
    // interpolation to accumulate adjacent pixels with fewer samples. The returned effect can be
    // used for both X and Y axes by changing the 'dir' uniform value (see below). It can be used
    // for all 1D blurs such that BlurLinearKernelWidth(radius) is less than or equal to
    // kMaxSamples. Like GetBlur2DEffect(), the caller is free to convert this to an SkShader or a
    // GrFragmentProcessor and is responsible for assigning uniforms with the appropriate API. Its
    // uniforms are declared as:
    //
    //     uniform half4  offsetsAndKernel[14];
    //     uniform half2  dir;
    //     uniform int    radius;
    //     uniform shader child;
    //
    // 'offsetsAndKernel' should be set to the output of Compute1DBlurLinearKernel(). 'radius'
    // should match the radius passed to that function. 'dir' should either be the vector {1,0} or
    // {0,1} for X and Y axis passes, respectively. 'child' should be bound to whatever input is
    // intended to be blurred and must use linear sampling in order for the outer blur effect to
    // function correctly.
    static const SkRuntimeEffect* GetLinearBlur1DEffect(int radius);

    // Calculates a set of weights for a 2D Gaussian blur of the given sigma and radius. It is
    // assumed that the radius was from prior calls to BlurSigmaRadius(sigma.width()|height()) and
    // is passed in to avoid redundant calculations.
    //
    // The provided span is fully written. The kernel is stored in row-major order based on the
    // provided radius. Any remaining indices in the span are zero initialized. The span must have
    // at least KernelWidth(radius.width())*KernelWidth(radius.height()) elements.
    //
    // NOTE: These take spans because it can be useful to compute full kernels that are larger than
    // what is supported in the GPU effects.
    static void Compute2DBlurKernel(SkSize sigma,
                                    SkISize radius,
                                    SkSpan<float> kernel);

    // A convenience function that packs the kMaxBlurSample scalars into SkV4's to match the
    // required type of the uniforms in GetBlur2DEffect().
    static void Compute2DBlurKernel(SkSize sigma,
                                    SkISize radius,
                                    std::array<SkV4, kMaxSamples/4>& kernel);

    // A convenience for the 2D case where one dimension has a sigma of 0.
    static  void Compute1DBlurKernel(float sigma, int radius, SkSpan<float> kernel) {
        Compute2DBlurKernel(SkSize{sigma, 0.f}, SkISize{radius, 0}, kernel);
    }

    // Utility function to fill in 'offsets' for the effect returned by GetBlur2DEffect(). It
    // automatically fills in the elements beyond the kernel size with the last real offset to
    // maximize texture cache hits. Each offset is really an SkV2 but are packed into SkV4's to
    // match the uniform declaration, and are otherwise ordered row-major.
    static void Compute2DBlurOffsets(SkISize radius, std::array<SkV4, kMaxSamples/2>& offsets);

    // Calculates a set of weights and sampling offsets for a 1D blur that uses GPU hardware to
    // linearly combine two logical source pixel values. This assumes that 'radius' was from a prior
    // call to BlurSigmaRadius() and is passed in to avoid redundant calculations. To match std140
    // uniform packing, the offset and kernel weight for adjacent samples are packed into a single
    // SkV4 as {offset[2*i], kernel[2*i], offset[2*i+1], kernel[2*i+1]}
    //
    // The provided array is fully written to. The calculated values are written to indices 0
    // through LinearKernelWidth(radius), with any remaining indices zero initialized.
    //
    // NOTE: This takes an array of a constrained size because its main use is calculating uniforms
    // for an effect with a matching constraint. Knowing the size of the linear kernel means the
    // full kernel can be stored on the stack internally.
    static void Compute1DBlurLinearKernel(float sigma,
                                          int radius,
                                          std::array<SkV4, kMaxSamples/2>& offsetsAndKernel);

};

#endif // SkBlurEngine_DEFINED
