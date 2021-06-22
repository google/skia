/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGradientEffect_DEFINED
#define GrGradientEffect_DEFINED

#include "src/gpu/GrFragmentProcessor.h"

namespace GrGradientColorizer {

    std::unique_ptr<GrFragmentProcessor> SingleInterval(SkPMColor4f start, SkPMColor4f end);

}  // namespace GrGradientColorizer

#endif
