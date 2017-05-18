/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrDitherEffect_DEFINED
#define GrDitherEffect_DEFINED

#include "GrTypes.h"
#include "GrTypesPriv.h"
#include "SkRefCnt.h"

class GrFragmentProcessor;

namespace GrDitherEffect {
    /**
     * Creates an effect that dithers the resulting color to an RGBA8 framebuffer
     */
    sk_sp<GrFragmentProcessor> Make();
};

#endif
