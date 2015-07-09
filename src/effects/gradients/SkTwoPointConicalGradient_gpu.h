/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTwoPointConicalGradient_gpu_DEFINED
#define SkTwoPointConicalGradient_gpu_DEFINED

#include "SkGradientShaderPriv.h"

class GrProcessor;
class SkTwoPointConicalGradient;

namespace Gr2PtConicalGradientEffect {
    /**
     * Creates an effect that produces a two point conical gradient based on the
     * shader passed in.
     */
    GrFragmentProcessor* Create(GrContext* ctx,GrProcessorDataManager* procDataManager,
                                const SkTwoPointConicalGradient& shader,
                                SkShader::TileMode tm, const SkMatrix* localMatrix);
};

#endif
