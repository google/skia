/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFilterQuality_DEFINED
#define SkFilterQuality_DEFINED

#include "SkTypes.h"

/**
 *  Controls how much filtering to be done when scaling/transforming complex colors
 *  e.g. images
 */
enum SkFilterQuality {
    kNone_SkFilterQuality,      //!< fastest but lowest quality, typically nearest-neighbor
    kLow_SkFilterQuality,       //!< typically bilerp
    kMedium_SkFilterQuality,    //!< typically bilerp + mipmaps for down-scaling
    kHigh_SkFilterQuality,      //!< slowest but highest quality, typically bicubic or better

    kLast_SkFilterQuality = kHigh_SkFilterQuality,
};

/** Returns name of filterQuality as null-terminated C string.

    @param filterQuality One of kNone_SkFilterQuality, kLow_SkFilterQuality,
                         kMedium_SkFilterQuality, or kHigh_SkFilterQuality.
    @return              C string
*/
SK_API const char* SkFilterQualityToString(SkFilterQuality filterQuality);


#endif
