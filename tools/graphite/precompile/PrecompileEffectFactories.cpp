/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/graphite/precompile/PrecompileEffectFactories.h"

#include "include/effects/SkRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileBlender.h"
#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileRuntimeEffect.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"
#include "src/core/SkRuntimeEffectPriv.h"

namespace skiatest::graphite {

using namespace skgpu::graphite;

namespace PrecompileFactories {

//--------------------------------------------------------------------------------------------------
const char* GetAnnulusShaderCode() {
    static const char* sCode =
        // draw a annulus centered at "center" w/ inner and outer radii in "radii"
        "uniform float2 center;"
        "uniform float2 radii;"
        "half4 main(float2 xy) {"
            "float len = length(xy - center);"
            "half value = len < radii.x ? 0.0 : (len > radii.y ? 0.0 : 1.0);"
            "return half4(value);"
        "}";

    return sCode;
}

SkRuntimeEffect* GetAnnulusShaderEffect() {
    static SkRuntimeEffect* sEffect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
                                                          GetAnnulusShaderCode());

    return sEffect;
}

std::pair<sk_sp<SkShader>, sk_sp<PrecompileShader>> CreateAnnulusRuntimeShader() {
    SkRuntimeEffect* effect = GetAnnulusShaderEffect();

    static const float kUniforms[4] = { 50.0f, 50.0f, 40.0f, 50.0f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));

    sk_sp<SkShader> s = effect->makeShader(std::move(uniforms), /* children= */ {});
    sk_sp<PrecompileShader> o = PrecompileRuntimeEffects::MakePrecompileShader(sk_ref_sp(effect));
    return { std::move(s), std::move(o) };
}

//--------------------------------------------------------------------------------------------------
SkRuntimeEffect* GetSrcBlenderEffect() {
    static SkRuntimeEffect* sEffect = SkMakeRuntimeEffect(
        SkRuntimeEffect::MakeForBlender,
        "half4 main(half4 src, half4 dst) {"
            "return src;"
        "}");

    return sEffect;
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> CreateSrcRuntimeBlender() {
    SkRuntimeEffect* effect = GetSrcBlenderEffect();

    sk_sp<SkBlender> b = effect->makeBlender(/* uniforms= */ nullptr);
    sk_sp<PrecompileBlender> o =
            PrecompileRuntimeEffects::MakePrecompileBlender(sk_ref_sp(effect));
    return { std::move(b) , std::move(o) };
}

SkRuntimeEffect* GetDstBlenderEffect() {
    static SkRuntimeEffect* sEffect = SkMakeRuntimeEffect(
        SkRuntimeEffect::MakeForBlender,
        "half4 main(half4 src, half4 dst) {"
            "return dst;"
        "}");

    return sEffect;
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> CreateDstRuntimeBlender() {
    SkRuntimeEffect* effect = GetDstBlenderEffect();

    sk_sp<SkBlender> b = effect->makeBlender(/* uniforms= */ nullptr);
    sk_sp<PrecompileBlender> o =
            PrecompileRuntimeEffects::MakePrecompileBlender(sk_ref_sp(effect));
    return { std::move(b) , std::move(o) };
}

SkRuntimeEffect* GetComboBlenderEffect() {
    static SkRuntimeEffect* sEffect = SkMakeRuntimeEffect(
        SkRuntimeEffect::MakeForBlender,
        "uniform float blendFrac;"
        "uniform blender a;"
        "uniform blender b;"
        "half4 main(half4 src, half4 dst) {"
            "return (blendFrac * a.eval(src, dst)) + ((1 - blendFrac) * b.eval(src, dst));"
        "}");

    return sEffect;
}

std::pair<sk_sp<SkBlender>, sk_sp<PrecompileBlender>> CreateComboRuntimeBlender() {
    SkRuntimeEffect* effect = GetComboBlenderEffect();

    auto [src, srcO] = CreateSrcRuntimeBlender();
    auto [dst, dstO] = CreateDstRuntimeBlender();

    SkRuntimeEffect::ChildPtr children[] = { src, dst };

    const float kUniforms[] = { 1.0f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));
    sk_sp<SkBlender> b = effect->makeBlender(std::move(uniforms), children);
    sk_sp<PrecompileBlender> o = PrecompileRuntimeEffects::MakePrecompileBlender(
            sk_ref_sp(effect),
            { { srcO }, { dstO } });
    return { std::move(b) , std::move(o) };
}

//--------------------------------------------------------------------------------------------------
SkRuntimeEffect* GetDoubleColorFilterEffect() {
    static SkRuntimeEffect* sEffect = SkMakeRuntimeEffect(
        SkRuntimeEffect::MakeForColorFilter,
        "half4 main(half4 c) {"
            "return 2*c;"
        "}");

    return sEffect;
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> CreateDoubleRuntimeColorFilter() {
    SkRuntimeEffect* effect = GetDoubleColorFilterEffect();

    return { effect->makeColorFilter(/* uniforms= */ nullptr),
             PrecompileRuntimeEffects::MakePrecompileColorFilter(sk_ref_sp(effect)) };
}

SkRuntimeEffect* GetHalfColorFilterEffect() {
    static SkRuntimeEffect* sEffect = SkMakeRuntimeEffect(
        SkRuntimeEffect::MakeForColorFilter,
        "half4 main(half4 c) {"
            "return 0.5*c;"
        "}");

    return sEffect;
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> CreateHalfRuntimeColorFilter() {
    SkRuntimeEffect* effect = GetHalfColorFilterEffect();

    return { effect->makeColorFilter(/* uniforms= */ nullptr),
             PrecompileRuntimeEffects::MakePrecompileColorFilter(sk_ref_sp(effect)) };
}

SkRuntimeEffect* GetComboColorFilterEffect() {
    static SkRuntimeEffect* sEffect = SkMakeRuntimeEffect(
        SkRuntimeEffect::MakeForColorFilter,
        "uniform float blendFrac;"
        "uniform colorFilter a;"
        "uniform colorFilter b;"
        "half4 main(half4 c) {"
            "return (blendFrac * a.eval(c)) + ((1 - blendFrac) * b.eval(c));"
        "}");

    return sEffect;
}

std::pair<sk_sp<SkColorFilter>, sk_sp<PrecompileColorFilter>> CreateComboRuntimeColorFilter() {
    SkRuntimeEffect* effect = GetComboColorFilterEffect();

    auto [src, srcO] = CreateDoubleRuntimeColorFilter();
    auto [dst, dstO] = CreateHalfRuntimeColorFilter();

    SkRuntimeEffect::ChildPtr children[] = { src, dst };

    const float kUniforms[] = { 0.5f };

    sk_sp<SkData> uniforms = SkData::MakeWithCopy(kUniforms, sizeof(kUniforms));
    sk_sp<SkColorFilter> cf = effect->makeColorFilter(std::move(uniforms), children);
    sk_sp<PrecompileColorFilter> o =
            PrecompileRuntimeEffects::MakePrecompileColorFilter(sk_ref_sp(effect),
                                                                { { srcO }, { dstO } });
    return { std::move(cf) , std::move(o) };
}

} // namespace PrecompileFactories

} // namespace skiatest::graphite
