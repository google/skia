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

class SK_API SkBlurMaskFilter {
public:
    enum BlurStyle {
        kNormal_BlurStyle,  //!< fuzzy inside and outside
        kSolid_BlurStyle,   //!< solid inside, fuzzy outside
        kOuter_BlurStyle,   //!< nothing inside, fuzzy outside
        kInner_BlurStyle,   //!< fuzzy inside, nothing outside

        kBlurStyleCount
    };

    enum BlurFlags {
        kNone_BlurFlag = 0x00,
        /** The blur layer's radius is not affected by transforms */
        kIgnoreTransform_BlurFlag   = 0x01,
        /** Use a smother, higher qulity blur algorithm */
        kHighQuality_BlurFlag       = 0x02,
        /** mask for all blur flags */
        kAll_BlurFlag = 0x03
    };

    /** Create a blur maskfilter.
        @param radius   The radius to extend the blur from the original mask. Must be > 0.
        @param style    The BlurStyle to use
        @param flags    Flags to use - defaults to none
        @return The new blur maskfilter
    */
    static SkMaskFilter* Create(SkScalar radius, BlurStyle style,
                                uint32_t flags = kNone_BlurFlag);

    /** Create an emboss maskfilter
        @param direction    array of 3 scalars [x, y, z] specifying the direction of the light source
        @param ambient      0...1 amount of ambient light
        @param specular     coefficient for specular highlights (e.g. 8)
        @param blurRadius   amount to blur before applying lighting (e.g. 3)
        @return the emboss maskfilter
    */
    static SkMaskFilter* CreateEmboss(  const SkScalar direction[3],
                                        SkScalar ambient, SkScalar specular,
                                        SkScalar blurRadius);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
private:
    SkBlurMaskFilter(); // can't be instantiated
};

#endif
