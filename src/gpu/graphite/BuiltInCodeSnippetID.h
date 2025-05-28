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

    // Snippet that passes through prior stage output
    kPriorOutput,

    // SkShader code snippets
    kSolidColorShader,
    kRGBPaintColor,
    kAlphaOnlyPaintColor,
    kLinearGradientShader4,
    kLinearGradientShader8,
    kLinearGradientShaderTexture,
    kLinearGradientShaderBuffer,
    kRadialGradientShader4,
    kRadialGradientShader8,
    kRadialGradientShaderTexture,
    kRadialGradientShaderBuffer,
    kSweepGradientShader4,
    kSweepGradientShader8,
    kSweepGradientShaderTexture,
    kSweepGradientShaderBuffer,
    kConicalGradientShader4,
    kConicalGradientShader8,
    kConicalGradientShaderTexture,
    kConicalGradientShaderBuffer,

    kLocalMatrixShader,
    kLocalMatrixShaderPersp,
    kImageShader,
    kImageShaderClamp,
    kCubicImageShader,
    kHWImageShader,
    kYUVImageShader,
    kCubicYUVImageShader,
    kHWYUVImageShader,
    kHWYUVNoSwizzleImageShader,
    kCoordNormalizeShader,
    kCoordClampShader,
    kDitherShader,
    kPerlinNoiseShader,

    // SkColorFilter code snippets
    kMatrixColorFilter,
    kHSLMatrixColorFilter,
    kTableColorFilter,
    kGaussianColorFilter,

    // Color space transform snippet and its specializations
    kColorSpaceXformColorFilter,
    kColorSpaceXformPremul,
    kColorSpaceXformSRGB,

    // Emits special variable holding the primitiveColor emitted by a RenderStep
    kPrimitiveColor,

    // Analytic clip for circular roundrect and AA rect shapes
    kAnalyticClip,

    // Analytic plus atlas-based clip
    kAnalyticAndAtlasClip,

    kCompose, // compose 2 children together: outer_1(inner_0(...))
    kBlendCompose, // compose 3 children together: outer_2(inner_0(...), inner_1(...))

    // SkBlender code snippets
    kPorterDuffBlender, // Can handle all GetPorterDuffBlendConstants() modes
    kHSLCBlender, // kHue,kSaturation,kLuminosity, and kColor modes
    // NOTE: We could offer an in-shader consolidation for overlay+hardlight, and darken+lighten
    // but for now, those will map to the FixedBlends.

    // Fixed blend modes hard code a specific blend function into the shader tree. This can be
    // valuable when an internal effect is known to always do a certain blend and we want to
    // benefit from inlining constants. It is also important for being able to convert the final
    // blend of the SkPaint into fixed function HW blending, where each HW blend is part of the
    // pipeline key, so using a known blend mode ID ensures the PaintParamsKey are also different.
    //
    // Lastly, for advanced blend modes that require complex shader calculations, we assume they
    // are used rarely and with intent (i.e. unlikely to share a common shader tree with another
    // advanced blend if we were to add branching). This keeps the amount of reachable SkSL that
    // must be compiled for a given pipeline with advanced blends to a minimum.
    //
    // NOTE: Pipeline code generation depends on the fixed-function code IDs being contiguous and be
    // defined last in the enum. They are ordered to match SkBlendMode such that:
    //     (id - kFirstFixedBlend) == SkBlendMode).
    kFixedBlend_Clear,
    kFixedBlend_Src,
    kFixedBlend_Dst,
    kFixedBlend_SrcOver,
    kFixedBlend_DstOver,
    kFixedBlend_SrcIn,
    kFixedBlend_DstIn,
    kFixedBlend_SrcOut,
    kFixedBlend_DstOut,
    kFixedBlend_SrcATop,
    kFixedBlend_DstATop,
    kFixedBlend_Xor,

    kFixedBlend_Plus,     // NOTE: Adds shader clamping, not compatible with GetPorterDuffConstants
    kFixedBlend_Modulate, // NOTE: Uses color channels, incompatible with kPorterDuffBlender
    kFixedBlend_Screen,   //   ""

    // With support for advanced blend modes, these can also be handled by HW for the final blend.
    kFixedBlend_Overlay,
    kFixedBlend_Darken,
    kFixedBlend_Lighten,
    kFixedBlend_ColorDodge,
    kFixedBlend_ColorBurn,
    kFixedBlend_HardLight,
    kFixedBlend_SoftLight,
    kFixedBlend_Difference,
    kFixedBlend_Exclusion,
    kFixedBlend_Multiply,

    kFixedBlend_Hue,
    kFixedBlend_Saturation,
    kFixedBlend_Color,
    kFixedBlend_Luminosity,

    kFirstFixedBlend = kFixedBlend_Clear,
    kLast = kFixedBlend_Luminosity
};
static constexpr int kBuiltInCodeSnippetIDCount = static_cast<int>(BuiltInCodeSnippetID::kLast)+1;
static constexpr int kFixedBlendIDOffset =
        static_cast<int>(BuiltInCodeSnippetID::kFirstFixedBlend);

static_assert(BuiltInCodeSnippetID::kLast == BuiltInCodeSnippetID::kFixedBlend_Luminosity);

}  // namespace skgpu::graphite

#endif // skgpu_graphite_BuiltInCodeSnippetID_DEFINED
