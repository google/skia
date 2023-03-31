/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_BuiltInCodeSnippetID_DEFINED
#define skgpu_graphite_BuiltInCodeSnippetID_DEFINED

#include "include/core/SkTypes.h"

namespace skgpu::graphite {

enum class BuiltInCodeSnippetID : int32_t {
    // This isn't just a signal for a failure during paintparams key creation. It also actually
    // implements the default behavior for an erroneous draw. Currently it just draws solid
    // magenta.
    kError,

    // SkShader code snippets
    kPassthroughShader,
    kSolidColorShader,
    kLinearGradientShader4,
    kLinearGradientShader8,
    kLinearGradientShaderTexture,
    kRadialGradientShader4,
    kRadialGradientShader8,
    kRadialGradientShaderTexture,
    kSweepGradientShader4,
    kSweepGradientShader8,
    kSweepGradientShaderTexture,
    kConicalGradientShader4,
    kConicalGradientShader8,
    kConicalGradientShaderTexture,

    kLocalMatrixShader,
    kImageShader,
    kCoordClampShader,
    kDitherShader,
    kPerlinNoiseShader,
    kPorterDuffBlendShader,     // ComposeShader (lightweight, only supports Porter-Duff blends)
    kBlendShader,               // ComposeShader (more code, but supports every SkBlendMode)
    kColorFilterShader,
    kRuntimeShader,

    // SkColorFilter code snippets
    kMatrixColorFilter,
    kBlendColorFilter,
    kComposeColorFilter,
    kTableColorFilter,
    kGaussianColorFilter,
    kColorSpaceXformColorFilter,

    // SkBlender code snippets (evaluating a blend as part of the shader graph)
    kPassthroughBlender,

    // BlendMode code snippets (applying a blend to a destination)
    kShaderBasedBlender,
    kPrimitiveColorShaderBasedBlender, // Blend dst: primitiveColor variable emitted by RenderStep
// Fixed-function blend modes are used for the final blend with the dst buffer's color when the
    // SkPaint is using a coefficient-based SkBlendMode. The actual coefficients are extracted into
    // the SkBlendInfo associated with each pipeline, but a unique code snippet ID is assigned so
    // that the pipeline keys remain distinct. They are ordered to match SkBlendMode such
    // that (id - kFirstFixedFunctionBlendMode) == SkBlendMode).
    //
    // NOTE: Pipeline code generation depends on the fixed-function code IDs being contiguous and
    // be defined last in the enum.
    kFixedFunctionClearBlendMode,
    kFixedFunctionSrcBlendMode,
    kFixedFunctionDstBlendMode,
    kFixedFunctionSrcOverBlendMode,
    kFixedFunctionDstOverBlendMode,
    kFixedFunctionSrcInBlendMode,
    kFixedFunctionDstInBlendMode,
    kFixedFunctionSrcOutBlendMode,
    kFixedFunctionDstOutBlendMode,
    kFixedFunctionSrcATopBlendMode,
    kFixedFunctionDstATopBlendMode,
    kFixedFunctionXorBlendMode,
    kFixedFunctionPlusBlendMode,
    kFixedFunctionModulateBlendMode,
    kFixedFunctionScreenBlendMode,

    kFirstFixedFunctionBlendMode = kFixedFunctionClearBlendMode,
    kLast = kFixedFunctionScreenBlendMode
};
static constexpr int kBuiltInCodeSnippetIDCount = static_cast<int>(BuiltInCodeSnippetID::kLast)+1;
static constexpr int kFixedFunctionBlendModeIDOffset =
        static_cast<int>(BuiltInCodeSnippetID::kFirstFixedFunctionBlendMode);

static_assert(BuiltInCodeSnippetID::kLast == BuiltInCodeSnippetID::kFixedFunctionScreenBlendMode);

} // skgpu::graphite

#endif // skgpu_graphite_BuiltInCodeSnippetID_DEFINED
