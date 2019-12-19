R"SKSL(
// Definitions of functions implementing all of the SkBlendMode blends.

half4 blend_clear(half4 src, half4 dst) { return half4(0); }

half4 blend_src(half4 src, half4 dst) { return src; }

half4 blend_dst(half4 src, half4 dst) { return dst; }

half4 blend_src_over(half4 src, half4 dst) { return src + (1 - src.a)*dst; }

half4 blend_dst_over(half4 src, half4 dst) { return (1 - dst.a)*src + dst; }

half4 blend_src_in(half4 src, half4 dst) {
    @if (sk_Caps.inBlendModesFailRandomlyForAllZeroVec) {
        if (src == half4(0)) {
            return half4(0);
        }
    }
    return src*dst.a;
}

half4 blend_dst_in(half4 src, half4 dst) { return blend_src_in(dst, src); }

half4 blend_src_out(half4 src, half4 dst) { return (1 - dst.a)*src; }

half4 blend_dst_out(half4 src, half4 dst) { return (1 - src.a)*dst; }

half4 blend_src_atop(half4 src, half4 dst) { return dst.a*src + (1 - src.a)*dst; }

half4 blend_dst_atop(half4 src, half4 dst)  { return  (1 - dst.a) * src + src.a*dst; }

half4 blend_xor(half4 src, half4 dst) { return (1 - dst.a)*src + (1 - src.a)*dst; }

half4 blend_plus(half4 src, half4 dst) { return min(src + dst, 1); }

half4 blend_modulate(half4 src, half4 dst) { return src*dst; }

half4 blend_screen(half4 src, half4 dst) { return src + (1 - src)*dst; }

half _blend_overlay_component(half sc, half sa, half dc, half da) {
    if (2*dc <= da) {
        return 2*sc*dc;
    }
    return sa*da - 2*(da - dc)*(sa - sc);
}

half4 blend_overlay(half4 src, half4 dst) {
     half4 result = half4(_blend_overlay_component(src.r, src.a, dst.r, dst.a),
                          _blend_overlay_component(src.g, src.a, dst.g, dst.a),
                          _blend_overlay_component(src.b, src.a, dst.b, dst.a),
                           src.a + (1 - src.a)*dst.a);
    result.rgb += dst.rgb*(1 - src.a) + src.rgb*(1 - dst.a);
    return result;
}

half4 blend_darken(half4 src, half4 dst) {
   half4 result = blend_src_over(src, dst);
   result.rgb = min(result.rgb, (1 - dst.a)*src.rgb + dst.rgb);
   return result;
}

half4 blend_lighten(half4 src, half4 dst) {
    half4 result = blend_src_over(src, dst);
    result.rgb = max(result.rgb, (1 - dst.a)*src.rgb + dst.rgb);
    return result;
}

half _guarded_divide(half n, half d) {
    @if (sk_Caps.mustGuardDivisionEvenAfterExplicitZeroCheck) {
        return n/(d + 0.00000001);
    }
    return n/d;
}

half _color_dodge_component(half sc, half sa, half dc, half da) {
    if (dc == 0) {
        return sc*(1 - da);
    } else {
        half d = sa - sc;
        if (d == 0) {
             return sa*da + sc*(1 - da) + dc*(1 - sa);
        }
        d = min(da, _guarded_divide(dc*sa, d));
        return d*sa + sc*(1 - da) + dc*(1 - sa);
    }
}

half4 blend_color_dodge(half4 src, half4 dst) {
    return half4(_color_dodge_component(src.r, src.a, dst.r, dst.a),
                 _color_dodge_component(src.g, src.a, dst.g, dst.a),
                 _color_dodge_component(src.b, src.a, dst.b, dst.a),
                 src.a + (1 - src.a)*dst.a);
}

half _color_burn_component(half sc, half sa, half dc, half da) {
    if (da == dc) {
        return sa*da + sc*(1 - da) + dc*(1 - sa);
    } else if (sc == 0) {
        return dc*(1 - sa);
    }
    half d = max(0, da - _guarded_divide((da - dc)*sa, sc));
    return d*sa + sc*(1 - da) + dc*(1 - sa);
}

half4 blend_color_burn(half4 src, half4 dst) {
    return half4(_color_burn_component(src.r, src.a, dst.r, dst.a),
                 _color_burn_component(src.g, src.a, dst.g, dst.a),
                 _color_burn_component(src.b, src.a, dst.b, dst.a),
                 src.a + (1 - src.a)*dst.a);
}

half4 blend_hard_light(half4 src, half4 dst) { return blend_overlay(dst, src); }

half _soft_light_component(half sc, half sa, half dc, half da) {
    if (2*sc <= sa) {
        return _guarded_divide(dc*dc*(sa - 2*sc), da) + (1 - da)*sc + dc*(-sa + 2*sc + 1);
    } else if (4.0 * dc <= da) {
        half DSqd = dc*dc;
        half DCub = DSqd*dc;
        half DaSqd = da*da;
        half DaCub = DaSqd*da;
        return _guarded_divide(DaSqd*(sc - dc*(3*sa - 6*sc - 1)) + 12*da*DSqd*(sa - 2*sc)
                               - 16*DCub * (sa - 2*sc) - DaCub*sc, DaSqd);
    }
    return dc*(sa - 2*sc + 1) + sc - sqrt(da*dc)*(sa - 2*sc) - da*sc;
}

half4 blend_soft_light(half4 src, half4 dst) {
    if (dst.a == 0) {
        return src;
    }
    return half4(_soft_light_component(src.r, src.a, dst.r, dst.a),
                 _soft_light_component(src.g, src.a, dst.g, dst.a),
                 _soft_light_component(src.b, src.a, dst.b, dst.a),
                 src.a + (1 - src.a)*dst.a);
}

half4 blend_difference(half4 src, half4 dst) {
    return half4(src.rgb + dst.rgb - 2*min(src.rgb*dst.a, dst.rgb*src.a),
                 src.a + (1 - src.a)*dst.a);
}

half4 blend_exclusion(half4 src, half4 dst) {
    return half4(dst.rgb + src.rgb - 2*dst.rgb*src.rgb, src.a + (1 - src.a)*dst.a);
}

half4 blend_multiply(half4 src, half4 dst) {
    return half4((1 - src.a)*dst.rgb + (1 - dst.a)*src.rgb + src.rgb*dst.rgb,
                 src.a + (1 - src.a)*dst.a);
}

half _blend_color_luminance(half3 color) { return dot(half3(0.3, 0.59, 0.11), color); }

half3 _blend_set_color_luminance(half3 hueSatColor, half alpha, half3 lumColor) {
    half lum = _blend_color_luminance(lumColor);
    half3 result = lum - _blend_color_luminance(hueSatColor) + hueSatColor;
    half minComp = min(min(result.r, result.g), result.b);
    half maxComp = max(max(result.r, result.g), result.b);
    if (minComp < 0 && lum != minComp) {
        result = lum + (result - lum) * lum/(lum - minComp);
    }
    if (maxComp > alpha && maxComp != lum) {
        return lum + ((result - lum) * (alpha - lum))/(maxComp - lum);
    }
    return result;
}

half _blend_color_saturation(half3 color) {
    return max(max(color.r, color.g), color.b) - min(min(color.r, color.g), color.b);
}

half3 _blend_set_color_saturation_helper(half3 minMidMax, half sat) {
    if (minMidMax.r < minMidMax.b) {
        return half3(0, sat*(minMidMax.g - minMidMax.r)/(minMidMax.b - minMidMax.r), sat);
    }
    return half3(0);
}

half3 _blend_set_color_saturation(half3 hueLumColor, half3 satColor) {
    half sat = _blend_color_saturation(satColor);
    if (hueLumColor.r <= hueLumColor.g) {
        if (hueLumColor.g <= hueLumColor.b) {
            hueLumColor.rgb = _blend_set_color_saturation_helper(hueLumColor.rgb, sat);
        } else if (hueLumColor.r <= hueLumColor.b) {
            hueLumColor.rbg = _blend_set_color_saturation_helper(hueLumColor.rbg, sat);
        } else {
            hueLumColor.brg = _blend_set_color_saturation_helper(hueLumColor.brg, sat);
        }
    } else if (hueLumColor.r <= hueLumColor.b) {
       hueLumColor.grb = _blend_set_color_saturation_helper(hueLumColor.grb, sat);
    } else if (hueLumColor.g <= hueLumColor.b) {
       hueLumColor.gbr = _blend_set_color_saturation_helper(hueLumColor.gbr, sat);
    } else {
       hueLumColor.bgr = _blend_set_color_saturation_helper(hueLumColor.bgr, sat);
    }
    return hueLumColor;
}

half4 blend_hue(half4 src, half4 dst) {
    half alpha = dst.a*src.a;
    half3 sda = src.rgb*dst.a;
    half3 dsa = dst.rgb*src.a;
    return half4(_blend_set_color_luminance(_blend_set_color_saturation(sda, dsa), alpha, dsa) +
                 dst.rgb - dsa + src.rgb - sda,
                 src.a + dst.a - alpha);
}

half4 blend_saturation(half4 src, half4 dst) {
    half alpha = dst.a*src.a;
    half3 sda = src.rgb*dst.a;
    half3 dsa = dst.rgb*src.a;
    return half4(_blend_set_color_luminance(_blend_set_color_saturation(dsa, sda), alpha, dsa) +
                 dst.rgb - dsa + src.rgb - sda,
                 src.a + dst.a - alpha);
}

half4 blend_color(half4 src, half4 dst)  {
    half alpha = dst.a*src.a;
    half3 sda = src.rgb*dst.a;
    half3 dsa = dst.rgb*src.a;
    return half4(_blend_set_color_luminance(sda, alpha, dsa) + dst.rgb - dsa + src.rgb - sda,
                 src.a + dst.a - alpha);
}

half4 blend_luminosity(half4 src, half4 dst) {
    half alpha = dst.a*src.a;
    half3 sda = src.rgb*dst.a;
    half3 dsa = dst.rgb*src.a;
    return half4(_blend_set_color_luminance(dsa, alpha, sda) + dst.rgb - dsa + src.rgb - sda,
                 src.a + dst.a - alpha);

}

enum class SkBlendMode {
    kClear      = 0,
    kSrc        = 1,
    kDst        = 2,
    kSrcOver    = 3,
    kDstOver    = 4,
    kSrcIn      = 5,
    kDstIn      = 6,
    kSrcOut     = 7,
    kDstOut     = 8,
    kSrcATop    = 9,
    kDstATop    = 10,
    kXor        = 11,
    kPlus       = 12,
    kModulate   = 13,
    kScreen     = 14,
    kOverlay    = 15,
    kDarken     = 16,
    kLighten    = 17,
    kColorDodge = 18,
    kColorBurn  = 19,
    kHardLight  = 20,
    kSoftLight  = 21,
    kDifference = 22,
    kExclusion  = 23,
    kMultiply   = 24,
    kHue        = 25,
    kSaturation = 26,
    kColor      = 27,
    kLuminosity = 28
};

half4 blend(SkBlendMode mode, half4 src, half4 dst) {
    switch (mode) {
        case SkBlendMode::kClear:      return blend_clear(src, dst);
        case SkBlendMode::kSrc:        return blend_src(src, dst);
        case SkBlendMode::kDst:        return blend_dst(src, dst);
        case SkBlendMode::kSrcOver:    return blend_src_over(src, dst);
        case SkBlendMode::kDstOver:    return blend_dst_over(src, dst);
        case SkBlendMode::kSrcIn:      return blend_src_in(src, dst);
        case SkBlendMode::kDstIn:      return blend_dst_in(src, dst);
        case SkBlendMode::kSrcOut:     return blend_src_out(src, dst);
        case SkBlendMode::kDstOut:     return blend_dst_out(src, dst);
        case SkBlendMode::kSrcATop:    return blend_src_atop(src, dst);
        case SkBlendMode::kDstATop:    return blend_dst_atop(src, dst);
        case SkBlendMode::kXor:        return blend_xor(src, dst);
        case SkBlendMode::kPlus:       return blend_plus(src, dst);
        case SkBlendMode::kModulate:   return blend_modulate(src, dst);
        case SkBlendMode::kScreen:     return blend_screen(src, dst);
        case SkBlendMode::kOverlay:    return blend_overlay(src, dst);
        case SkBlendMode::kDarken:     return blend_darken(src, dst);
        case SkBlendMode::kLighten:    return blend_lighten(src, dst);
        case SkBlendMode::kColorDodge: return blend_color_dodge(src, dst);
        case SkBlendMode::kColorBurn:  return blend_color_burn(src, dst);
        case SkBlendMode::kHardLight:  return blend_hard_light(src, dst);
        case SkBlendMode::kSoftLight:  return blend_soft_light(src, dst);
        case SkBlendMode::kDifference: return blend_difference(src, dst);
        case SkBlendMode::kExclusion:  return blend_exclusion(src, dst);
        case SkBlendMode::kMultiply:   return blend_multiply(src, dst);
        case SkBlendMode::kHue:        return blend_hue(src, dst);
        case SkBlendMode::kSaturation: return blend_saturation(src, dst);
        case SkBlendMode::kColor:      return blend_color(src, dst);
        case SkBlendMode::kLuminosity: return blend_luminosity(src, dst);
    }
    return half4(0);  // Avoids "'blend' can exit without returning a value."
}
)SKSL"
