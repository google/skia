/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGradientShader_DEFINE
#define GrGradientShader_DEFINE

#include "src/gpu/GrFPArgs.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/shaders/gradients/SkGradientShaderPriv.h"
#include "src/shaders/gradients/SkLinearGradient.h"
#include "src/shaders/gradients/SkRadialGradient.h"
#include "src/shaders/gradients/SkSweepGradient.h"
#include "src/shaders/gradients/SkTwoPointConicalGradient.h"

#if GR_TEST_UTILS
#include "include/utils/SkRandom.h"
#endif

namespace GrGradientShader {
    std::unique_ptr<GrFragmentProcessor> MakeLinear(const SkLinearGradient& shader,
                                                    const GrFPArgs& args);

    std::unique_ptr<GrFragmentProcessor> MakeRadial(const SkRadialGradient& shader,
                                                    const GrFPArgs& args);

    std::unique_ptr<GrFragmentProcessor> MakeSweep(const SkSweepGradient& shader,
                                                   const GrFPArgs& args);

    std::unique_ptr<GrFragmentProcessor> MakeConical(const SkTwoPointConicalGradient& shader,
                                                     const GrFPArgs& args);

#if GR_TEST_UTILS
    /** Helper struct that stores (and populates) parameters to construct a random gradient.
        If fUseColors4f is true, then the SkColor4f factory should be called, with fColors4f and
        fColorSpace. Otherwise, the SkColor factory should be called, with fColors. fColorCount
        will be the number of color stops in either case, and fColors and fStops can be passed to
        the gradient factory. (The constructor may decide not to use stops, in which case fStops
        will be nullptr). */
    struct RandomParams {
        static constexpr int kMaxRandomGradientColors = 5;

        // Should be of similar magnitude to the draw area of the tests so that the gradient
        // sampling is done at an appropriate scale.
        static constexpr SkScalar kGradientScale = 256.0f;

        RandomParams(SkRandom* r);

        bool fUseColors4f;
        SkColor fColors[kMaxRandomGradientColors];
        SkColor4f fColors4f[kMaxRandomGradientColors];
        sk_sp<SkColorSpace> fColorSpace;
        SkScalar fStopStorage[kMaxRandomGradientColors];
        SkTileMode fTileMode;
        int fColorCount;
        SkScalar* fStops;
    };
#endif

}

#endif // GrGradientShader_DEFINE
