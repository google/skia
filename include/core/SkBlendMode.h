/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlendMode_DEFINED
#define SkBlendMode_DEFINED

#include "SkTypes.h"

enum class SkBlendMode {
    kClear,    //!< [0, 0]
    kSrc,      //!< [Sa, Sc]
    kDst,      //!< [Da, Dc]
    kSrcOver,  //!< [Sa + Da * (1 - Sa), Sc + Dc * (1 - Sa)]
    kDstOver,  //!< [Da + Sa * (1 - Da), Dc + Sc * (1 - Da)]
    kSrcIn,    //!< [Sa * Da, Sc * Da]
    kDstIn,    //!< [Da * Sa, Dc * Sa]
    kSrcOut,   //!< [Sa * (1 - Da), Sc * (1 - Da)]
    kDstOut,   //!< [Da * (1 - Sa), Dc * (1 - Sa)]
    kSrcATop,  //!< [Da, Sc * Da + Dc * (1 - Sa)]
    kDstATop,  //!< [Sa, Dc * Sa + Sc * (1 - Da)]
    kXor,      //!< [Sa + Da - 2 * Sa * Da, Sc * (1 - Da) + Dc * (1 - Sa)]
    kPlus,     //!< [Sa + Da, Sc + Dc]
    kModulate, // multiplies all components (= alpha and color)

    // Following blend modes are defined in the CSS Compositing standard:
    // https://dvcs.w3.org/hg/FXTF/rawfile/tip/compositing/index.html#blending
    kScreen,
    kLastCoeffMode = kScreen,

    kOverlay,
    kDarken,
    kLighten,
    kColorDodge,
    kColorBurn,
    kHardLight,
    kSoftLight,
    kDifference,
    kExclusion,
    kMultiply,
    kLastSeparableMode = kMultiply,

    kHue,
    kSaturation,
    kColor,
    kLuminosity,
    kLastMode = kLuminosity
};

/**
 *  Return the (c-string) name of the blendmode.
 */
SK_API const char* SkBlendMode_Name(SkBlendMode);

#endif
