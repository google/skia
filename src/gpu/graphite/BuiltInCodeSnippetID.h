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

enum class BuiltInCodeSnippetID : uint32_t {
    // This isn't just a signal for a failure during paintparams key creation. It also actually
    // implements the default behavior for an erroneous draw. That said, Device discards draws that
    // have a PaintParamsKey referencing an error ID so this behavior is mostly academic.
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

    // Color space, read swizzle, and alpha type conversions w/ specializations - these are built by
    // composing together specialized or generalized child snippets that handle these stages:
    // 1. Initial alpha (unpremul, force-opaque, identity, or alpha-swizzle)
    // 2. Linear transfer function
    // 3. Gamut transform
    // 4. Encoding transfer function
    // 5. Post alpha handling (premul, identity)
    //
    // The way an SkColorSpaceXformSteps is converted to a series of kCSXform snippets depends on
    // the presence of KeyGenFlags::kSpecializeColorSpaceXform. If that flag is present, then only
    // the non-generic snippets will be used in each stage and no-op steps are skipped entirely.
    // This leads to the following combinations (setting aside AlphaOnly):
    //  [no-op, Unpremul, ForceOpaque] X [no-op, sRGB, PQ, HLG]^2 X [no-op, Gamut] X [no-op, Premul]
    //     => 3 x 4^2 x 2 x 2 => 192 options (minus a few combos that don't show up in practice).
    //
    // In the scope of kSpecializeColorSpaceXform, it is assumed that the actual encountered
    // color spaces are locked down enough that the actual set of encountered combinations will be
    // much lower than 192. To avoid this same explosion for simpler pipelines, the 192 options are
    // mapped to 10 more generic options (specializing just the transfer functions):
    //  [*] X no-op X no-op X no-op X [*] => PreAlpha
    //  [*] X TF1 X [no-op, Gamut] X TF2 X [*] => PreAlpha+TF1+Gamut+TF2+PostAlpha
    //
    // In the latter case, identity transfer functions will map to sRGB since it has a fast-path
    // for the identity already. Identity gamut matrices are not elided either. This means that
    // there are only 9 combinations plus the specialized PreAlpha for near-identity cases:
    //  PreAlpha+[sRGB, PQ, HLG]+Gamut+[sRGB, PQ, HLGInv]+PostAlpha
    //
    // The measured overhead of PreAlpha, PostAlpha, and always including Gamut is much smaller than
    // the overhead of branching around the various transfer functions. Always specializing the
    // transfer functions gives the maximum performance tradeoff with a minimal number of pipelines.

    // Initial alpha stages:
    //  - Alpha-only images are always specialized to _AlphaOnly because the rest of the pipeline
    //    is already specialized on blending in color. It is never combined with other stages.
    //  - The generic _PreAlpha can be specialized into _Unpremul, _ForceOpaque, or eliding the
    //    stage (for no-op) when desired. It can also handle _Premul if there are no other stages.
    //  - These stages apply the alpha swizzle of the texture's read swizzle.
    kCSXform_AlphaOnly,   // handles alpha-swizzle (no other CSXform steps combine with it)
    kCSXform_PreAlpha,    // handles unpremul, force-opaque, no-op
    kCSXform_Unpremul,    // specialization always performing unpremul
    kCSXform_ForceOpaque, // specialization always forcing to opaque

    // Transfer function stages:
    //  - sRGB and PQ can be used for both linear and encode stages; HLG is used to convert to
    //    linear, and HLGInv is used to encode.
    //  - Identity transfer functions can either be elided or encoded into sRGB coefficients.
    kCSXform_sRGB,       // sRGB linear and encoding
    kCSXform_PQ,         // PQ linear and encoding
    kCSXform_HLG,        // linear stage only, supports optional post-ootf parameters
    kCSXform_HLGInv,     // encoding stage only, supports optional pre-ootf parameters

    // Gamut transform stage:
    //  - When specializing, can be elided entirely; otherwise can be set to the identity
    //  - This matrix is also used to apply any RGB swizzle of the texture's read swizzle
    kCSXform_Gamut,

    // Post alpha stages:
    kCSXform_PostAlpha, // handles premul and no-op
    kCSXform_Premul,    // specialization always performing premul

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
