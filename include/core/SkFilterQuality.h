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
 *  e.g. images.
    These values are persisted to logs. Entries should not be renumbered and
    numeric values should never be reused.
 */
enum SkFilterQuality {
    kNone_SkFilterQuality   = 0,    //!< nearest-neighbor; fastest but lowest quality
    kLow_SkFilterQuality    = 1,    //!< bilerp
    kMedium_SkFilterQuality = 2,    //!< bilerp + mipmaps; good for down-scaling
    kHigh_SkFilterQuality   = 3,    //!< bicubic resampling; slowest but good quality

    kLast_SkFilterQuality = kHigh_SkFilterQuality,
};

#endif
