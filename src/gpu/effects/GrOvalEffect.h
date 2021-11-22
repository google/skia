/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrOvalEffect_DEFINED
#define GrOvalEffect_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/gpu/GrTypes.h"
#include "include/private/GrTypesPriv.h"
#include "src/gpu/GrFragmentProcessor.h"

struct GrShaderCaps;
struct SkRect;

namespace GrOvalEffect {

/**
 * Creates an effect that performs clipping against an oval.
 */
GrFPResult Make(std::unique_ptr<GrFragmentProcessor>, GrClipEdgeType, const SkRect&,
                const GrShaderCaps&);
}  // namespace GrOvalEffect

#endif
