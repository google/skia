/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeEffectPriv_DEFINED
#define SkRuntimeEffectPriv_DEFINED

#include "include/effects/SkRuntimeEffect.h"
#include "include/private/SkColorData.h"
#include "src/core/SkVM.h"

#include <functional>

#ifdef SK_ENABLE_SKSL

class SkRuntimeEffectPriv {
public:
    // Helper function when creating an effect for a GrSkSLFP that verifies an effect will
    // implement the constant output for constant input optimization flag.
    static bool SupportsConstantOutputForConstantInput(sk_sp<SkRuntimeEffect> effect) {
        return effect->getFilterColorProgram();
    }

    static SkRuntimeEffect::Options ES3Options() {
        SkRuntimeEffect::Options options;
        options.enforceES2Restrictions = false;
        return options;
    }

    static void EnableFragCoord(SkRuntimeEffect::Options* options) {
        options->allowFragCoord = true;
    }
};

// These internal APIs for creating runtime effects vary from the public API in two ways:
//
//     1) they're used in contexts where it's not useful to receive an error message;
//     2) they're cached.
//
// Users of the public SkRuntimeEffect::Make*() can of course cache however they like themselves;
// keeping these APIs private means users will not be forced into our cache or cache policy.

sk_sp<SkRuntimeEffect> SkMakeCachedRuntimeEffect(SkRuntimeEffect::Result (*make)(SkString sksl),
                                                 SkString sksl);

inline sk_sp<SkRuntimeEffect> SkMakeCachedRuntimeEffect(SkRuntimeEffect::Result (*make)(SkString),
                                                        const char* sksl) {
    return SkMakeCachedRuntimeEffect(make, SkString{sksl});
}

// Internal API that assumes (and asserts) that the shader code is valid, but does no internal
// caching. Used when the caller will cache the result in a static variable.
inline sk_sp<SkRuntimeEffect> SkMakeRuntimeEffect(
        SkRuntimeEffect::Result (*make)(SkString, const SkRuntimeEffect::Options&),
        const char* sksl,
        SkRuntimeEffect::Options options = SkRuntimeEffect::Options{}) {
    SkRuntimeEffectPriv::EnableFragCoord(&options);
    auto result = make(SkString{sksl}, options);
    SkASSERTF(result.effect, "%s", result.errorText.c_str());
    return result.effect;
}

// This is mostly from skvm's rgb->hsl code, with some GPU-related finesse pulled from
// GrHighContrastFilterEffect.fp, see next comment.
inline constexpr char kRGB_to_HSL_sksl[] =
    "half3 rgb_to_hsl(half3 c) {"
        "half mx = max(max(c.r,c.g),c.b),"
        "     mn = min(min(c.r,c.g),c.b),"
        "      d = mx-mn,                "
        "   invd = 1.0 / d,              "
        " g_lt_b = c.g < c.b ? 6.0 : 0.0;"

        // We'd prefer to write these tests like `mx == c.r`, but on some GPUs max(x,y) is
        // not always equal to either x or y.  So we use long form, c.r >= c.g && c.r >= c.b.
        "half h = (1/6.0) * (mx == mn                 ? 0.0 :"
        "     /*mx==c.r*/    c.r >= c.g && c.r >= c.b ? invd * (c.g - c.b) + g_lt_b :"
        "     /*mx==c.g*/    c.g >= c.b               ? invd * (c.b - c.r) + 2.0  "
        "     /*mx==c.b*/                             : invd * (c.r - c.g) + 4.0);"

        "half sum = mx+mn,"
        "       l = sum * 0.5,"
        "       s = mx == mn ? 0.0"
        "                    : d / (l > 0.5 ? 2.0 - sum : sum);"
        "return half3(h,s,l);"
    "}";

//This is straight out of GrHSLToRGBFilterEffect.fp.
inline constexpr char kHSL_to_RGB_sksl[] =
    "half3 hsl_to_rgb(half3 hsl) {"
        "half  C = (1 - abs(2 * hsl.z - 1)) * hsl.y;"
        "half3 p = hsl.xxx + half3(0, 2/3.0, 1/3.0);"
        "half3 q = saturate(abs(fract(p) * 6 - 3) - 1);"
        "return (q - 0.5) * C + hsl.z;"
    "}";

/**
 * Runtime effects are often long lived & cached. Individual color filters or FPs created from them
 * and are often short-lived. However, color filters and FPs may need to operate on a single color
 * (on the CPU). This may be done at the paint level (eg, filter the paint color), or as part of
 * FP tree analysis.
 *
 * SkFilterColorProgram is an skvm program representing a (color filter) SkRuntimeEffect. It can
 * process a single color, without knowing the details of a particular instance (uniform values or
 * children).
 */
class SkFilterColorProgram {
public:
    static std::unique_ptr<SkFilterColorProgram> Make(const SkRuntimeEffect* effect);

    SkPMColor4f eval(const SkPMColor4f& inColor,
                     const void* uniformData,
                     std::function<SkPMColor4f(int, SkPMColor4f)> evalChild) const;

    bool isAlphaUnchanged() const { return fAlphaUnchanged; }

private:
    struct SampleCall {
        enum class Kind {
            kInputColor,  // eg sample(child) or sample(child, inputColor)
            kImmediate,   // eg sample(child, half4(1))
            kPrevious,    // eg sample(child1, sample(child2))
            kUniform,     // eg uniform half4 color; ... sample(child, color)
        };

        int  fChild;
        Kind fKind;
        union {
            SkPMColor4f fImm;       // for kImmediate
            int         fPrevious;  // for kPrevious
            int         fOffset;    // for kUniform
        };
    };

    SkFilterColorProgram(skvm::Program program,
                         std::vector<SampleCall> sampleCalls,
                         bool alphaUnchanged);

    skvm::Program           fProgram;
    std::vector<SampleCall> fSampleCalls;
    bool                    fAlphaUnchanged;
};

#endif  // SK_ENABLE_SKSL

#endif  // SkRuntimeEffectPriv_DEFINED
