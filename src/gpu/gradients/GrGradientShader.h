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
#include "SkTwoPointConicalGradient.h"

namespace GrGradientShader {
    // NOTE: The layout-specific subclasses of SkGradientShaderBase do not
    // expose their geometric specifications, so without updating their APIs,
    // these factory functions cannot just take SkLinearGradient, SkRadialGradient,
    // etc., and instead the specific gradient subclasses pass in their private
    // members in their asFragmentProcessor() implementations.


    std::unique_ptr<GrFragmentProcessor> MakeLinear(
        const SkGradientShaderBase& shader, const GrFPArgs& args,
        const SkPoint& start, const SkPoint& end);

    std::unique_ptr<GrFragmentProcessor> MakeRadial(
        const SkGradientShaderBase& shader, const GrFPArgs& args,
        const SkPoint& center, SkScalar radius);

    std::unique_ptr<GrFragmentProcessor> MakeSweep(
        const SkGradientShaderBase& shader, const GrFPArgs& args,
        const SkPoint& center, SkScalar bias, SkScalar scale);

    // 2pt conical gradient does actual expose all of its geometric parameters
    std::unique_ptr<GrFragmentProcessor> MakeConical(
        const SkTwoPointConicalGradient& shader, const GrFPArgs& args);
}

#endif // GrGradientShader_DEFINE
