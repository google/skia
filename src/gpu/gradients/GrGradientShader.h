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

#if GR_TEST_UTILS
    /** Helper struct that stores (and populates) parameters to construct a random gradient.
        If fUseColors4f is true, then the SkColor4f factory should be called, with fColors4f and
        fColorSpace. Otherwise, the SkColor factory should be called, with fColors. fColorCount
        will be the number of color stops in either case, and fColors and fStops can be passed to
        the gradient factory. (The constructor may decide not to use stops, in which case fStops
        will be nullptr). */
    struct RandomParams {
        static constexpr int kMaxRandomGradientColors = 5;

        RandomParams(SkRandom* r);

        bool fUseColors4f;
        SkColor fColors[kMaxRandomGradientColors];
        SkColor4f fColors4f[kMaxRandomGradientColors];
        sk_sp<SkColorSpace> fColorSpace;
        SkScalar fStopStorage[kMaxRandomGradientColors];
        SkShader::TileMode fTileMode;
        int fColorCount;
        SkScalar* fStops;
    };
#endif

}

#endif // GrGradientShader_DEFINE
