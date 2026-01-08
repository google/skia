/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/graphite/precompile/AndroidRuntimeEffectManager.h"

#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"

namespace {

// Note: passing in a name to 'makeEffect' is a difference from Android's factory functions.
sk_sp<SkRuntimeEffect> makeEffect(const SkString& sksl, const char* name) {
    SkRuntimeEffect::Options options;
    options.fName = name;

    auto [effect, error] = SkRuntimeEffect::MakeForShader(sksl, options);
    if (!effect) {
        SkDebugf("%s\n", error.c_str());
    }
    return effect;
}

} // anonymous namespace

// All the SkSL snippets in this constructor are just stubs for the Android code.
// For Skia's testing purposes they only need to match the real code wrt:
//   name
//   number of child shaders
//   access method for child shaders (direct sample "eval(xy)" vs indirect "eval(modified-xy)")
RuntimeEffectManager::RuntimeEffectManager() {
    static const SkString kBlurFilter_MixEffectCode(R"(
        uniform shader img1;
        uniform shader img2;

        half4 main(float2 xy) {
            return half4(mix(img1.eval(xy), img2.eval(xy), 0.5)).rgb1;
        }
    )");

    static const SkString kEdgeExtensionEffectCode(R"(
        uniform shader img;

        vec4 main(vec2 xy) {
            float3 sample = img.eval(0.115 * xy).rgb;
            return float4(sample, 1.0);
        }
    )");

    static const SkString kGainmapEffectCode(R"(
        uniform shader img1;
        uniform shader img2;

        vec4 main(vec2 xy) {
            float3 sample = img1.eval(xy).rgb + img2.eval(xy).rgb;
            return float4(sample, 1.0);
        }
    )");

    static const SkString kKawaseBlurDualFilterV2_QuarterResDownSampleBlurEffectCode(R"(
        uniform shader img;

        half4 main(float2 xy) {
            half3 c = img.eval(0.6 * xy).rgb;
            return half4(c * 0.5, 1.0);
        }
    )");

    static const SkString kKawaseBlurDualFilterV2_HalfResDownSampleBlurEffectCode(R"(
        uniform shader img;

        half4 main(float2 xy) {
            half3 c = img.eval(0.55 * xy).rgb;
            return half4(c, 1.0);
        }
    )");

    static const SkString kKawaseBlurDualFilterV2_UpSampleBlurEffectCode(R"(
        uniform shader img;

        half4 main(float2 xy) {
            half3 c = img.eval(0.55 * xy).rgb;
            return half4(c, 1.0);
        }
    )");

    static const SkString kKawaseBlurEffectCode(R"(
        uniform shader img;

        half4 main(float2 xy) {
            half3 c = img.eval(0.55 * xy).rgb;
            return half4(c, 1.0);
        }
    )");

    static const SkString kLutEffectCode(R"(
        uniform shader img;
        uniform shader lut;

        half4 main(float2 xy) {
            half3 c = img.eval(xy).rgb;
            float rV = lut.eval(vec2(255, 0)).r;
            float gV = lut.eval(vec2(128, 0)).r;
            float bV = lut.eval(vec2(64, 0)).r;
            return half4(rV+c.r, gV+c.g, bV+c.b, 1.0);
        }
    )");

    static const SkString kMouriMap_CrossTalkAndChunk16x16EffectCode(R"(
        uniform shader img;
        vec4 main(vec2 xy) {
            float3 linear = img.eval(0.25 * xy).rgb;
            return float4(linear, 1.0);
        }
    )");

    static const SkString kMouriMap_Chunk8x8EffectCode(R"(
        uniform shader img;
        vec4 main(vec2 xy) {
            return float4(img.eval(0.33 * xy).rgb, 1.0);
        }
    )");

    static const SkString kMouriMap_BlurEffectCode(R"(
        uniform shader img;
        vec4 main(vec2 xy) {
            return float4(img.eval(0.4 * xy).rgb, 0.0);
        }
    )");

    static const SkString kMouriMap_TonemapEffectCode(R"(
        uniform shader image;
        uniform shader lux;
        vec4 main(vec2 xy) {
            float localMax = lux.eval(xy * 0.4).r;
            float4 rgba = image.eval(xy);
            float3 linear = rgba.rgb * 0.7;

            return float4(linear, rgba.a);
        }
    )");

    static const SkString kStretchEffectCode(R"(
        uniform shader image;

        vec4 main(vec2 xy) {
            float4 rgba = image.eval(0.5 * xy);

            return rgba;
        }
    )");

    static const SkString kBoxShadowEffectCode(R"(
        vec4 main(vec2 xy) {
            return float4(.5, .7, .9, .95);
        }
    )");

#define M(id) f##id = makeEffect(k##id##Code, "RE_" #id);
    SK_ALL_ANDROID_KNOWN_EFFECTS(M)
#undef M
}

namespace {

SkString to_str(ui::Dataspace ds) {
    switch (ds) {
        case ui::Dataspace::UNKNOWN:       return SkString("UNKNOWN");
        case ui::Dataspace::SRGB:          return SkString("SRGB");
        case ui::Dataspace::BT2020:        return SkString("BT2020");
        case ui::Dataspace::BT2020_ITU_PQ: return SkString("BT2020_ITU_PQ");
        case ui::Dataspace::BT2020_HLG:    return SkString("BT2020_HLG");
        case ui::Dataspace::DISPLAY_P3:    return SkString("DISPLAY_P3");
        case ui::Dataspace::V0_SRGB:       return SkString("V0_SRGB");
    }

    return SkStringPrintf("0x%x", static_cast<uint32_t>(ds));
}

const char* to_str(shaders::LinearEffect::SkSLType type) {
    switch (type) {
        case shaders::LinearEffect::SkSLType::Shader:      return "Shader";
        case shaders::LinearEffect::SkSLType::ColorFilter: return "ColorFilter";
    }
    SkUNREACHABLE;
}

} // anonymous namespace

sk_sp<SkRuntimeEffect> RuntimeEffectManager::getOrCreateLinearRuntimeEffect(
        const shaders::LinearEffect& linearEffect) {

    SkString name = SkStringPrintf("RE_LinearEffect_%s__%s__%s__%s__%s",
                                   to_str(linearEffect.inputDataspace).c_str(),
                                   to_str(linearEffect.outputDataspace).c_str(),
                                   linearEffect.undoPremultipliedAlpha ? "true" : "false",
                                   to_str(linearEffect.fakeOutputDataspace).c_str(),
                                   to_str(linearEffect.type));

    auto result = fLinearEffects.find(name.c_str());
    if (result != fLinearEffects.end()) {
        return result->second;
    }

    // Each code snippet must be unique, otherwise Skia will internally find a match
    // and uniquify things. To avoid this we just add an arbitrary alpha constant
    // to the code.
    static float arbitraryAlpha = 0.051f;
    SkString linearEffectCode = SkStringPrintf(
            "uniform shader child;"
            "vec4 main(vec2 xy) {"
                "float3 linear = child.eval(xy).rgb;"
                "return float4(linear, %f);"
            "}",
            arbitraryAlpha);
    arbitraryAlpha += 0.05f;

    sk_sp<SkRuntimeEffect> effect = makeEffect(linearEffectCode, name.c_str());
    fLinearEffects.insert({ name.c_str(), effect });
    return effect;
}
