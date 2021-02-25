
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
vec4 blend_overlay(vec4 src, vec4 dst) {
    vec4 result = vec4(_blend_overlay_component(src.xw, dst.xw), _blend_overlay_component(src.yw, dst.yw), _blend_overlay_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
float _color_dodge_component(vec2 s, vec2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            float _3_guarded_divide;
            float _4_n = d.x * s.y;
            delta = min(d.y, _4_n / delta);

            return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        }
    }
}
float _color_burn_component(vec2 s, vec2 d) {
    if (d.y == d.x) {
        return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    } else if (s.x == 0.0) {
        return d.x * (1.0 - s.y);
    } else {
        float _5_guarded_divide;
        float _6_n = (d.y - d.x) * s.y;
        float delta = max(0.0, d.y - _6_n / s.x);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component(vec2 s, vec2 d) {
    if (2.0 * s.x <= s.y) {
        float _7_guarded_divide;
        float _8_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        return (_8_n / d.y + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);

    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _9_guarded_divide;
        float _10_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        return _10_n / DaSqd;

    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float _11_blend_color_luminance;
    float lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);

    float _12_blend_color_luminance;
    vec3 result = (lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor)) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        float _13_guarded_divide;
        float _14_d = lum - minComp;
        result = lum + (result - lum) * (lum / _14_d);

    }
    if (maxComp > alpha && maxComp != lum) {
        vec3 _15_guarded_divide;
        vec3 _16_n = (result - lum) * (alpha - lum);
        float _17_d = maxComp - lum;
        return lum + _16_n / _17_d;

    } else {
        return result;
    }
}
vec3 _blend_set_color_saturation_helper(vec3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        float _18_guarded_divide;
        float _19_n = sat * (minMidMax.y - minMidMax.x);
        float _20_d = minMidMax.z - minMidMax.x;
        return vec3(0.0, _19_n / _20_d, sat);

    } else {
        return vec3(0.0);
    }
}
vec3 _blend_set_color_saturation(vec3 hueLumColor, vec3 satColor) {
    float _21_blend_color_saturation;
    float sat = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);

    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            return _blend_set_color_saturation_helper(hueLumColor, sat);
        } else if (hueLumColor.x <= hueLumColor.z) {
            return _blend_set_color_saturation_helper(hueLumColor.xzy, sat).xzy;
        } else {
            return _blend_set_color_saturation_helper(hueLumColor.zxy, sat).yzx;
        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        return _blend_set_color_saturation_helper(hueLumColor.yxz, sat).yxz;
    } else if (hueLumColor.y <= hueLumColor.z) {
        return _blend_set_color_saturation_helper(hueLumColor.yzx, sat).zxy;
    } else {
        return _blend_set_color_saturation_helper(hueLumColor.zyx, sat).zyx;
    }
}
void main() {
    vec4 _0_blend;
    for (int _1_loop = 0;_1_loop < 1; _1_loop++) {
        switch (13) {
            case 0:
                vec4 _2_blend_clear;
                {
                    _0_blend = vec4(0.0);
                    continue;
                }

            case 1:
                vec4 _3_blend_src;
                {
                    _0_blend = src;
                    continue;
                }

            case 2:
                vec4 _4_blend_dst;
                {
                    _0_blend = dst;
                    continue;
                }

            case 3:
                vec4 _5_blend_src_over;
                {
                    _0_blend = src + (1.0 - src.w) * dst;
                    continue;
                }

            case 4:
                vec4 _6_blend_dst_over;
                {
                    _0_blend = (1.0 - dst.w) * src + dst;
                    continue;
                }

            case 5:
                vec4 _7_blend_src_in;
                {
                    _0_blend = src * dst.w;
                    continue;
                }

            case 6:
                vec4 _8_blend_dst_in;
                vec4 _9_blend_src_in;

                {
                    _0_blend = dst * src.w;
                    continue;
                }

            case 7:
                vec4 _10_blend_src_out;
                {
                    _0_blend = (1.0 - dst.w) * src;
                    continue;
                }

            case 8:
                vec4 _11_blend_dst_out;
                {
                    _0_blend = (1.0 - src.w) * dst;
                    continue;
                }

            case 9:
                vec4 _12_blend_src_atop;
                {
                    _0_blend = dst.w * src + (1.0 - src.w) * dst;
                    continue;
                }

            case 10:
                vec4 _13_blend_dst_atop;
                {
                    _0_blend = (1.0 - dst.w) * src + src.w * dst;
                    continue;
                }

            case 11:
                vec4 _14_blend_xor;
                {
                    _0_blend = (1.0 - dst.w) * src + (1.0 - src.w) * dst;
                    continue;
                }

            case 12:
                vec4 _15_blend_plus;
                {
                    _0_blend = min(src + dst, 1.0);
                    continue;
                }

            case 13:
                vec4 _16_blend_modulate;
                {
                    _0_blend = src * dst;
                    continue;
                }

            case 14:
                vec4 _17_blend_screen;
                {
                    _0_blend = src + (1.0 - src) * dst;
                    continue;
                }

            case 15:
                {
                    _0_blend = blend_overlay(src, dst);
                    continue;
                }
            case 16:
                vec4 _18_blend_darken;
                vec4 _19_blend_src_over;
                vec4 _20_result = src + (1.0 - src.w) * dst;

                _20_result.xyz = min(_20_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
                {
                    _0_blend = _20_result;
                    continue;
                }

            case 17:
                vec4 _21_blend_lighten;
                vec4 _22_blend_src_over;
                vec4 _23_result = src + (1.0 - src.w) * dst;

                _23_result.xyz = max(_23_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
                {
                    _0_blend = _23_result;
                    continue;
                }

            case 18:
                vec4 _24_blend_color_dodge;
                {
                    _0_blend = vec4(_color_dodge_component(src.xw, dst.xw), _color_dodge_component(src.yw, dst.yw), _color_dodge_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 19:
                vec4 _25_blend_color_burn;
                {
                    _0_blend = vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 20:
                vec4 _26_blend_hard_light;
                {
                    _0_blend = blend_overlay(dst, src);
                    continue;
                }

            case 21:
                vec4 _27_blend_soft_light;
                {
                    _0_blend = dst.w == 0.0 ? src : vec4(_soft_light_component(src.xw, dst.xw), _soft_light_component(src.yw, dst.yw), _soft_light_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 22:
                vec4 _28_blend_difference;
                {
                    _0_blend = vec4((src.xyz + dst.xyz) - 2.0 * min(src.xyz * dst.w, dst.xyz * src.w), src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 23:
                vec4 _29_blend_exclusion;
                {
                    _0_blend = vec4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 24:
                vec4 _30_blend_multiply;
                {
                    _0_blend = vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 25:
                vec4 _31_blend_hue;
                float _32_alpha = dst.w * src.w;
                vec3 _33_sda = src.xyz * dst.w;
                vec3 _34_dsa = dst.xyz * src.w;
                {
                    _0_blend = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_33_sda, _34_dsa), _32_alpha, _34_dsa) + dst.xyz) - _34_dsa) + src.xyz) - _33_sda, (src.w + dst.w) - _32_alpha);
                    continue;
                }

            case 26:
                vec4 _35_blend_saturation;
                float _36_alpha = dst.w * src.w;
                vec3 _37_sda = src.xyz * dst.w;
                vec3 _38_dsa = dst.xyz * src.w;
                {
                    _0_blend = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_38_dsa, _37_sda), _36_alpha, _38_dsa) + dst.xyz) - _38_dsa) + src.xyz) - _37_sda, (src.w + dst.w) - _36_alpha);
                    continue;
                }

            case 27:
                vec4 _39_blend_color;
                float _40_alpha = dst.w * src.w;
                vec3 _41_sda = src.xyz * dst.w;
                vec3 _42_dsa = dst.xyz * src.w;
                {
                    _0_blend = vec4((((_blend_set_color_luminance(_41_sda, _40_alpha, _42_dsa) + dst.xyz) - _42_dsa) + src.xyz) - _41_sda, (src.w + dst.w) - _40_alpha);
                    continue;
                }

            case 28:
                vec4 _43_blend_luminosity;
                float _44_alpha = dst.w * src.w;
                vec3 _45_sda = src.xyz * dst.w;
                vec3 _46_dsa = dst.xyz * src.w;
                {
                    _0_blend = vec4((((_blend_set_color_luminance(_46_dsa, _44_alpha, _45_sda) + dst.xyz) - _46_dsa) + src.xyz) - _45_sda, (src.w + dst.w) - _44_alpha);
                    continue;
                }

            default:
                {
                    _0_blend = vec4(0.0);
                    continue;
                }
        }
    }
    sk_FragColor = _0_blend;

}
