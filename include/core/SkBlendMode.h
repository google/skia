/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendMode_DEFINED
#define SkBlendMode_DEFINED

#include "include/core/SkTypes.h"

enum class SkBlendMode {
    kClear,                           //!< replaces destination with zero: fully transparent
    kSrc,                             //!< replaces destination
    kDst,                             //!< preserves destination
    kSrcOver,                         //!< source over destination
    kDstOver,                         //!< destination over source
    kSrcIn,                           //!< source trimmed inside destination
    kDstIn,                           //!< destination trimmed by source
    kSrcOut,                          //!< source trimmed outside destination
    kDstOut,                          //!< destination trimmed outside source
    kSrcATop,                         //!< source inside destination blended with destination
    kDstATop,                         //!< destination inside source blended with source
    kXor,                             //!< each of source and destination trimmed outside the other
    kPlus,                            //!< sum of colors
    kModulate,                        //!< product of premultiplied colors; darkens destination
    kScreen,                //!< multiply inverse of pixels, inverting result; brightens destination
    kLastCoeffMode     = kScreen,     //!< last porter duff blend mode
    kOverlay,                         //!< multiply or screen, depending on destination
    kDarken,                          //!< darker of source and destination
    kLighten,                         //!< lighter of source and destination
    kColorDodge,                      //!< brighten destination to reflect source
    kColorBurn,                       //!< darken destination to reflect source
    kHardLight,                       //!< multiply or screen, depending on source
    kSoftLight,                       //!< lighten or darken, depending on source
    kDifference,                      //!< subtract darker from lighter with higher contrast
    kExclusion,                       //!< subtract darker from lighter with lower contrast
    kMultiply,                        //!< multiply source with destination, darkening image
    kLastSeparableMode = kMultiply,   //!< last blend mode operating separately on components
    kHue,                           //!< hue of source with saturation and luminosity of destination
    kSaturation,                    //!< saturation of source with hue and luminosity of destination
    kColor,                         //!< hue and saturation of source with luminosity of destination
    kLuminosity,                    //!< luminosity of source with hue and saturation of destination
    kLastMode          = kLuminosity, //!< last valid value
};

/**
 * For Porter-Duff SkBlendModes (those <= kLastCoeffMode), these coefficients describe the blend
 * equation used. Coefficient-based blend modes specify an equation:
 * ('dstCoeff' * dst + 'srcCoeff' * src), where the coefficient values are constants, functions of
 * the src or dst alpha, or functions of the src or dst color.
 */
enum class SkBlendModeCoeff {
    kZero, /** 0 */
    kOne,  /** 1 */
    kSC,   /** src color */
    kISC,  /** inverse src color (i.e. 1 - sc) */
    kDC,   /** dst color */
    kIDC,  /** inverse dst color (i.e. 1 - dc) */
    kSA,   /** src alpha */
    kISA,  /** inverse src alpha (i.e. 1 - sa) */
    kDA,   /** dst alpha */
    kIDA,  /** inverse dst alpha (i.e. 1 - da) */

    kCoeffCount
};

/**
 * Returns true if 'mode' is a coefficient-based blend mode (<= kLastCoeffMode). If true is
 * returned, the mode's src and dst coefficient functions are set in 'src' and 'dst'.
 */
SK_API bool SkBlendMode_AsCoeff(SkBlendMode mode, SkBlendModeCoeff* src, SkBlendModeCoeff* dst);


/** Returns name of blendMode as null-terminated C string.

    @return           C string
*/
SK_API const char* SkBlendMode_Name(SkBlendMode blendMode);

#endif
