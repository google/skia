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

static sk_sp<SkBlender> blender_func(std::initializer_list<const char*> code) {
    SkString concat;
    for (const char* part : code) {
        concat.append(part);
    }
    SkRuntimeEffect::Result result = SkRuntimeEffect::MakeForBlender(std::move(concat));
    SkASSERTF(result.effect, "%s", result.errorText.c_str());
    return result.effect->makeBlender(/*uniforms=*/nullptr);
}

static sk_sp<SkBlender> blender_expr(const char* returnExpr) {
    return blender_func({"half4 main(half4 src, half4 dst) {\n"
                         "    return ", returnExpr, ";\n"
                         "}"});
}

sk_sp<SkBlender> GetRuntimeBlendForBlendMode(SkBlendMode mode) {
    static constexpr char kGuardedDivide[] = R"(
        half _guarded_divide(half n, half d) {
            return n/(d + 0.00000001);
        }

        half3 _guarded_divide(half3 n, half d) {
            return n/(d + 0.00000001);
        }
    )";

    static constexpr char kHSLCSharedCode[] = R"(
        half _blend_color_luminance(half3 color) { return dot(half3(0.3, 0.59, 0.11), color); }

        half3 _blend_set_color_luminance(half3 hueSatColor, half alpha, half3 lumColor) {
            half lum = _blend_color_luminance(lumColor);
            half3 result = lum - _blend_color_luminance(hueSatColor) + hueSatColor;
            half minComp = min(min(result.r, result.g), result.b);
            half maxComp = max(max(result.r, result.g), result.b);
            if (minComp < 0 && lum != minComp) {
                result = lum + (result - lum) * _guarded_divide(lum, (lum - minComp));
            }
            if (maxComp > alpha && maxComp != lum) {
                return lum + _guarded_divide((result - lum) * (alpha - lum), (maxComp - lum));
            } else {
                return result;
            }
        }

        half _blend_color_saturation(half3 color) {
            return max(max(color.r, color.g), color.b) - min(min(color.r, color.g), color.b);
        }

        half3 _blend_set_color_saturation_helper(half3 minMidMax, half sat) {
            if (minMidMax.r < minMidMax.b) {
                return half3(0,
                            _guarded_divide(sat*(minMidMax.g - minMidMax.r),
                                            (minMidMax.b - minMidMax.r)),
                            sat);
            } else {
                return half3(0);
            }
        }

        half3 _blend_set_color_saturation(half3 hueLumColor, half3 satColor) {
            half sat = _blend_color_saturation(satColor);
            if (hueLumColor.r <= hueLumColor.g) {
                if (hueLumColor.g <= hueLumColor.b) {
                    return _blend_set_color_saturation_helper(hueLumColor.rgb, sat);
                } else if (hueLumColor.r <= hueLumColor.b) {
                    return _blend_set_color_saturation_helper(hueLumColor.rbg, sat).rbg;
                } else {
                    return _blend_set_color_saturation_helper(hueLumColor.brg, sat).gbr;
                }
            } else if (hueLumColor.r <= hueLumColor.b) {
               return _blend_set_color_saturation_helper(hueLumColor.grb, sat).grb;
            } else if (hueLumColor.g <= hueLumColor.b) {
               return _blend_set_color_saturation_helper(hueLumColor.gbr, sat).brg;
            } else {
               return _blend_set_color_saturation_helper(hueLumColor.bgr, sat).bgr;
            }
        }
    )";

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
            static auto blender = blender_func({R"(
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
            )"});
            return blender;
        }
        case SkBlendMode::kDarken: {
            static auto blender = blender_func({R"(
                half4 main(half4 src, half4 dst) {
                    half4 result = src + (1 - src.a)*dst;
                    result.rgb = min(result.rgb, (1 - dst.a)*src.rgb + dst.rgb);
                    return result;
                }
            )"});
            return blender;
        }
        case SkBlendMode::kLighten: {
            static auto blender = blender_func({R"(
                half4 main(half4 src, half4 dst) {
                    half4 result = src + (1 - src.a)*dst;
                    result.rgb = max(result.rgb, (1 - dst.a)*src.rgb + dst.rgb);
                    return result;
                }
            )"});
            return blender;
        }
        case SkBlendMode::kColorDodge: {
            static auto blender = blender_func({kGuardedDivide, R"(
                half _color_dodge_component(half2 s, half2 d) {
                    if (d.x == 0) {
                        return s.x*(1 - d.y);
                    } else {
                        half delta = s.y - s.x;
                        if (delta == 0) {
                             return s.y*d.y + s.x*(1 - d.y) + d.x*(1 - s.y);
                        } else {
                            delta = min(d.y, _guarded_divide(d.x*s.y, delta));
                            return delta*s.y + s.x*(1 - d.y) + d.x*(1 - s.y);
                        }
                    }
                }

                half4 main(half4 src, half4 dst) {
                    return half4(_color_dodge_component(src.ra, dst.ra),
                                 _color_dodge_component(src.ga, dst.ga),
                                 _color_dodge_component(src.ba, dst.ba),
                                 src.a + (1 - src.a)*dst.a);
                }
            )"});
            return blender;
        }
        case SkBlendMode::kColorBurn: {
            static auto blender = blender_func({kGuardedDivide, R"(
                half _color_burn_component(half2 s, half2 d) {
                    if (d.y == d.x) {
                        return s.y*d.y + s.x*(1 - d.y) + d.x*(1 - s.y);
                    } else if (s.x == 0) {
                        return d.x*(1 - s.y);
                    } else {
                        half delta = max(0, d.y - _guarded_divide((d.y - d.x)*s.y, s.x));
                        return delta*s.y + s.x*(1 - d.y) + d.x*(1 - s.y);
                    }
                }

                half4 main(half4 src, half4 dst) {
                    return half4(_color_burn_component(src.ra, dst.ra),
                                 _color_burn_component(src.ga, dst.ga),
                                 _color_burn_component(src.ba, dst.ba),
                                 src.a + (1 - src.a)*dst.a);
                }
            )"});
            return blender;
        }
        case SkBlendMode::kHardLight: {
            static auto blender = blender_func({R"(
                half _blend_overlay_component(half2 s, half2 d) {
                    return (2*d.x <= d.y)
                            ? 2*s.x*d.x
                            : s.y*d.y - 2*(d.y - d.x)*(s.y - s.x);
                }

                half4 main(half4 dst, half4 src) {
                    half4 result = half4(_blend_overlay_component(src.ra, dst.ra),
                                         _blend_overlay_component(src.ga, dst.ga),
                                         _blend_overlay_component(src.ba, dst.ba),
                                         src.a + (1 - src.a)*dst.a);
                    result.rgb += dst.rgb*(1 - src.a) + src.rgb*(1 - dst.a);
                    return result;
                }
            )"});
            return blender;
        }
        case SkBlendMode::kSoftLight: {
            static auto blender = blender_func({kGuardedDivide, R"(
                half _soft_light_component(half2 s, half2 d) {
                    if (2*s.x <= s.y) {
                        return _guarded_divide(d.x*d.x*(s.y - 2*s.x), d.y) +
                               (1 - d.y)*s.x + d.x*(-s.y + 2*s.x + 1);
                    } else if (4.0 * d.x <= d.y) {
                        half DSqd = d.x*d.x;
                        half DCub = DSqd*d.x;
                        half DaSqd = d.y*d.y;
                        half DaCub = DaSqd*d.y;
                        return _guarded_divide(DaSqd*(s.x - d.x*(3*s.y - 6*s.x - 1))
                                               + 12*d.y*DSqd*(s.y - 2*s.x)
                                               - 16*DCub * (s.y - 2*s.x) - DaCub*s.x, DaSqd);
                    } else {
                        return d.x*(s.y - 2*s.x + 1) + s.x - sqrt(d.y*d.x)*(s.y - 2*s.x) - d.y*s.x;
                    }
                }

                half4 main(half4 src, half4 dst) {
                    return (dst.a == 0) ? src : half4(_soft_light_component(src.ra, dst.ra),
                                                      _soft_light_component(src.ga, dst.ga),
                                                      _soft_light_component(src.ba, dst.ba),
                                                      src.a + (1 - src.a)*dst.a);
                }
            )"});
            return blender;
        }
        case SkBlendMode::kDifference: {
            static auto blender = blender_func({R"(
                half4 main(half4 src, half4 dst) {
                    return half4(src.rgb + dst.rgb - 2*min(src.rgb*dst.a, dst.rgb*src.a),
                                 src.a + (1 - src.a)*dst.a);
                }
            )"});
            return blender;
        }
        case SkBlendMode::kExclusion: {
            static auto blender = blender_func({R"(
                half4 main(half4 src, half4 dst) {
                    return half4(dst.rgb + src.rgb - 2*dst.rgb*src.rgb, src.a + (1 - src.a)*dst.a);
                }
            )"});
            return blender;
        }
        case SkBlendMode::kMultiply: {
            static auto blender = blender_func({R"(
                half4 main(half4 src, half4 dst) {
                    return half4((1 - src.a)*dst.rgb + (1 - dst.a)*src.rgb + src.rgb*dst.rgb,
                                 src.a + (1 - src.a)*dst.a);
                }
            )"});
            return blender;
        }
        case SkBlendMode::kHue: {
            static auto blender = blender_func({kGuardedDivide, kHSLCSharedCode, R"(
                half4 main(half4 src, half4 dst) {
                    half alpha = dst.a*src.a;
                    half3 sda = src.rgb*dst.a;
                    half3 dsa = dst.rgb*src.a;
                    return half4(_blend_set_color_luminance(_blend_set_color_saturation(sda, dsa),
                                                            alpha, dsa) +
                                 dst.rgb - dsa + src.rgb - sda,
                                 src.a + dst.a - alpha);
                                }
            )"});
            return blender;
        }
        case SkBlendMode::kSaturation: {
            static auto blender = blender_func({kGuardedDivide, kHSLCSharedCode, R"(
                half4 main(half4 src, half4 dst) {
                    half alpha = dst.a*src.a;
                    half3 sda = src.rgb*dst.a;
                    half3 dsa = dst.rgb*src.a;
                    return half4(_blend_set_color_luminance(_blend_set_color_saturation(dsa, sda),
                                                            alpha, dsa) +
                                 dst.rgb - dsa + src.rgb - sda,
                                 src.a + dst.a - alpha);
                }
            )"});
            return blender;
        }
        case SkBlendMode::kColor: {
            static auto blender = blender_func({kGuardedDivide, kHSLCSharedCode, R"(
                half4 main(half4 src, half4 dst)  {
                    half alpha = dst.a*src.a;
                    half3 sda = src.rgb*dst.a;
                    half3 dsa = dst.rgb*src.a;
                    return half4(_blend_set_color_luminance(sda, alpha, dsa) + dst.rgb
                                 - dsa + src.rgb - sda, src.a + dst.a - alpha);
                }
            )"});
            return blender;
        }
        case SkBlendMode::kLuminosity: {
            static auto blender = blender_func({kGuardedDivide, kHSLCSharedCode, R"(
                half4 main(half4 src, half4 dst) {
                    half alpha = dst.a*src.a;
                    half3 sda = src.rgb*dst.a;
                    half3 dsa = dst.rgb*src.a;
                    return half4(_blend_set_color_luminance(dsa, alpha, sda) + dst.rgb - dsa
                                 + src.rgb - sda, src.a + dst.a - alpha);
                }
            )"});
            return blender;
        }
        default:
            SkDEBUGFAILF("unrecognized blend mode %d", (int)mode);
            return nullptr;
    }
}

