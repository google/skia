/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGradientShader.h"

#include "GrGradientEffect2.h"
#include "GrLinearGradientLayout.h"
#include "GrSingleIntervalGradientColorizer.h"

#include "SkMatrix.h"
#include "SkGradientShaderPriv.h"
#include "GrColor.h"

namespace GrGradientShader {

std::unique_ptr<GrFragmentProcessor> MakeColorizer(
        const SkGradientShaderBase& shader, const GrFPArgs& args) {
    // Currently only supports 2-color single intervals, and no textured gradients
    if (shader.fColorCount == 2) {
        SkColor4fXformer xformedColors(shader.fOrigColors4f, shader.fColorCount,
            shader.fColorSpace.get(), args.fDstColorSpaceInfo->colorSpace());

        return GrSingleIntervalGradientColorizer::Make(
            GrColor4f::FromSkColor4f(xformedColors.fColors[0]),
            GrColor4f::FromSkColor4f(xformedColors.fColors[1]));
    }

    return nullptr;
}

std::unique_ptr<GrFragmentProcessor> MakeGradient(
        std::unique_ptr<GrFragmentProcessor> colorizer,
        std::unique_ptr<GrFragmentProcessor> layout) {
    return GrFragmentProcessor::MulChildByInputAlpha(
        GrGradientEffect2::Make(std::move(colorizer), std::move(layout)));
}

std::unique_ptr<GrFragmentProcessor> MakeLinear(
    const SkGradientShaderBase& shader, const GrFPArgs& args,
    const SkPoint& start, const SkPoint& end) {

    // First create colorizer, and if the gradient's color spec is unsupported
    // then return early before allocating the remaining components of the
    // linear gradient
    std::unique_ptr<GrFragmentProcessor> colorizer = MakeColorizer(shader, args);
    if (colorizer == nullptr) {
        return nullptr;
    }

    SkMatrix invLocalMatrix;
    if (!shader.totalLocalMatrix(args.fPreLocalMatrix, args.fPostLocalMatrix)->invert(&invLocalMatrix)) {
        return nullptr;
    }

    std::unique_ptr<GrFragmentProcessor> layout = GrLinearGradientLayout::Make(
        invLocalMatrix, start, end);
    if (layout == nullptr) {
        return nullptr;
    }

    return MakeGradient(std::move(colorizer), std::move(layout));
}

}
