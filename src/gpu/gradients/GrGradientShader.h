/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGradientShader_DEFINE
#define GrGradientShader_DEFINE

#include "GrFPArgs.h"
#include "SkGradientShaderPriv.h"
#include "SkLinearGradient.h"

namespace GrGradientShader {
    std::unique_ptr<GrFragmentProcessor> MakeLinear(const SkLinearGradient& shader,
                                                    const GrFPArgs& args);
}

#endif // GrGradientShader_DEFINE
