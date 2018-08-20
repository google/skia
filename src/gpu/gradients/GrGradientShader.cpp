/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrGradientShader.h"

#include "SkMatrix.h"


std::unique_ptr<GrFragmentProcessor> MakeColorizer(
        const SkGradientShaderBase& shader, const GrFPArgs& args) {
    // Currently only supports 2-color single intervals, and no textured gradients

}

std::unique_ptr<GrFragmentProcessor> MakeGradient(
        std::unique_ptr<GrFragmentProcessor> colorizer,
        std::unique_ptr<GrFragmentProcessor> layout) {
    return GrFragmentProcessor::MulChildByInputAlpha(
        GrGradientEffect2::Make(std::move(colorizer), std::move(layout)));
}

std::unique_ptr<GrFragmentProcessor> GrGradientShader::MakeLinear(
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

    return MakeGradient(colorizer, layout);
}
