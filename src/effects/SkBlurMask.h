
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBlurMask_DEFINED
#define SkBlurMask_DEFINED

#include "SkShader.h"
#include "SkMask.h"
#include "SkRRect.h"

class SkBlurMask {
public:
    enum Style {
        kNormal_Style,  //!< fuzzy inside and outside
        kSolid_Style,   //!< solid inside, fuzzy outside
        kOuter_Style,   //!< nothing inside, fuzzy outside
        kInner_Style,   //!< fuzzy inside, nothing outside

        kStyleCount
    };

    enum Quality {
        kLow_Quality,   //!< box blur
        kHigh_Quality   //!< three pass box blur (similar to gaussian)
    };

    static bool BlurRect(SkScalar sigma, SkMask *dst, const SkRect &src,
                         Style style,
                         SkIPoint *margin = NULL,
                         SkMask::CreateMode createMode =
                                                SkMask::kComputeBoundsAndRenderImage_CreateMode);
    static bool BlurRRect(SkScalar sigma, SkMask *dst, const SkRRect &src,
                         Style style,
                         SkIPoint *margin = NULL,
                         SkMask::CreateMode createMode =
                                                SkMask::kComputeBoundsAndRenderImage_CreateMode);
    static bool BoxBlur(SkMask* dst, const SkMask& src,
                        SkScalar sigma, Style style, Quality quality,
                        SkIPoint* margin = NULL);

    // the "ground truth" blur does a gaussian convolution; it's slow
    // but useful for comparison purposes.
    static bool BlurGroundTruth(SkScalar sigma, SkMask* dst, const SkMask& src,
                                Style style,
                                SkIPoint* margin = NULL);

    static SkScalar ConvertRadiusToSigma(SkScalar radius);

    /* Helper functions for analytic rectangle blurs */

    /** Look up the intensity of the (one dimnensional) blurred half-plane.
        @param profile The precomputed 1D blur profile; memory allocated by and managed by
                       ComputeBlurProfile below.
        @param loc the location to look up; The lookup will clamp invalid inputs, but
                   meaningful data are available between 0 and blurred_width
        @param blurred_width The width of the final, blurred rectangle
        @param sharp_width The width of the original, unblurred rectangle.
    */
    static uint8_t ProfileLookup(const uint8_t* profile, int loc, int blurred_width, int sharp_width);

    /** Allocate memory for and populate the profile of a 1D blurred halfplane.  The caller
        must free the memory.  The amount of memory allocated will be exactly 6*sigma bytes.
        @param sigma The standard deviation of the gaussian blur kernel
        @param profile_out The location to store the allocated profile curve
    */

    static void ComputeBlurProfile(SkScalar sigma, uint8_t** profile_out);

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
