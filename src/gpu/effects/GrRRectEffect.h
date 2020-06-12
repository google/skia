/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrRRectEffect_DEFINED
#define GrRRectEffect_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"

class GrFragmentProcessor;
class GrShaderCaps;
class GrProcessor;
class SkRRect;

namespace GrRRectEffect {

/**
 * Creates an effect that performs anti-aliased clipping against a SkRRect. It doesn't support
 * all varieties of SkRRect so the caller must check for a nullptr return.
 *
 * The input fragment processor is passed as a pointer because it is only absorbed if creation
 * of the round-rect effect is successful. If Make returns nullptr, the inputFP is left as-is.
 */
std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor>* inputFP,
                                          GrClipEdgeType, const SkRRect&, const GrShaderCaps&);
};

#endif
