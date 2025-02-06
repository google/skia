/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurMask_DEFINED
#define SkBlurMask_DEFINED

#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "src/core/SkMask.h"

#include <cstdint>

class SkRRect;
enum SkBlurStyle : int;
struct SkRect;

class SkBlurMask {
public:
    [[nodiscard]] static bool BlurRect(SkScalar sigma, SkMaskBuilder *dst, const SkRect &src,
                                       SkBlurStyle, SkIVector *margin = nullptr,
                                       SkMaskBuilder::CreateMode createMode =
                                           SkMaskBuilder::kComputeBoundsAndRenderImage_CreateMode);
    [[nodiscard]] static bool BlurRRect(SkScalar sigma, SkMaskBuilder *dst, const SkRRect &src,
                                        SkBlurStyle, SkIVector *margin = nullptr,
                                        SkMaskBuilder::CreateMode createMode =
                                            SkMaskBuilder::kComputeBoundsAndRenderImage_CreateMode);

    // forceQuality will prevent BoxBlur from falling back to the low quality approach when sigma
    // is very small -- this can be used predict the margin bump ahead of time without completely
    // replicating the internal logic.  This permits not only simpler caching of blurred results,
    // but also being able to predict precisely at what pixels the blurred profile of e.g. a
    // rectangle will lie.
    //
    // Calling details:
    // * calculate margin - if src.fImage is null, then this call only calculates the border.
    // * failure          - if src.fImage is not null, failure is signal with dst->fImage being
    //                      null.

    [[nodiscard]] static bool BoxBlur(SkMaskBuilder* dst,
                                      const SkMask& src,
                                      SkScalar sigma,
                                      SkBlurStyle style,
                                      SkIVector* margin = nullptr);

    // the "ground truth" blur does a gaussian convolution; it's slow
    // but useful for comparison purposes.
    [[nodiscard]] static bool BlurGroundTruth(SkScalar sigma,
                                              SkMaskBuilder* dst,
                                              const SkMask& src,
                                              SkBlurStyle,
                                              SkIVector* margin = nullptr);

    // If radius > 0, return the corresponding sigma, else return 0
    static SkScalar SK_SPI ConvertRadiusToSigma(SkScalar radius);
    // If sigma > 0.5, return the corresponding radius, else return 0
    static SkScalar SK_SPI ConvertSigmaToRadius(SkScalar sigma);

    /* Helper functions for analytic rectangle blurs */

    /** Look up the intensity of the (one dimnensional) blurred half-plane.
        @param profile The precomputed 1D blur profile; initialized by ComputeBlurProfile below.
        @param loc the location to look up; The lookup will clamp invalid inputs, but
                   meaningful data are available between 0 and blurred_width
        @param blurred_width The width of the final, blurred rectangle
        @param sharp_width The width of the original, unblurred rectangle.
    */
    static uint8_t ProfileLookup(const uint8_t* profile, int loc, int blurredWidth, int sharpWidth);

    /** Populate the profile of a 1D blurred halfplane.
        @param profile The 1D table to fill in
        @param size    Should be 6*sigma bytes
        @param sigma   The standard deviation of the gaussian blur kernel
    */
    static void ComputeBlurProfile(uint8_t* profile, int size, SkScalar sigma);

    /** Compute an entire scanline of a blurred step function.  This is a 1D helper that
        will produce both the horizontal and vertical profiles of the blurry rectangle.
        @param pixels Location to store the resulting pixel data; allocated and managed by caller
        @param profile Precomputed blur profile computed by ComputeBlurProfile above.
        @param width Size of the pixels array.
        @param sigma Standard deviation of the gaussian blur kernel used to compute the profile;
                     this implicitly gives the size of the pixels array.
    */

    static void ComputeBlurredScanline(uint8_t* pixels, const uint8_t* profile,
                                       unsigned int width, SkScalar sigma);
};

#endif
