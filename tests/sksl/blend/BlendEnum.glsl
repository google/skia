#version 400
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
            float _0_n = d.x * s.y;
            delta = min(d.y, _0_n / delta);

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
        float _1_n = (d.y - d.x) * s.y;
        float delta = max(0.0, d.y - _1_n / s.x);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component(vec2 s, vec2 d) {
    if (2.0 * s.x <= s.y) {
        float _2_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        return (_2_n / d.y + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);

    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _3_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        return _3_n / DaSqd;

    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);

    vec3 result = (lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor)) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        float _4_d = lum - minComp;
        result = lum + (result - lum) * (lum / _4_d);

    }
    if (maxComp > alpha && maxComp != lum) {
        vec3 _5_n = (result - lum) * (alpha - lum);
        float _6_d = maxComp - lum;
        return lum + _5_n / _6_d;

    } else {
        return result;
    }
}
vec3 _blend_set_color_saturation_helper(vec3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        float _7_n = sat * (minMidMax.y - minMidMax.x);
        float _8_d = minMidMax.z - minMidMax.x;
        return vec3(0.0, _7_n / _8_d, sat);

    } else {
        return vec3(0.0);
    }
}
vec3 _blend_set_color_saturation(vec3 hueLumColor, vec3 satColor) {
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
                {
                    _0_blend = vec4(0.0);
                    continue;
                }

            case 1:
                {
                    _0_blend = src;
                    continue;
                }

            case 2:
                {
                    _0_blend = dst;
                    continue;
                }

            case 3:
                {
                    _0_blend = src + (1.0 - src.w) * dst;
                    continue;
                }

            case 4:
                {
                    _0_blend = (1.0 - dst.w) * src + dst;
                    continue;
                }

            case 5:
                {
                    _0_blend = src * dst.w;
                    continue;
                }

            case 6:
                {
                    _0_blend = dst * src.w;
                    continue;
                }

            case 7:
                {
                    _0_blend = (1.0 - dst.w) * src;
                    continue;
                }

            case 8:
                {
                    _0_blend = (1.0 - src.w) * dst;
                    continue;
                }

            case 9:
                {
                    _0_blend = dst.w * src + (1.0 - src.w) * dst;
                    continue;
                }

            case 10:
                {
                    _0_blend = (1.0 - dst.w) * src + src.w * dst;
                    continue;
                }

            case 11:
                {
                    _0_blend = (1.0 - dst.w) * src + (1.0 - src.w) * dst;
                    continue;
                }

            case 12:
                {
                    _0_blend = min(src + dst, 1.0);
                    continue;
                }

            case 13:
                {
                    _0_blend = src * dst;
                    continue;
                }

            case 14:
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
                vec4 _2_result = src + (1.0 - src.w) * dst;

                _2_result.xyz = min(_2_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
                {
                    _0_blend = _2_result;
                    continue;
                }

            case 17:
                vec4 _3_result = src + (1.0 - src.w) * dst;

                _3_result.xyz = max(_3_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
                {
                    _0_blend = _3_result;
                    continue;
                }

            case 18:
                {
                    _0_blend = vec4(_color_dodge_component(src.xw, dst.xw), _color_dodge_component(src.yw, dst.yw), _color_dodge_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 19:
                {
                    _0_blend = vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 20:
                {
                    _0_blend = blend_overlay(dst, src);
                    continue;
                }

            case 21:
                {
                    _0_blend = dst.w == 0.0 ? src : vec4(_soft_light_component(src.xw, dst.xw), _soft_light_component(src.yw, dst.yw), _soft_light_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 22:
                {
                    _0_blend = vec4((src.xyz + dst.xyz) - 2.0 * min(src.xyz * dst.w, dst.xyz * src.w), src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 23:
                {
                    _0_blend = vec4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 24:
                {
                    _0_blend = vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
                    continue;
                }

            case 25:
                float _4_alpha = dst.w * src.w;
                vec3 _5_sda = src.xyz * dst.w;
                vec3 _6_dsa = dst.xyz * src.w;
                {
                    _0_blend = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_5_sda, _6_dsa), _4_alpha, _6_dsa) + dst.xyz) - _6_dsa) + src.xyz) - _5_sda, (src.w + dst.w) - _4_alpha);
                    continue;
                }

            case 26:
                float _7_alpha = dst.w * src.w;
                vec3 _8_sda = src.xyz * dst.w;
                vec3 _9_dsa = dst.xyz * src.w;
                {
                    _0_blend = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_9_dsa, _8_sda), _7_alpha, _9_dsa) + dst.xyz) - _9_dsa) + src.xyz) - _8_sda, (src.w + dst.w) - _7_alpha);
                    continue;
                }

            case 27:
                float _10_alpha = dst.w * src.w;
                vec3 _11_sda = src.xyz * dst.w;
                vec3 _12_dsa = dst.xyz * src.w;
                {
                    _0_blend = vec4((((_blend_set_color_luminance(_11_sda, _10_alpha, _12_dsa) + dst.xyz) - _12_dsa) + src.xyz) - _11_sda, (src.w + dst.w) - _10_alpha);
                    continue;
                }

            case 28:
                float _13_alpha = dst.w * src.w;
                vec3 _14_sda = src.xyz * dst.w;
                vec3 _15_dsa = dst.xyz * src.w;
                {
                    _0_blend = vec4((((_blend_set_color_luminance(_15_dsa, _13_alpha, _14_sda) + dst.xyz) - _15_dsa) + src.xyz) - _14_sda, (src.w + dst.w) - _13_alpha);
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
