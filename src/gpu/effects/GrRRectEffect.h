/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRRectEffect_DEFINED
#define GrRRectEffect_DEFINED

#include "GrTypes.h"
#include "GrTypesPriv.h"
#include "SkRefCnt.h"

class GrFragmentProcessor;
class GrProcessor;
class SkRRect;

namespace GrRRectEffect {
    /**
     * Creates an effect that performs anti-aliased clipping against a SkRRect. It doesn't support
     * all varieties of SkRRect so the caller must check for a nullptr return.
     */
    sk_sp<GrFragmentProcessor> Make(GrPrimitiveEdgeType, const SkRRect&);
};

#endif
