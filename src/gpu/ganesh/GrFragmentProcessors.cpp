/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrFragmentProcessors.h"

#include "src/core/SkMaskFilterBase.h"
#include "src/effects/SkShaderMaskFilterImpl.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/shaders/SkShaderBase.h"

#include <memory>
#include <utility>

namespace GrFragmentProcessors {
std::unique_ptr<GrFragmentProcessor> Make(const SkMaskFilter* maskfilter,
                                          const GrFPArgs& args,
                                          const SkMatrix& ctm) {
    if (!maskfilter) {
        return nullptr;
    }
    auto mfb = as_MFB(maskfilter);
    if (mfb->type() != SkMaskFilterBase::Type::kShader) {
        return nullptr;
    }
    auto shaderMF = static_cast<const SkShaderMaskFilterImpl*>(maskfilter);
    auto fp = as_SB(shaderMF->shader())->asFragmentProcessor(args, SkShaderBase::MatrixRec(ctm));
    return GrFragmentProcessor::MulInputByChildAlpha(std::move(fp));
}

bool IsSupported(const SkMaskFilter* maskfilter) {
    if (!maskfilter) {
        return false;
    }
    auto mfb = as_MFB(maskfilter);
    if (mfb->type() != SkMaskFilterBase::Type::kShader) {
        return false;
    }
    return true;
}
}  // namespace GrFragmentProcessors
