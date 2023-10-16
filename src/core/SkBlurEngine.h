/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurEngine_DEFINED
#define SkBlurEngine_DEFINED

#include "include/core/SkRefCnt.h"

class SkSpecialImage;
struct SkIRect;
struct SkSize;
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

    // Returns an Algorithm ideal for the requested 'sigma' that will support sampling the input
    // with 'tileMode' and the given 'colorType'. If the engine does not support the requested
    // configuration, it returns null. The engine maintains the lifetime of its algorithms, so the
    // returned non-null Algorithms live as long as the engine does.
    virtual const Algorithm* findAlgorithm(SkSize sigma,
                                           SkTileMode tileMode,
                                           SkColorType colorType) const = 0;

    // TODO: Consolidate common utility functions from SkBlurMask.h, skgpu::BlurUtils, and
    // skgpu::ganesh::GrBlurUtils into this header.

};

class SkBlurEngine::Algorithm {
public:
    virtual ~Algorithm() = default;

    // The maximum sigma that can be passed to blur() in the X and/or Y sigma values. Larger
    // requested sigmas must manually downscale the input image and upscale the output image.
    virtual float maxSigma() const = 0;

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

#endif // SkBlurEngine_DEFINED
