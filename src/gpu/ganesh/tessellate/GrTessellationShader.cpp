/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/ganesh/tessellate/GrTessellationShader.h"

#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrDstProxyView.h"
#include "src/gpu/ganesh/GrPipeline.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/tessellate/WangsFormula.h"

#include <utility>

enum class GrAAType : unsigned int;

const GrPipeline* GrTessellationShader::MakePipeline(const ProgramArgs& args,
                                                     GrAAType aaType,
                                                     GrAppliedClip&& appliedClip,
                                                     GrProcessorSet&& processors) {
    GrPipeline::InitArgs pipelineArgs;

    pipelineArgs.fCaps = args.fCaps;
    pipelineArgs.fDstProxyView = *args.fDstProxyView;
    pipelineArgs.fWriteSwizzle = args.fWriteView.swizzle();

    return args.fArena->make<GrPipeline>(pipelineArgs,
                                         std::move(processors),
                                         std::move(appliedClip));
}

const char* GrTessellationShader::WangsFormulaSkSL() {
    static_assert(skgpu::wangs_formula::length_term<3>(1) == 0.75);
    static_assert(skgpu::wangs_formula::length_term_p2<3>(1) == 0.5625);

    return
// Returns the length squared of the largest forward difference from Wang's cubic formula.
"float wangs_formula_max_fdiff_p2(float2 p0, float2 p1, float2 p2, float2 p3,"
                                 "float2x2 matrix) {"
    "float2 d0 = matrix * (fma(float2(-2), p1, p2) + p0);"
    "float2 d1 = matrix * (fma(float2(-2), p2, p3) + p1);"
    "return max(dot(d0,d0), dot(d1,d1));"
"}"
"float wangs_formula_cubic(float _precision_, float2 p0, float2 p1, float2 p2, float2 p3,"
                          "float2x2 matrix) {"
    "float m = wangs_formula_max_fdiff_p2(p0, p1, p2, p3, matrix);"
    "return max(ceil(sqrt(0.75 * _precision_ * sqrt(m))), 1.0);"
"}"
"float wangs_formula_cubic_log2(float _precision_, float2 p0, float2 p1, float2 p2, float2 p3,"
                               "float2x2 matrix) {"
    "float m = wangs_formula_max_fdiff_p2(p0, p1, p2, p3, matrix);"
    "return ceil(log2(max(0.5625 * _precision_ * _precision_ * m, 1.0)) * .25);"
"}"
"float wangs_formula_conic_p2(float _precision_, float2 p0, float2 p1, float2 p2, float w) {"
    // Translate the bounding box center to the origin.
    "float2 C = (min(min(p0, p1), p2) + max(max(p0, p1), p2)) * 0.5;"
    "p0 -= C;"
    "p1 -= C;"
    "p2 -= C;"

    // Compute max length.
    "float m = sqrt(max(max(dot(p0,p0), dot(p1,p1)), dot(p2,p2)));"

    // Compute forward differences.
    "float2 dp = fma(float2(-2.0 * w), p1, p0) + p2;"
    "float dw = abs(fma(-2.0, w, 2.0));"

    // Compute numerator and denominator for parametric step size of linearization. Here, the
    // epsilon referenced from the cited paper is 1/precision.
    "float rp_minus_1 = max(0.0, fma(m, _precision_, -1.0));"
    "float numer = length(dp) * _precision_ + rp_minus_1 * dw;"
    "float denom = 4 * min(w, 1.0);"

    "return numer/denom;"
"}"
"float wangs_formula_conic(float _precision_, float2 p0, float2 p1, float2 p2, float w) {"
    "float n2 = wangs_formula_conic_p2(_precision_, p0, p1, p2, w);"
    "return max(ceil(sqrt(n2)), 1.0);"
"}"
"float wangs_formula_conic_log2(float _precision_, float2 p0, float2 p1, float2 p2, float w) {"
    "float n2 = wangs_formula_conic_p2(_precision_, p0, p1, p2, w);"
    "return ceil(log2(max(n2, 1.0)) * .5);"
"}"
;
}
