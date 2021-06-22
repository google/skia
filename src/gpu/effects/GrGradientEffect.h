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

std::unique_ptr<GrFragmentProcessor> SingleInterval(const SkPMColor4f& start,
                                                    const SkPMColor4f& end);

std::unique_ptr<GrFragmentProcessor> DualInterval(const SkPMColor4f& c0,
                                                  const SkPMColor4f& c1,
                                                  const SkPMColor4f& c2,
                                                  const SkPMColor4f& c3,
                                                  float threshold);

}  // namespace GrGradientColorizer

#endif
