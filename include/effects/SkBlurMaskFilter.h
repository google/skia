/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurMaskFilter_DEFINED
#define SkBlurMaskFilter_DEFINED

// we include this since our callers will need to at least be able to ref/unref
#include "SkMaskFilter.h"
#include "SkScalar.h"
#include "SkBlurTypes.h"

class SK_API SkBlurMaskFilter {
public:
    /**
     *  If radius > 0, return the corresponding sigma, else return 0. Use this to convert from the
     *  (legacy) idea of specify the blur "radius" to the standard notion of specifying its sigma.
     */
    static SkScalar ConvertRadiusToSigma(SkScalar radius);

    enum BlurFlags {
        kNone_BlurFlag              = 0x00,
        /** The blur layer's radius is not affected by transforms */
        kIgnoreTransform_BlurFlag   = 0x01,
        /** Use a smother, higher qulity blur algorithm */
        kHighQuality_BlurFlag       = 0x02,
        /** mask for all blur flags */
        kAll_BlurFlag               = 0x03
    };

    /** Create a blur maskfilter.
     *  @param style     The SkBlurStyle to use
     *  @param sigma     Standard deviation of the Gaussian blur to apply. Must be > 0.
     *  @param occluder  The rect for which no pixels need be drawn (b.c. it will be overdrawn
     *                   with some opaque object. This is just a hint which backends are free to
     *                   ignore.
     *  @param flags     Flags to use - defaults to none
     *  @return The new blur maskfilter
     */
    static sk_sp<SkMaskFilter> Make(SkBlurStyle style, SkScalar sigma,
                                    const SkRect& occluder, uint32_t flags = kNone_BlurFlag);

    static sk_sp<SkMaskFilter> Make(SkBlurStyle style, SkScalar sigma,
                                    uint32_t flags = kNone_BlurFlag) {
        return Make(style, sigma, SkRect::MakeEmpty(), flags);
    }

#ifdef SK_SUPPORT_LEGACY_EMBOSSMASKFILTER
    /** Create an emboss maskfilter
        @param blurSigma    standard deviation of the Gaussian blur to apply
                            before applying lighting (e.g. 3)
        @param direction    array of 3 scalars [x, y, z] specifying the direction of the light source
        @param ambient      0...1 amount of ambient light
        @param specular     coefficient for specular highlights (e.g. 8)
        @return the emboss maskfilter
    */
    static sk_sp<SkMaskFilter> MakeEmboss(SkScalar blurSigma, const SkScalar direction[3],
                                          SkScalar ambient, SkScalar specular);
#endif

    static const int kMaxDivisions = 6;

    // This method computes all the parameters for drawing a partially occluded nine-patched
    // blurred rrect mask:
    //   rrectToDraw - the integerized rrect to draw in the mask
    //   widthHeight - how large to make the mask (rrectToDraw will be centered in this coord sys)
    //   rectXs, rectYs - the x & y coordinates of the covering geometry lattice
    //   texXs, texYs - the texture coordinate at each point in rectXs & rectYs
    //   numXs, numYs - number of coordinates in the x & y directions
    //   skipMask - bit mask that contains a 1-bit whenever one of the cells is occluded
    // It returns true if 'devRRect' is nine-patchable
    static bool ComputeBlurredRRectParams(const SkRRect& srcRRect, const SkRRect& devRRect,
                                          const SkRect& occluder,
                                          SkScalar sigma, SkScalar xformedSigma,
                                          SkRRect* rrectToDraw,
                                          SkISize* widthHeight,
                                          SkScalar rectXs[kMaxDivisions],
                                          SkScalar rectYs[kMaxDivisions],
                                          SkScalar texXs[kMaxDivisions],
                                          SkScalar texYs[kMaxDivisions],
                                          int* numXs, int* numYs, uint32_t* skipMask);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

private:
    SkBlurMaskFilter(); // can't be instantiated
};

#endif
