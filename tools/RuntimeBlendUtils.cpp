/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBlendMode.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/effects/SkRuntimeEffect.h"

static sk_sp<SkBlender> blender_func(SkString code) {
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForBlender(std::move(code));
    SkASSERT(result.effect);
    return result.effect->makeBlender(/*uniforms=*/nullptr);
}

static sk_sp<SkBlender> blender_expr(const char* returnExpr) {
    SkString code = SkStringPrintf("half4 main(half4 src, half4 dst) {\n"
                                   "    return %s;\n"
                                   "}",
                                   returnExpr);
    return blender_func(std::move(code));
}

sk_sp<SkBlender> GetRuntimeBlendForBlendMode(SkBlendMode mode) {
    switch (mode) {
        case SkBlendMode::kClear: {
            static auto blender = blender_expr("half4(0)");
            return blender;
        }
        case SkBlendMode::kSrc: {
            static auto blender = blender_expr("src");
            return blender;
        }
        case SkBlendMode::kDst: {
            static auto blender = blender_expr("dst");
            return blender;
        }
        case SkBlendMode::kSrcOver: {
            static auto blender = blender_expr("src + (1 - src.a)*dst");
            return blender;
        }
        case SkBlendMode::kDstOver: {
            static auto blender = blender_expr("(1 - dst.a)*src + dst");
            return blender;
        }
        case SkBlendMode::kSrcIn: {
            static auto blender = blender_expr("(src == half4(0) ? half4(0) : src*dst.a)");
            return blender;
        }
        case SkBlendMode::kDstIn: {
            static auto blender = blender_expr("(dst == half4(0) ? half4(0) : dst*src.a)");
            return blender;
        }
        case SkBlendMode::kSrcOut: {
            static auto blender = blender_expr("(1 - dst.a)*src");
            return blender;
        }
        case SkBlendMode::kDstOut: {
            static auto blender = blender_expr("(1 - src.a)*dst");
            return blender;
        }
        case SkBlendMode::kSrcATop: {
            static auto blender = blender_expr("dst.a*src + (1 - src.a)*dst");
            return blender;
        }
        case SkBlendMode::kDstATop: {
            static auto blender = blender_expr("(1 - dst.a) * src + src.a*dst");
            return blender;
        }
        case SkBlendMode::kXor: {
            static auto blender = blender_expr("(1 - dst.a)*src + (1 - src.a)*dst");
            return blender;
        }
        case SkBlendMode::kPlus: {
            static auto blender = blender_expr("min(src + dst, 1)");
            return blender;
        }
        case SkBlendMode::kModulate: {
            static auto blender = blender_expr("src*dst");
            return blender;
        }
        case SkBlendMode::kScreen: {
            static auto blender = blender_expr("src + (1 - src)*dst");
            return blender;
        }
        case SkBlendMode::kOverlay: {
            static auto blender = blender_func(SkString(R"(
                half _blend_overlay_component(half2 s, half2 d) {
                    return (2*d.x <= d.y)
                            ? 2*s.x*d.x
                            : s.y*d.y - 2*(d.y - d.x)*(s.y - s.x);
                }

                half4 main(half4 src, half4 dst) {
                    half4 result = half4(_blend_overlay_component(src.ra, dst.ra),
                                         _blend_overlay_component(src.ga, dst.ga),
                                         _blend_overlay_component(src.ba, dst.ba),
                                         src.a + (1 - src.a)*dst.a);
                    result.rgb += dst.rgb*(1 - src.a) + src.rgb*(1 - dst.a);
                    return result;
                }
            )"));
            return blender;
        }
        case SkBlendMode::kDarken: {
            static auto blender = blender_func(SkString(R"(
                half4 main(half4 src, half4 dst) {
                    half4 result = src + (1 - src.a)*dst;
                    result.rgb = min(result.rgb, (1 - dst.a)*src.rgb + dst.rgb);
                    return result;
                }
            )"));
            return blender;
        }
        case SkBlendMode::kLighten: {
            static auto blender = blender_func(SkString(R"(
                half4 main(half4 src, half4 dst) {
                    half4 result = src + (1 - src.a)*dst;
                    result.rgb = max(result.rgb, (1 - dst.a)*src.rgb + dst.rgb);
                    return result;
                }
            )"));
            return blender;
        }
        default:
            SkDEBUGFAILF("unrecognized blend mode %d", (int)mode);
            return nullptr;
    }
}

