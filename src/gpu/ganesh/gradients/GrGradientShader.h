/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGradientShader_DEFINE
#define GrGradientShader_DEFINE

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "src/shaders/gradients/SkLinearGradient.h"

#include <memory>

class GrFragmentProcessor;
class SkGradientBaseShader;
class SkMatrix;
class SkRandom;
enum class SkTileMode;
struct GrFPArgs;

namespace SkShaders {
class MatrixRec;
}

namespace GrGradientShader {
std::unique_ptr<GrFragmentProcessor> MakeGradientFP(const SkGradientBaseShader& shader,
                                                    const GrFPArgs& args,
                                                    const SkShaders::MatrixRec&,
                                                    std::unique_ptr<GrFragmentProcessor> layout,
                                                    const SkMatrix* overrideMatrix = nullptr);

std::unique_ptr<GrFragmentProcessor> MakeLinear(const SkLinearGradient& shader,
                                                const GrFPArgs& args,
                                                const SkShaders::MatrixRec&);

#if defined(GPU_TEST_UTILS)
    /** Helper struct that stores (and populates) parameters to construct a random gradient.
        If fUseColors4f is true, then the SkColor4f factory should be called, with fColors4f and
        fColorSpace. Otherwise, the SkColor factory should be called, with fColors. fColorCount
        will be the number of color stops in either case, and fColors and fStops can be passed to
        the gradient factory. (The constructor may decide not to use stops, in which case fStops
        will be nullptr). */
    struct RandomParams {
        inline static constexpr int kMaxRandomGradientColors = 5;

        // Should be of similar magnitude to the draw area of the tests so that the gradient
        // sampling is done at an appropriate scale.
        inline static constexpr SkScalar kGradientScale = 256.0f;

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

}  // namespace GrGradientShader

#endif // GrGradientShader_DEFINE
