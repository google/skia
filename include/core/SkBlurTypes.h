/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlurTypes_DEFINED
#define SkBlurTypes_DEFINED

#include "SkTypes.h"

enum SkBlurStyle {
    kNormal_SkBlurStyle,  //!< fuzzy inside and outside
    kSolid_SkBlurStyle,   //!< solid inside, fuzzy outside
    kOuter_SkBlurStyle,   //!< nothing inside, fuzzy outside
    kInner_SkBlurStyle,   //!< fuzzy inside, nothing outside

    kLastEnum_SkBlurStyle = kInner_SkBlurStyle
};

enum SkBlurQuality {
    kLow_SkBlurQuality,     //!< e.g. box filter
    kHigh_SkBlurQuality,    //!< e.g. 3-pass similar to gaussian

    kLastEnum_SkBlurQuality
};

#endif
