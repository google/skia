#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 src;
    float4 dst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float _blend_overlay_component(float2 s, float2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
float4 blend_overlay(float4 src, float4 dst) {
    float4 result = float4(_blend_overlay_component(src.xw, dst.xw), _blend_overlay_component(src.yw, dst.yw), _blend_overlay_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    result.xyz = result.xyz + dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
float _color_dodge_component(float2 s, float2 d) {
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
float _color_burn_component(float2 s, float2 d) {
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
float _soft_light_component(float2 s, float2 d) {
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
float3 _blend_set_color_luminance(float3 hueSatColor, float alpha, float3 lumColor) {
    float _11_blend_color_luminance;
    float lum = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);

    float _12_blend_color_luminance;
    float3 result = (lum - dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor)) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        float _13_guarded_divide;
        float _14_d = lum - minComp;
        result = lum + (result - lum) * (lum / _14_d);

    }
    if (maxComp > alpha && maxComp != lum) {
        float3 _15_guarded_divide;
        float3 _16_n = (result - lum) * (alpha - lum);
        float _17_d = maxComp - lum;
        return lum + _16_n / _17_d;

    } else {
        return result;
    }
}
float3 _blend_set_color_saturation_helper(float3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        float _18_guarded_divide;
        float _19_n = sat * (minMidMax.y - minMidMax.x);
        float _20_d = minMidMax.z - minMidMax.x;
        return float3(0.0, _19_n / _20_d, sat);

    } else {
        return float3(0.0);
    }
}
float3 _blend_set_color_saturation(float3 hueLumColor, float3 satColor) {
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


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 _0_blend;
    for (int _1_loop = 0;_1_loop < 1; _1_loop++) {
        switch (13) {
            case 0:
                float4 _2_blend_clear;
                {
                    _0_blend = float4(0.0);
                    continue;
                }

            case 1:
                float4 _3_blend_src;
                {
                    _0_blend = _in.src;
                    continue;
                }

            case 2:
                float4 _4_blend_dst;
                {
                    _0_blend = _in.dst;
                    continue;
                }

            case 3:
                float4 _5_blend_src_over;
                {
                    _0_blend = _in.src + (1.0 - _in.src.w) * _in.dst;
                    continue;
                }

            case 4:
                float4 _6_blend_dst_over;
                {
                    _0_blend = (1.0 - _in.dst.w) * _in.src + _in.dst;
                    continue;
                }

            case 5:
                float4 _7_blend_src_in;
                {
                    _0_blend = _in.src * _in.dst.w;
                    continue;
                }

            case 6:
                float4 _8_blend_dst_in;
                float4 _9_blend_src_in;

                {
                    _0_blend = _in.dst * _in.src.w;
                    continue;
                }

            case 7:
                float4 _10_blend_src_out;
                {
                    _0_blend = (1.0 - _in.dst.w) * _in.src;
                    continue;
                }

            case 8:
                float4 _11_blend_dst_out;
                {
                    _0_blend = (1.0 - _in.src.w) * _in.dst;
                    continue;
                }

            case 9:
                float4 _12_blend_src_atop;
                {
                    _0_blend = _in.dst.w * _in.src + (1.0 - _in.src.w) * _in.dst;
                    continue;
                }

            case 10:
                float4 _13_blend_dst_atop;
                {
                    _0_blend = (1.0 - _in.dst.w) * _in.src + _in.src.w * _in.dst;
                    continue;
                }

            case 11:
                float4 _14_blend_xor;
                {
                    _0_blend = (1.0 - _in.dst.w) * _in.src + (1.0 - _in.src.w) * _in.dst;
                    continue;
                }

            case 12:
                float4 _15_blend_plus;
                {
                    _0_blend = min(_in.src + _in.dst, 1.0);
                    continue;
                }

            case 13:
                float4 _16_blend_modulate;
                {
                    _0_blend = _in.src * _in.dst;
                    continue;
                }

            case 14:
                float4 _17_blend_screen;
                {
                    _0_blend = _in.src + (1.0 - _in.src) * _in.dst;
                    continue;
                }

            case 15:
                {
                    _0_blend = blend_overlay(_in.src, _in.dst);
                    continue;
                }
            case 16:
                float4 _18_blend_darken;
                float4 _19_blend_src_over;
                float4 _20_result = _in.src + (1.0 - _in.src.w) * _in.dst;

                _20_result.xyz = min(_20_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
                {
                    _0_blend = _20_result;
                    continue;
                }

            case 17:
                float4 _21_blend_lighten;
                float4 _22_blend_src_over;
                float4 _23_result = _in.src + (1.0 - _in.src.w) * _in.dst;

                _23_result.xyz = max(_23_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
                {
                    _0_blend = _23_result;
                    continue;
                }

            case 18:
                float4 _24_blend_color_dodge;
                {
                    _0_blend = float4(_color_dodge_component(_in.src.xw, _in.dst.xw), _color_dodge_component(_in.src.yw, _in.dst.yw), _color_dodge_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 19:
                float4 _25_blend_color_burn;
                {
                    _0_blend = float4(_color_burn_component(_in.src.xw, _in.dst.xw), _color_burn_component(_in.src.yw, _in.dst.yw), _color_burn_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 20:
                float4 _26_blend_hard_light;
                {
                    _0_blend = blend_overlay(_in.dst, _in.src);
                    continue;
                }

            case 21:
                float4 _27_blend_soft_light;
                {
                    _0_blend = _in.dst.w == 0.0 ? _in.src : float4(_soft_light_component(_in.src.xw, _in.dst.xw), _soft_light_component(_in.src.yw, _in.dst.yw), _soft_light_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 22:
                float4 _28_blend_difference;
                {
                    _0_blend = float4((_in.src.xyz + _in.dst.xyz) - 2.0 * min(_in.src.xyz * _in.dst.w, _in.dst.xyz * _in.src.w), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 23:
                float4 _29_blend_exclusion;
                {
                    _0_blend = float4((_in.dst.xyz + _in.src.xyz) - (2.0 * _in.dst.xyz) * _in.src.xyz, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 24:
                float4 _30_blend_multiply;
                {
                    _0_blend = float4(((1.0 - _in.src.w) * _in.dst.xyz + (1.0 - _in.dst.w) * _in.src.xyz) + _in.src.xyz * _in.dst.xyz, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 25:
                float4 _31_blend_hue;
                float _32_alpha = _in.dst.w * _in.src.w;
                float3 _33_sda = _in.src.xyz * _in.dst.w;
                float3 _34_dsa = _in.dst.xyz * _in.src.w;
                {
                    _0_blend = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_33_sda, _34_dsa), _32_alpha, _34_dsa) + _in.dst.xyz) - _34_dsa) + _in.src.xyz) - _33_sda, (_in.src.w + _in.dst.w) - _32_alpha);
                    continue;
                }

            case 26:
                float4 _35_blend_saturation;
                float _36_alpha = _in.dst.w * _in.src.w;
                float3 _37_sda = _in.src.xyz * _in.dst.w;
                float3 _38_dsa = _in.dst.xyz * _in.src.w;
                {
                    _0_blend = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_38_dsa, _37_sda), _36_alpha, _38_dsa) + _in.dst.xyz) - _38_dsa) + _in.src.xyz) - _37_sda, (_in.src.w + _in.dst.w) - _36_alpha);
                    continue;
                }

            case 27:
                float4 _39_blend_color;
                float _40_alpha = _in.dst.w * _in.src.w;
                float3 _41_sda = _in.src.xyz * _in.dst.w;
                float3 _42_dsa = _in.dst.xyz * _in.src.w;
                {
                    _0_blend = float4((((_blend_set_color_luminance(_41_sda, _40_alpha, _42_dsa) + _in.dst.xyz) - _42_dsa) + _in.src.xyz) - _41_sda, (_in.src.w + _in.dst.w) - _40_alpha);
                    continue;
                }

            case 28:
                float4 _43_blend_luminosity;
                float _44_alpha = _in.dst.w * _in.src.w;
                float3 _45_sda = _in.src.xyz * _in.dst.w;
                float3 _46_dsa = _in.dst.xyz * _in.src.w;
                {
                    _0_blend = float4((((_blend_set_color_luminance(_46_dsa, _44_alpha, _45_sda) + _in.dst.xyz) - _46_dsa) + _in.src.xyz) - _45_sda, (_in.src.w + _in.dst.w) - _44_alpha);
                    continue;
                }

            default:
                {
                    _0_blend = float4(0.0);
                    continue;
                }
        }
    }
    _out.sk_FragColor = _0_blend;

    return _out;
}
