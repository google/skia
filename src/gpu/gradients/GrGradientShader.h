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

namespace GrGradientShader {
    std::unique_ptr<GrFragmentProcessor> MakeLinear(
        const SkGradientShaderBase& shader, const GrFPArgs& args,
        const SkPoint& start, const SkPoint& end);

    std::unique_ptr<GrFragmentProcessor> MakeRadial(
        const SkGradientShaderBase& shader, const GrFPArgs& args,
        const SkPoint& center, SkScalar radius);
}

#endif // GrGradientShader_DEFINE
