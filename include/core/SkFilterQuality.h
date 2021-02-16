/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFilterQuality_DEFINED
#define SkFilterQuality_DEFINED

#include "include/core/SkTypes.h"

/**
 *  Controls how much filtering to be done when scaling/transforming complex colors
 *  e.g. images
 */
enum SkFilterQuality {
    kNone_SkFilterQuality = 0,      //!< fastest but lowest quality, typically nearest-neighbor
    kLow_SkFilterQuality = 1,       //!< typically bilerp
    kMedium_SkFilterQuality = 2,    //!< typically bilerp + mipmaps for down-scaling
    kHigh_SkFilterQuality = 3,      //!< slowest but highest quality, typically bicubic or better

    kLast_SkFilterQuality = kHigh_SkFilterQuality,
    kMaxValue = kLast_SkFilterQuality,
};

#endif
