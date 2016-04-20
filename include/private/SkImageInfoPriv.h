/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkImageInfoPriv_DEFINED
#define SkImageInfoPriv_DEFINED

#include "SkImageInfo.h"

// Indicate how images and gradients should interpret colors by default.
extern bool gDefaultProfileIsSRGB;

static SkColorProfileType SkDefaultColorProfile() {
    return gDefaultProfileIsSRGB ? kSRGB_SkColorProfileType : kLinear_SkColorProfileType;
}

#endif  // SkImageInfoPriv_DEFINED
