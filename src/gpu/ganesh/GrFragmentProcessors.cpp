/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/GrFragmentProcessors.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkSpan.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/GrTypes.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTArray.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkBlendModeBlender.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkRuntimeBlender.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/effects/SkShaderMaskFilterImpl.h"
#include "src/effects/colorfilters/SkBlendModeColorFilter.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"
#include "src/effects/colorfilters/SkColorSpaceXformColorFilter.h"
#include "src/effects/colorfilters/SkComposeColorFilter.h"
#include "src/effects/colorfilters/SkGaussianColorFilter.h"
#include "src/effects/colorfilters/SkMatrixColorFilter.h"
#include "src/effects/colorfilters/SkRuntimeColorFilter.h"
#include "src/effects/colorfilters/SkTableColorFilter.h"
#include "src/effects/colorfilters/SkWorkingFormatColorFilter.h"
#include "src/gpu/ResourceKey.h"
#include "src/gpu/Swizzle.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrColorSpaceXform.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProxyProvider.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrSamplerState.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/GrSurfaceProxy.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTextureProxy.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrBlendFragmentProcessor.h"
#include "src/gpu/ganesh/effects/GrColorTableEffect.h"
#include "src/gpu/ganesh/effects/GrMatrixEffect.h"
#include "src/gpu/ganesh/effects/GrPerlinNoise2Effect.h"
#include "src/gpu/ganesh/effects/GrSkSLFP.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/gradients/GrGradientShader.h"
#include "src/gpu/ganesh/image/GrImageUtils.h"
#include "src/shaders/SkBlendShader.h"
#include "src/shaders/SkColorFilterShader.h"
#include "src/shaders/SkColorShader.h"
#include "src/shaders/SkCoordClampShader.h"
#include "src/shaders/SkEmptyShader.h"
#include "src/shaders/SkImageShader.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/shaders/SkPerlinNoiseShaderImpl.h"
#include "src/shaders/SkPictureShader.h"
#include "src/shaders/SkRuntimeShader.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/SkTransformShader.h"
#include "src/shaders/SkTriColorShader.h"
#include "src/shaders/SkWorkingColorSpaceShader.h"
#include "src/shaders/gradients/SkConicalGradient.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"
#include "src/shaders/gradients/SkLinearGradient.h"
#include "src/shaders/gradients/SkRadialGradient.h"
#include "src/shaders/gradients/SkSweepGradient.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <optional>
#include <utility>

class SkBitmap;
enum class SkTileMode;

namespace GrFragmentProcessors {
static std::unique_ptr<GrFragmentProcessor>
        make_fp_from_shader_mask_filter(const SkMaskFilterBase* maskfilter,
                                        const GrFPArgs& args,
                                        const SkMatrix& ctm) {
    SkASSERT(maskfilter);
    auto shaderMF = static_cast<const SkShaderMaskFilterImpl*>(maskfilter);
    auto fp = Make(shaderMF->shader().get(), args, ctm);
    return GrFragmentProcessor::MulInputByChildAlpha(std::move(fp));
}

std::unique_ptr<GrFragmentProcessor> Make(const SkMaskFilter* maskfilter,
                                          const GrFPArgs& args,
                                          const SkMatrix& ctm) {
    if (!maskfilter) {
        return nullptr;
    }
    auto mfb = as_MFB(maskfilter);
    switch (mfb->type()) {
        case SkMaskFilterBase::Type::kShader:
            return make_fp_from_shader_mask_filter(mfb, args, ctm);
        case SkMaskFilterBase::Type::kBlur:
        case SkMaskFilterBase::Type::kEmboss:
        case SkMaskFilterBase::Type::kSDF:
        case SkMaskFilterBase::Type::kTable:
            return nullptr;
    }
    SkUNREACHABLE;
}

bool IsSupported(const SkMaskFilter* maskfilter) {
    if (!maskfilter) {
        return false;
    }
    auto mfb = as_MFB(maskfilter);
    switch (mfb->type()) {
        case SkMaskFilterBase::Type::kShader:
            return true;
        case SkMaskFilterBase::Type::kBlur:
        case SkMaskFilterBase::Type::kEmboss:
        case SkMaskFilterBase::Type::kSDF:
        case SkMaskFilterBase::Type::kTable:
            return false;
    }
    SkUNREACHABLE;
}

using ChildType = SkRuntimeEffect::ChildType;

GrFPResult MakeChildFP(const SkRuntimeEffect::ChildPtr& child, const GrFPArgs& childArgs) {
    std::optional<ChildType> type = child.type();
    if (!type.has_value()) {
        // We have a null child effect.
        return GrFPNullableSuccess(nullptr);
    }

    switch (*type) {
        case ChildType::kShader: {
            // Convert a SkShader into a child FP.
            SkShaders::MatrixRec mRec(SkMatrix::I());
            mRec.markTotalMatrixInvalid();
            auto childFP = GrFragmentProcessors::Make(child.shader(), childArgs, mRec);
            return childFP ? GrFPSuccess(std::move(childFP))
                           : GrFPFailure(nullptr);
        }
        case ChildType::kColorFilter: {
            // Convert a SkColorFilter into a child FP.
            auto [success, childFP] = GrFragmentProcessors::Make(childArgs.fContext,
                                                                 child.colorFilter(),
                                                                 /*inputFP=*/nullptr,
                                                                 *childArgs.fDstColorInfo,
                                                                 childArgs.fSurfaceProps);
            return success ? GrFPSuccess(std::move(childFP))
                           : GrFPFailure(nullptr);
        }
        case ChildType::kBlender: {
            // Convert a SkBlender into a child FP.
            auto childFP = GrFragmentProcessors::Make(as_BB(child.blender()),
                                                      /*srcFP=*/nullptr,
                                                      GrFragmentProcessor::DestColor(),
                                                      childArgs);
            return childFP ? GrFPSuccess(std::move(childFP))
                           : GrFPFailure(nullptr);
        }
    }

    SkUNREACHABLE;
}

static GrFPResult make_effect_fp(sk_sp<SkRuntimeEffect> effect,
                                 const char* name,
                                 sk_sp<const SkData> uniforms,
                                 std::unique_ptr<GrFragmentProcessor> inputFP,
                                 std::unique_ptr<GrFragmentProcessor> destColorFP,
                                 SkSpan<const SkRuntimeEffect::ChildPtr> children,
                                 const GrFPArgs& childArgs) {
    skia_private::STArray<8, std::unique_ptr<GrFragmentProcessor>> childFPs;
    for (const auto& child : children) {
        auto [success, childFP] = MakeChildFP(child, childArgs);
        if (!success) {
            return GrFPFailure(std::move(inputFP));
        }
        childFPs.push_back(std::move(childFP));
    }
    auto fp = GrSkSLFP::MakeWithData(std::move(effect),
                                     name,
                                     childArgs.fDstColorInfo->refColorSpace(),
                                     std::move(inputFP),
                                     std::move(destColorFP),
                                     std::move(uniforms),
                                     SkSpan(childFPs));
    SkASSERT(fp);
    return GrFPSuccess(std::move(fp));
}

static std::unique_ptr<GrFragmentProcessor> make_blender_fp(
        const SkRuntimeBlender* rtb,
        std::unique_ptr<GrFragmentProcessor> srcFP,
        std::unique_ptr<GrFragmentProcessor> dstFP,
        const GrFPArgs& fpArgs) {
    SkASSERT(rtb);
    if (!SkRuntimeEffectPriv::CanDraw(fpArgs.fContext->priv().caps(), rtb->effect().get())) {
        return nullptr;
    }

    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
            rtb->effect()->uniforms(),
            rtb->uniforms(),
            fpArgs.fDstColorInfo->colorSpace());
    SkASSERT(uniforms);
    GrFPArgs childArgs(fpArgs.fContext,
                       fpArgs.fDstColorInfo,
                       fpArgs.fSurfaceProps,
                       GrFPArgs::Scope::kRuntimeEffect);
    auto [success, fp] = make_effect_fp(rtb->effect(),
                                        "runtime_blender",
                                        std::move(uniforms),
                                        std::move(srcFP),
                                        std::move(dstFP),
                                        rtb->children(),
                                        childArgs);

    return success ? std::move(fp) : nullptr;
}

static std::unique_ptr<GrFragmentProcessor> make_blender_fp(
        const SkBlendModeBlender* blender,
        std::unique_ptr<GrFragmentProcessor> srcFP,
        std::unique_ptr<GrFragmentProcessor> dstFP,
        const GrFPArgs& fpArgs) {
    SkASSERT(blender);
    return GrBlendFragmentProcessor::Make(std::move(srcFP), std::move(dstFP), blender->mode());
}

std::unique_ptr<GrFragmentProcessor> Make(const SkBlenderBase* blender,
                                          std::unique_ptr<GrFragmentProcessor> srcFP,
                                          std::unique_ptr<GrFragmentProcessor> dstFP,
                                          const GrFPArgs& fpArgs) {
    if (!blender) {
        return nullptr;
    }
    switch (blender->type()) {
#define M(type)                                                                \
    case SkBlenderBase::BlenderType::k##type:                                  \
        return make_blender_fp(static_cast<const Sk##type##Blender*>(blender), \
                               std::move(srcFP),                               \
                               std::move(dstFP),                               \
                               fpArgs);
        SK_ALL_BLENDERS(M)
#undef M
    }
    SkUNREACHABLE;
}

static SkPMColor4f map_color(const SkColor4f& c, SkColorSpace* src, SkColorSpace* dst) {
    SkPMColor4f color = {c.fR, c.fG, c.fB, c.fA};
    SkColorSpaceXformSteps(src, kUnpremul_SkAlphaType, dst, kPremul_SkAlphaType).apply(color.vec());
    return color;
}
static GrFPResult make_colorfilter_fp(GrRecordingContext*,
                                      const SkBlendModeColorFilter* filter,
                                      std::unique_ptr<GrFragmentProcessor> inputFP,
                                      const GrColorInfo& dstColorInfo,
                                      const SkSurfaceProps& props) {
    if (filter->mode() == SkBlendMode::kDst) {
        // If the blend mode is "dest," the blend color won't factor into it at all.
        // We can return the input FP as-is.
        return GrFPSuccess(std::move(inputFP));
    }

    SkDEBUGCODE(const bool fpHasConstIO = !inputFP || inputFP->hasConstantOutputForConstantInput();)

    SkPMColor4f color = map_color(filter->color(), sk_srgb_singleton(), dstColorInfo.colorSpace());

    auto colorFP = GrFragmentProcessor::MakeColor(color);
    auto xferFP =
            GrBlendFragmentProcessor::Make(std::move(colorFP), std::move(inputFP), filter->mode());

    if (xferFP == nullptr) {
        // This is only expected to happen if the blend mode is "dest" and the input FP is null.
        // Since we already did an early-out in the "dest" blend mode case, we shouldn't get here.
        SkDEBUGFAIL("GrBlendFragmentProcessor::Make returned null unexpectedly");
        return GrFPFailure(nullptr);
    }

    // With a solid color input this should always be able to compute the blended color
    // (at least for coeff modes).
    // Occasionally, we even do better than we started; specifically, in "src" blend mode, we end up
    // ditching the input FP entirely, which turns a non-constant operation into a constant one.
    SkASSERT(filter->mode() > SkBlendMode::kLastCoeffMode ||
             xferFP->hasConstantOutputForConstantInput() >= fpHasConstIO);

    return GrFPSuccess(std::move(xferFP));
}

static GrFPResult make_colorfilter_fp(GrRecordingContext* context,
                                      const SkComposeColorFilter* filter,
                                      std::unique_ptr<GrFragmentProcessor> inputFP,
                                      const GrColorInfo& dstColorInfo,
                                      const SkSurfaceProps& props) {
    // Unfortunately, we need to clone the input before we know we need it. This lets us return
    // the original FP if either internal color filter fails.
    auto inputClone = inputFP ? inputFP->clone() : nullptr;

    auto [innerSuccess, innerFP] =
            Make(context, filter->inner().get(), std::move(inputFP), dstColorInfo, props);
    if (!innerSuccess) {
        return GrFPFailure(std::move(inputClone));
    }

    auto [outerSuccess, outerFP] =
            Make(context, filter->outer().get(), std::move(innerFP), dstColorInfo, props);
    if (!outerSuccess) {
        return GrFPFailure(std::move(inputClone));
    }

    return GrFPSuccess(std::move(outerFP));
}

static GrFPResult make_colorfilter_fp(GrRecordingContext*,
                                      const SkColorSpaceXformColorFilter* filter,
                                      std::unique_ptr<GrFragmentProcessor> inputFP,
                                      const GrColorInfo&,
                                      const SkSurfaceProps&) {
    // wish our caller would let us know if our input was opaque...
    constexpr SkAlphaType alphaType = kPremul_SkAlphaType;
    return GrFPSuccess(GrColorSpaceXformEffect::Make(
            std::move(inputFP), filter->src().get(), alphaType, filter->dst().get(), alphaType));
}

static GrFPResult make_colorfilter_fp(GrRecordingContext*,
                                      const SkGaussianColorFilter*,
                                      std::unique_ptr<GrFragmentProcessor> inputFP,
                                      const GrColorInfo&,
                                      const SkSurfaceProps&) {
    static const SkRuntimeEffect* effect =
            SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                "half4 main(half4 inColor) {"
                                "half factor = 1 - inColor.a;"
                                "factor = exp(-factor * factor * 4) - 0.018;"
                                "return half4(factor);"
                                "}");
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
    return GrFPSuccess(
            GrSkSLFP::Make(effect, "gaussian_fp", std::move(inputFP), GrSkSLFP::OptFlags::kNone));
}

static std::unique_ptr<GrFragmentProcessor> rgb_to_hsl(std::unique_ptr<GrFragmentProcessor> child) {
    static const SkRuntimeEffect* effect =
            SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                "half4 main(half4 color) {"
                                "return $rgb_to_hsl(color.rgb, color.a);"
                                "}");
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
    return GrSkSLFP::Make(
            effect, "RgbToHsl", std::move(child), GrSkSLFP::OptFlags::kPreservesOpaqueInput);
}

static std::unique_ptr<GrFragmentProcessor> hsl_to_rgb(std::unique_ptr<GrFragmentProcessor> child) {
    static const SkRuntimeEffect* effect =
            SkMakeRuntimeEffect(SkRuntimeEffect::MakeForColorFilter,
                                "half4 main(half4 color) {"
                                "return $hsl_to_rgb(color.rgb, color.a);"
                                "}");
    SkASSERT(SkRuntimeEffectPriv::SupportsConstantOutputForConstantInput(effect));
    return GrSkSLFP::Make(
            effect, "HslToRgb", std::move(child), GrSkSLFP::OptFlags::kPreservesOpaqueInput);
}

static GrFPResult make_colorfilter_fp(GrRecordingContext*,
                                      const SkMatrixColorFilter* filter,
                                      std::unique_ptr<GrFragmentProcessor> inputFP,
                                      const GrColorInfo&,
                                      const SkSurfaceProps&) {
    switch (filter->domain()) {
        case SkMatrixColorFilter::Domain::kRGBA:
            return GrFPSuccess(GrFragmentProcessor::ColorMatrix(std::move(inputFP),
                                                                filter->matrix(),
                                                                /* unpremulInput = */ true,
                                                                /* clampRGBOutput = */ true,
                                                                /* premulOutput = */ true));

        case SkMatrixColorFilter::Domain::kHSLA: {
            auto fp = rgb_to_hsl(std::move(inputFP));
            fp = GrFragmentProcessor::ColorMatrix(std::move(fp),
                                                  filter->matrix(),
                                                  /* unpremulInput = */ false,
                                                  /* clampRGBOutput = */ false,
                                                  /* premulOutput = */ false);
            return GrFPSuccess(hsl_to_rgb(std::move(fp)));
        }
    }
    SkUNREACHABLE;
}

static GrFPResult make_colorfilter_fp(GrRecordingContext* context,
                                      const SkRuntimeColorFilter* filter,
                                      std::unique_ptr<GrFragmentProcessor> inputFP,
                                      const GrColorInfo& colorInfo,
                                      const SkSurfaceProps& props) {
    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
            filter->effect()->uniforms(), filter->uniforms(), colorInfo.colorSpace());
    SkASSERT(uniforms);

    GrFPArgs childArgs(context, &colorInfo, props, GrFPArgs::Scope::kRuntimeEffect);
    return make_effect_fp(filter->effect(),
                          "runtime_color_filter",
                          std::move(uniforms),
                          std::move(inputFP),
                          /*destColorFP=*/nullptr,
                          filter->children(),
                          childArgs);
}

static GrFPResult make_colorfilter_fp(GrRecordingContext* context,
                                      const SkTableColorFilter* filter,
                                      std::unique_ptr<GrFragmentProcessor> inputFP,
                                      const GrColorInfo&,
                                      const SkSurfaceProps&) {
    auto cte = ColorTableEffect::Make(std::move(inputFP), context, filter->bitmap());
    return cte ? GrFPSuccess(std::move(cte)) : GrFPFailure(nullptr);
}

static GrFPResult make_colorfilter_fp(GrRecordingContext* context,
                                      const SkWorkingFormatColorFilter* filter,
                                      std::unique_ptr<GrFragmentProcessor> inputFP,
                                      const GrColorInfo& dstColorInfo,
                                      const SkSurfaceProps& props) {
    sk_sp<SkColorSpace> dstCS = dstColorInfo.refColorSpace();
    if (!dstCS) {
        dstCS = SkColorSpace::MakeSRGB();
    }

    SkAlphaType workingAT;
    sk_sp<SkColorSpace> workingCS = filter->workingFormat(dstCS, &workingAT);

    GrColorInfo dst = {dstColorInfo.colorType(), dstColorInfo.alphaType(), dstCS},
                working = {dstColorInfo.colorType(), workingAT, workingCS};

    auto [ok, fp] = Make(context,
                         filter->child().get(),
                         GrColorSpaceXformEffect::Make(std::move(inputFP), dst, working),
                         working,
                         props);

    return ok ? GrFPSuccess(GrColorSpaceXformEffect::Make(std::move(fp), working, dst))
              : GrFPFailure(std::move(fp));
}

GrFPResult Make(GrRecordingContext* ctx,
                const SkColorFilter* cf,
                std::unique_ptr<GrFragmentProcessor> inputFP,
                const GrColorInfo& dstColorInfo,
                const SkSurfaceProps& props) {
    if (!cf) {
        return GrFPFailure(nullptr);
    }
    auto cfb = as_CFB(cf);
    switch (cfb->type()) {
        case SkColorFilterBase::Type::kNoop:
            return GrFPFailure(nullptr);
#define M(type)                                                                   \
    case SkColorFilterBase::Type::k##type:                                        \
        return make_colorfilter_fp(ctx,                                           \
                                   static_cast<const Sk##type##ColorFilter*>(cf), \
                                   std::move(inputFP),                            \
                                   dstColorInfo,                                  \
                                   props);
            SK_ALL_COLOR_FILTERS(M)
#undef M
    }
    SkUNREACHABLE;
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkBlendShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    auto fpA = Make(shader->dst().get(), args, mRec);
    auto fpB = Make(shader->src().get(), args, mRec);
    if (!fpA || !fpB) {
        // This is unexpected. Both src and dst shaders should be valid. Just fail.
        return nullptr;
    }
    return GrBlendFragmentProcessor::Make(std::move(fpB), std::move(fpA), shader->mode());
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkColorFilterShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    auto shaderFP = Make(shader->shader().get(), args, mRec);
    if (!shaderFP) {
        return nullptr;
    }

    // TODO I guess, but it shouldn't come up as used today.
    SkASSERT(shader->alpha() == 1.0f);

    auto [success, fp] = Make(args.fContext,
                              shader->filter().get(),
                              std::move(shaderFP),
                              *args.fDstColorInfo,
                              args.fSurfaceProps);
    // If the filter FP could not be created, we still want to return the shader FP, so checking
    // success can be omitted here.
    return std::move(fp);
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkColorShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    return GrFragmentProcessor::MakeColor(SkColorToPMColor4f(shader->color(), *args.fDstColorInfo));
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkColor4Shader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    SkColorSpaceXformSteps steps{shader->colorSpace().get(),
                                 kUnpremul_SkAlphaType,
                                 args.fDstColorInfo->colorSpace(),
                                 kUnpremul_SkAlphaType};
    SkColor4f color = shader->color();
    steps.apply(color.vec());
    return GrFragmentProcessor::MakeColor(color.premul());
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkCoordClampShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    static const SkRuntimeEffect* effect =
            SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                "uniform shader c;"
                                "uniform float4 s;"
                                "half4 main(float2 p) {"
                                    "return c.eval(clamp(p, s.LT, s.RB));"
                                "}");

    auto fp = Make(shader->shader().get(), args, mRec.applied());
    if (!fp) {
        return nullptr;
    }

    GrSkSLFP::OptFlags flags = GrSkSLFP::OptFlags::kNone;
    if (fp->compatibleWithCoverageAsAlpha()) {
        flags |= GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha;
    }
    if (fp->preservesOpaqueInput()) {
        flags |= GrSkSLFP::OptFlags::kPreservesOpaqueInput;
    }
    fp = GrSkSLFP::Make(effect,
                        "clamp_fp",
                        /*inputFP=*/nullptr,
                        flags,
                        "c",
                        std::move(fp),
                        "s",
                        shader->subset());

    auto [total, ok] = mRec.applyForFragmentProcessor({});
    if (!ok) {
        return nullptr;
    }
    return GrMatrixEffect::Make(total, std::move(fp));
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkCTMShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    SkMatrix ctmInv;
    if (!shader->ctm().invert(&ctmInv)) {
        return nullptr;
    }

    auto base = Make(shader->proxyShader().get(), args, shader->ctm());
    if (!base) {
        return nullptr;
    }

    // In order for the shader to be evaluated with the original CTM, we explicitly evaluate it
    // at sk_FragCoord, and pass that through the inverse of the original CTM. This avoids requiring
    // local coords for the shader and mapping from the draw's local to device and then back.
    return GrFragmentProcessor::DeviceSpace(GrMatrixEffect::Make(ctmInv, std::move(base)));
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkEmptyShader* shader,
                                                           const GrFPArgs&,
                                                           const SkShaders::MatrixRec&) {
    return nullptr;
}

static bool needs_subset(sk_sp<const SkImage> img, const SkRect& subset) {
    return subset != SkRect::Make(img->dimensions());
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkImageShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    SkTileMode tileModes[2] = {shader->tileModeX(), shader->tileModeY()};
    const SkRect shaderSubset = shader->subset();
    const SkRect* subset = needs_subset(shader->image(), shaderSubset) ? &shaderSubset : nullptr;
    auto fp = skgpu::ganesh::AsFragmentProcessor(
            args.fContext, shader->image(), shader->sampling(), tileModes, SkMatrix::I(), subset);
    if (!fp) {
        return nullptr;
    }

    auto [total, ok] = mRec.applyForFragmentProcessor({});
    if (!ok) {
        return nullptr;
    }
    fp = GrMatrixEffect::Make(total, std::move(fp));

    if (!shader->isRaw()) {
        fp = GrColorSpaceXformEffect::Make(std::move(fp),
                                           shader->image()->colorSpace(),
                                           shader->image()->alphaType(),
                                           args.fDstColorInfo->colorSpace(),
                                           kPremul_SkAlphaType);

        // Alpha-only image shaders are tinted by the input color (typically the paint color).
        // We suppress that behavior when sampled from a runtime effect.
        if (shader->image()->isAlphaOnly() && args.fScope != GrFPArgs::Scope::kRuntimeEffect) {
            fp = GrBlendFragmentProcessor::Make<SkBlendMode::kDstIn>(std::move(fp), nullptr);
        }
    }

    return fp;
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkLocalMatrixShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    return Make(shader->wrappedShader().get(), args, mRec.concat(shader->localMatrix()));
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkPerlinNoiseShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    SkASSERT(args.fContext);
    SkASSERT(shader->numOctaves());

    const SkMatrix& totalMatrix = mRec.totalMatrix();

    // Either we don't stitch tiles, or we have a valid tile size
    SkASSERT(!shader->stitchTiles() || !shader->tileSize().isEmpty());

    auto paintingData = shader->getPaintingData(totalMatrix);
    paintingData->generateBitmaps();

    // Like shadeSpan, we start from device space. We will account for that below with a device
    // space effect.

    auto context = args.fContext;

    const SkBitmap& permutationsBitmap = paintingData->getPermutationsBitmap();
    const SkBitmap& noiseBitmap = paintingData->getNoiseBitmap();

    auto permutationsView = std::get<0>(GrMakeCachedBitmapProxyView(
            context,
            permutationsBitmap,
            /*label=*/"PerlinNoiseShader_FragmentProcessor_PermutationsView"));
    auto noiseView = std::get<0>(GrMakeCachedBitmapProxyView(
            context, noiseBitmap, /*label=*/"PerlinNoiseShader_FragmentProcessor_NoiseView"));

    if (permutationsView && noiseView) {
        return GrFragmentProcessor::DeviceSpace(
                GrMatrixEffect::Make(SkMatrix::Translate(1 - totalMatrix.getTranslateX(),
                                                         1 - totalMatrix.getTranslateY()),
                                     GrPerlinNoise2Effect::Make(shader->noiseType(),
                                                                shader->numOctaves(),
                                                                shader->stitchTiles(),
                                                                std::move(paintingData),
                                                                std::move(permutationsView),
                                                                std::move(noiseView),
                                                                *context->priv().caps())));
    }
    return nullptr;
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkPictureShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    auto ctx = args.fContext;
    SkColorType dstColorType = GrColorTypeToSkColorType(args.fDstColorInfo->colorType());
    if (dstColorType == kUnknown_SkColorType) {
        dstColorType = kRGBA_8888_SkColorType;
    }
    sk_sp<SkColorSpace> dstCS = SkColorSpace::MakeSRGB();
    if (args.fDstColorInfo->colorSpace()) {
        dstCS = sk_ref_sp(args.fDstColorInfo->colorSpace());
    }

    auto info = SkPictureShader::CachedImageInfo::Make(shader->tile(),
                                                       mRec.totalMatrix(),
                                                       dstColorType,
                                                       dstCS.get(),
                                                       ctx->priv().caps()->maxTextureSize(),
                                                       args.fSurfaceProps);
    if (!info.success) {
        return nullptr;
    }

    // Gotta be sure the GPU can support our requested colortype (might be FP16)
    if (!ctx->colorTypeSupportedAsSurface(info.imageInfo.colorType())) {
        info.imageInfo = info.imageInfo.makeColorType(kRGBA_8888_SkColorType);
    }

    static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey key;
    std::tuple keyData = {dstCS->toXYZD50Hash(),
                          dstCS->transferFnHash(),
                          static_cast<uint32_t>(dstColorType),
                          shader->picture()->uniqueID(),
                          shader->tile(),
                          info.tileScale,
                          info.props};
    skgpu::UniqueKey::Builder builder(
            &key, kDomain, sizeof(keyData) / sizeof(uint32_t), "Picture Shader Image");
    memcpy(&builder[0], &keyData, sizeof(keyData));
    builder.finish();

    GrProxyProvider* provider = ctx->priv().proxyProvider();
    GrSurfaceProxyView view;
    if (auto proxy = provider->findOrCreateProxyByUniqueKey(key)) {
        view = GrSurfaceProxyView(proxy, kTopLeft_GrSurfaceOrigin, skgpu::Swizzle());
    } else {
        const int msaaSampleCount = 0;
        const bool createWithMips = false;
        const bool kUnprotected = false;
        auto image = info.makeImage(SkSurfaces::RenderTarget(ctx,
                                                             skgpu::Budgeted::kYes,
                                                             info.imageInfo,
                                                             msaaSampleCount,
                                                             kTopLeft_GrSurfaceOrigin,
                                                             &info.props,
                                                             createWithMips,
                                                             kUnprotected),
                                    shader->picture().get());
        if (!image) {
            return nullptr;
        }

        auto [v, ct] = skgpu::ganesh::AsView(ctx, image, skgpu::Mipmapped::kNo);
        view = std::move(v);
        provider->assignUniqueKeyToProxy(key, view.asTextureProxy());
    }

    const GrSamplerState sampler(static_cast<GrSamplerState::WrapMode>(shader->tileModeX()),
                                 static_cast<GrSamplerState::WrapMode>(shader->tileModeY()),
                                 shader->filter());
    auto fp = GrTextureEffect::Make(
            std::move(view), kPremul_SkAlphaType, SkMatrix::I(), sampler, *ctx->priv().caps());
    SkMatrix scale = SkMatrix::Scale(info.tileScale.width(), info.tileScale.height());
    auto [total, ok] = mRec.applyForFragmentProcessor(scale);
    if (!ok) {
        return nullptr;
    }
    return GrMatrixEffect::Make(total, std::move(fp));
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkRuntimeShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    if (!SkRuntimeEffectPriv::CanDraw(args.fContext->priv().caps(), shader->asRuntimeEffect())) {
        return nullptr;
    }

    sk_sp<const SkData> uniforms = SkRuntimeEffectPriv::TransformUniforms(
            shader->asRuntimeEffect()->uniforms(),
            shader->uniformData(args.fDstColorInfo->colorSpace()),
            args.fDstColorInfo->colorSpace());
    SkASSERT(uniforms);

    bool success;
    std::unique_ptr<GrFragmentProcessor> fp;
    GrFPArgs childArgs(
            args.fContext, args.fDstColorInfo, args.fSurfaceProps, GrFPArgs::Scope::kRuntimeEffect);
    std::tie(success, fp) = make_effect_fp(shader->effect(),
                                           "runtime_shader",
                                           std::move(uniforms),
                                           /*inputFP=*/nullptr,
                                           /*destColorFP=*/nullptr,
                                           shader->children(),
                                           childArgs);
    if (!success) {
        return nullptr;
    }

    auto [total, ok] = mRec.applyForFragmentProcessor({});
    if (!ok) {
        return nullptr;
    }
    return GrMatrixEffect::Make(total, std::move(fp));
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkTransformShader* shader,
                                                           const GrFPArgs&,
                                                           const SkShaders::MatrixRec&) {
    return nullptr;
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkTriColorShader* shader,
                                                           const GrFPArgs&,
                                                           const SkShaders::MatrixRec&) {
    return nullptr;
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkWorkingColorSpaceShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    const GrColorInfo* dstInfo = args.fDstColorInfo;
    sk_sp<SkColorSpace> dstCS = dstInfo->refColorSpace();
    if (!dstCS) {
        dstCS = SkColorSpace::MakeSRGB();
    }

    GrColorInfo dst     = {dstInfo->colorType(), dstInfo->alphaType(), dstCS},
                working = {dstInfo->colorType(), dstInfo->alphaType(), shader->workingSpace()};
    GrFPArgs workingArgs(args.fContext, &working, args.fSurfaceProps, args.fScope);

    auto childFP = Make(shader->shader().get(), workingArgs, mRec);
    if (!childFP) {
        return nullptr;
    }

    auto childWithWorkingInput = GrFragmentProcessor::Compose(
            std::move(childFP), GrColorSpaceXformEffect::Make(nullptr, dst, working));

    return GrColorSpaceXformEffect::Make(std::move(childWithWorkingInput), working, dst);
}

//////////////////////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<GrFragmentProcessor> make_gradient_fp(const SkConicalGradient* shader,
                                                             const GrFPArgs& args,
                                                             const SkShaders::MatrixRec& mRec) {
    // The 2 point conical gradient can reject a pixel so it does change opacity even if the input
    // was opaque. Thus, all of these layout FPs disable that optimization.
    std::unique_ptr<GrFragmentProcessor> fp;
    SkTLazy<SkMatrix> matrix;
    switch (shader->getType()) {
        case SkConicalGradient::Type::kStrip: {
            static const SkRuntimeEffect* kEffect =
                SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                        "uniform half r0_2;"
                        "half4 main(float2 p) {"
                            // validation flag, set to negative to discard fragment later.
                            "half v = 1;"
                            "float t = r0_2 - p.y * p.y;"
                            "if (t >= 0) {"
                                "t = p.x + sqrt(t);"
                            "} else {"
                                "v = -1;"
                            "}"
                            "return half4(half(t), v, 0, 0);"
                        "}"
                    );
            float r0 = shader->getStartRadius() / shader->getCenterX1();
            fp = GrSkSLFP::Make(kEffect,
                                "TwoPointConicalStripLayout",
                                /*inputFP=*/nullptr,
                                GrSkSLFP::OptFlags::kNone,
                                "r0_2",
                                r0 * r0);
        } break;

        case SkConicalGradient::Type::kRadial: {
            static const SkRuntimeEffect* kEffect =
                SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                        "uniform half r0;"
                        "uniform half lengthScale;"
                        "half4 main(float2 p) {"
                            // validation flag, set to negative to discard fragment later
                            "half v = 1;"
                            "float t = length(p) * lengthScale - r0;"
                            "return half4(half(t), v, 0, 0);"
                        "}"
                    );
            float dr = shader->getDiffRadius();
            float r0 = shader->getStartRadius() / dr;
            bool isRadiusIncreasing = dr >= 0;
            fp = GrSkSLFP::Make(kEffect,
                                "TwoPointConicalRadialLayout",
                                /*inputFP=*/nullptr,
                                GrSkSLFP::OptFlags::kNone,
                                "r0",
                                r0,
                                "lengthScale",
                                isRadiusIncreasing ? 1.0f : -1.0f);

            // GPU radial matrix is different from the original matrix, since we map the diff radius
            // to have |dr| = 1, so manually compute the final gradient matrix here.

            // Map center to (0, 0)
            matrix.set(SkMatrix::Translate(-shader->getStartCenter().fX,
                                           -shader->getStartCenter().fY));
            // scale |diffRadius| to 1
            matrix->postScale(1 / dr, 1 / dr);
        } break;

        case SkConicalGradient::Type::kFocal: {
            static const SkRuntimeEffect* kEffect =
                SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                        // Optimization flags, all specialized:
                        "uniform int isRadiusIncreasing;"
                        "uniform int isFocalOnCircle;"
                        "uniform int isWellBehaved;"
                        "uniform int isSwapped;"
                        "uniform int isNativelyFocal;"

                        "uniform half invR1;"  // 1/r1
                        "uniform half fx;"     // focalX = r0/(r0-r1)

                        "half4 main(float2 p) {"
                            "float t = -1;"
                            "half v = 1;" // validation flag,set to negative to discard fragment later

                            "float x_t = -1;"
                            "if (bool(isFocalOnCircle)) {"
                                "x_t = dot(p, p) / p.x;"
                            "} else if (bool(isWellBehaved)) {"
                                "x_t = length(p) - p.x * invR1;"
                            "} else {"
                                "float temp = p.x * p.x - p.y * p.y;"

                                // Only do sqrt if temp >= 0; this is significantly slower than
                                // checking temp >= 0 in the if statement that checks r(t) >= 0.
                                // But GPU may break if we sqrt a negative float. (Although I
                                // haven't observed that on any devices so far, and the old
                                // approach also does sqrt negative value without a check.) If
                                // the performance is really critical, maybe we should just
                                // compute the area where temp and x_t are always valid and drop
                                // all these ifs.
                                "if (temp >= 0) {"
                                    "if (bool(isSwapped) || !bool(isRadiusIncreasing)) {"
                                        "x_t = -sqrt(temp) - p.x * invR1;"
                                    "} else {"
                                        "x_t = sqrt(temp) - p.x * invR1;"
                                    "}"
                                "}"
                            "}"

                            // The final calculation of t from x_t has lots of static
                            // optimizations but only do them when x_t is positive (which
                            // can be assumed true if isWellBehaved is true)
                            "if (!bool(isWellBehaved)) {"
                                // This will still calculate t even though it will be ignored
                                // later in the pipeline to avoid a branch
                                "if (x_t <= 0.0) {"
                                    "v = -1;"
                                "}"
                            "}"
                            "if (bool(isRadiusIncreasing)) {"
                                "if (bool(isNativelyFocal)) {"
                                    "t = x_t;"
                                "} else {"
                                    "t = x_t + fx;"
                                "}"
                            "} else {"
                                "if (bool(isNativelyFocal)) {"
                                    "t = -x_t;"
                                "} else {"
                                    "t = -x_t + fx;"
                                "}"
                            "}"

                            "if (bool(isSwapped)) {"
                                "t = 1 - t;"
                            "}"

                            "return half4(half(t), v, 0, 0);"
                        "}"
                    );

            const SkConicalGradient::FocalData& focalData = shader->getFocalData();
            bool isRadiusIncreasing = (1 - focalData.fFocalX) > 0,
                 isFocalOnCircle = focalData.isFocalOnCircle(),
                 isWellBehaved = focalData.isWellBehaved(), isSwapped = focalData.isSwapped(),
                 isNativelyFocal = focalData.isNativelyFocal();

            fp = GrSkSLFP::Make(kEffect, "TwoPointConicalFocalLayout", /*inputFP=*/nullptr,
                                GrSkSLFP::OptFlags::kNone,
                                "isRadiusIncreasing", GrSkSLFP::Specialize<int>(isRadiusIncreasing),
                                "isFocalOnCircle",    GrSkSLFP::Specialize<int>(isFocalOnCircle),
                                "isWellBehaved",      GrSkSLFP::Specialize<int>(isWellBehaved),
                                "isSwapped",          GrSkSLFP::Specialize<int>(isSwapped),
                                "isNativelyFocal",    GrSkSLFP::Specialize<int>(isNativelyFocal),
                                "invR1",              1.0f / focalData.fR1,
                                "fx",                 focalData.fFocalX);
        } break;
    }
    return GrGradientShader::MakeGradientFP(
            *shader, args, mRec, std::move(fp), matrix.getMaybeNull());
}

static std::unique_ptr<GrFragmentProcessor> make_gradient_fp(const SkLinearGradient* shader,
                                                             const GrFPArgs& args,
                                                             const SkShaders::MatrixRec& mRec) {
    return GrGradientShader::MakeLinear(*shader, args, mRec);
}

static std::unique_ptr<GrFragmentProcessor> make_gradient_fp(const SkRadialGradient* shader,
                                                             const GrFPArgs& args,
                                                             const SkShaders::MatrixRec& mRec) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(
            SkRuntimeEffect::MakeForShader,
            "half4 main(float2 coord) {"
                "return half4(half(length(coord)), 1, 0, 0);"  // y = 1 for always valid
            "}");
    // The radial gradient never rejects a pixel so it doesn't change opacity
    auto fp = GrSkSLFP::Make(
            effect, "RadialLayout", /*inputFP=*/nullptr, GrSkSLFP::OptFlags::kPreservesOpaqueInput);
    return GrGradientShader::MakeGradientFP(*shader, args, mRec, std::move(fp));
}

static std::unique_ptr<GrFragmentProcessor> make_gradient_fp(const SkSweepGradient* shader,
                                                             const GrFPArgs& args,
                                                             const SkShaders::MatrixRec& mRec) {
    // On some devices they incorrectly implement atan2(y,x) as atan(y/x). In actuality it is
    // atan2(y,x) = 2 * atan(y / (sqrt(x^2 + y^2) + x)). So to work around this we pass in (sqrt(x^2
    // + y^2) + x) as the second parameter to atan2 in these cases. We let the device handle the
    // undefined behavior of the second paramenter being 0 instead of doing the divide ourselves and
    // using atan instead.
    int useAtanWorkaround =
            args.fContext->priv().caps()->shaderCaps()->fAtan2ImplementedAsAtanYOverX;
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform half bias;"
        "uniform half scale;"
        "uniform int useAtanWorkaround;"  // specialized

        "half4 main(float2 coord) {"
            "half angle;"
            "if (bool(useAtanWorkaround)) {"
                "angle = half(2 * atan(-coord.y, length(coord) - coord.x));"
            "} else {"
                // Hardcode pi/2 for the angle when x == 0, to avoid undefined behavior in this
                // case. This hasn't proven to be necessary in the atan workaround case.
                "angle = (coord.x != 0) ? half(atan(-coord.y, -coord.x)) :"
                                        " sign(coord.y) * -1.5707963267949;"
            "}"

            // 0.1591549430918 is 1/(2*pi), used since atan returns values [-pi, pi]
            "half t = (angle * 0.1591549430918 + 0.5 + bias) * scale;"
            "return half4(t, 1, 0, 0);" // y = 1 for always valid
        "}"
    );

    // The sweep gradient never rejects a pixel so it doesn't change opacity
    auto fp = GrSkSLFP::Make(effect, "SweepLayout", /*inputFP=*/nullptr,
                             GrSkSLFP::OptFlags::kPreservesOpaqueInput,
                             "bias", shader->tBias(),
                             "scale", shader->tScale(),
                             "useAtanWorkaround", GrSkSLFP::Specialize(useAtanWorkaround));
    return GrGradientShader::MakeGradientFP(*shader, args, mRec, std::move(fp));
}

static std::unique_ptr<GrFragmentProcessor> make_shader_fp(const SkGradientBaseShader* shader,
                                                           const GrFPArgs& args,
                                                           const SkShaders::MatrixRec& mRec) {
    SkASSERT(shader);

    switch (shader->asGradient()) {
#define M(type)                               \
    case SkShaderBase::GradientType::k##type: \
        return make_gradient_fp(static_cast<const Sk##type##Gradient*>(shader), args, mRec);
        SK_ALL_GRADIENTS(M)
#undef M
        case SkShaderBase::GradientType::kNone:
            SkDEBUGFAIL("Gradient shader says its type is none");
            return nullptr;
    }
    SkUNREACHABLE;
}

std::unique_ptr<GrFragmentProcessor> Make(const SkShader* shader,
                                          const GrFPArgs& args,
                                          const SkMatrix& ctm) {
    return Make(shader, args, SkShaders::MatrixRec(ctm));
}

std::unique_ptr<GrFragmentProcessor> Make(const SkShader* shader,
                                          const GrFPArgs& args,
                                          const SkShaders::MatrixRec& mRec) {
    if (!shader) {
        return nullptr;
    }
    auto base = as_SB(shader);
    switch (base->type()) {
#define M(type)                             \
    case SkShaderBase::ShaderType::k##type: \
        return make_shader_fp(static_cast<const Sk##type##Shader*>(base), args, mRec);
        SK_ALL_SHADERS(M)
#undef M
    }
    SkUNREACHABLE;
}

}  // namespace GrFragmentProcessors
