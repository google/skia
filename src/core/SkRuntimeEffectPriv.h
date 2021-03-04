/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRuntimeEffectPriv_DEFINED
#define SkRuntimeEffectPriv_DEFINED

#include "include/effects/SkRuntimeEffect.h"

// These internal APIs for creating runtime effects vary from the public API in two ways:
//
//     1) they're used in contexts where it's not useful to receive an error message;
//     2) they're cached.
//
// Users of the public SkRuntimeEffect::Make() can of course cache however they like themselves;
// keeping these APIs private means users will not be forced into our cache or cache policy.

sk_sp<SkRuntimeEffect> SkMakeCachedRuntimeEffect(SkString);

inline sk_sp<SkRuntimeEffect> SkMakeCachedRuntimeEffect(const char* sksl) {
    return SkMakeCachedRuntimeEffect(SkString{sksl});
}

// This is mostly from skvm's rgb->hsl code, with some GPU-related finesse pulled from
// GrHighContrastFilterEffect.fp, see next comment.
constexpr char kRGB_to_HSL_sksl[] =
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
constexpr char kHSL_to_RGB_sksl[] =
    "half3 hsl_to_rgb(half3 hsl) {"
        "half  C = (1 - abs(2 * hsl.z - 1)) * hsl.y;"
        "half3 p = hsl.xxx + half3(0, 2/3.0, 1/3.0);"
        "half3 q = saturate(abs(fract(p) * 6 - 3) - 1);"
        "return (q - 0.5) * C + hsl.z;"
    "}";

#endif
