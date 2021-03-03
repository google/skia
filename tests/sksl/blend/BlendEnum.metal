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
            float _0_n = d.x * s.y;
            delta = min(d.y, _0_n / delta);

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
        float _1_n = (d.y - d.x) * s.y;
        float delta = max(0.0, d.y - _1_n / s.x);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component(float2 s, float2 d) {
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
float3 _blend_set_color_luminance(float3 hueSatColor, float alpha, float3 lumColor) {
    float lum = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);

    float3 result = (lum - dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor)) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        float _4_d = lum - minComp;
        result = lum + (result - lum) * (lum / _4_d);

    }
    if (maxComp > alpha && maxComp != lum) {
        float3 _5_n = (result - lum) * (alpha - lum);
        float _6_d = maxComp - lum;
        return lum + _5_n / _6_d;

    } else {
        return result;
    }
}
float3 _blend_set_color_saturation_helper(float3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        float _7_n = sat * (minMidMax.y - minMidMax.x);
        float _8_d = minMidMax.z - minMidMax.x;
        return float3(0.0, _7_n / _8_d, sat);

    } else {
        return float3(0.0);
    }
}
float3 _blend_set_color_saturation(float3 hueLumColor, float3 satColor) {
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
                {
                    _0_blend = float4(0.0);
                    continue;
                }

            case 1:
                {
                    _0_blend = _in.src;
                    continue;
                }

            case 2:
                {
                    _0_blend = _in.dst;
                    continue;
                }

            case 3:
                {
                    _0_blend = _in.src + (1.0 - _in.src.w) * _in.dst;
                    continue;
                }

            case 4:
                {
                    _0_blend = (1.0 - _in.dst.w) * _in.src + _in.dst;
                    continue;
                }

            case 5:
                {
                    _0_blend = _in.src * _in.dst.w;
                    continue;
                }

            case 6:
                {
                    _0_blend = _in.dst * _in.src.w;
                    continue;
                }

            case 7:
                {
                    _0_blend = (1.0 - _in.dst.w) * _in.src;
                    continue;
                }

            case 8:
                {
                    _0_blend = (1.0 - _in.src.w) * _in.dst;
                    continue;
                }

            case 9:
                {
                    _0_blend = _in.dst.w * _in.src + (1.0 - _in.src.w) * _in.dst;
                    continue;
                }

            case 10:
                {
                    _0_blend = (1.0 - _in.dst.w) * _in.src + _in.src.w * _in.dst;
                    continue;
                }

            case 11:
                {
                    _0_blend = (1.0 - _in.dst.w) * _in.src + (1.0 - _in.src.w) * _in.dst;
                    continue;
                }

            case 12:
                {
                    _0_blend = min(_in.src + _in.dst, 1.0);
                    continue;
                }

            case 13:
                {
                    _0_blend = _in.src * _in.dst;
                    continue;
                }

            case 14:
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
                float4 _2_result = _in.src + (1.0 - _in.src.w) * _in.dst;

                _2_result.xyz = min(_2_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
                {
                    _0_blend = _2_result;
                    continue;
                }

            case 17:
                float4 _3_result = _in.src + (1.0 - _in.src.w) * _in.dst;

                _3_result.xyz = max(_3_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
                {
                    _0_blend = _3_result;
                    continue;
                }

            case 18:
                {
                    _0_blend = float4(_color_dodge_component(_in.src.xw, _in.dst.xw), _color_dodge_component(_in.src.yw, _in.dst.yw), _color_dodge_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 19:
                {
                    _0_blend = float4(_color_burn_component(_in.src.xw, _in.dst.xw), _color_burn_component(_in.src.yw, _in.dst.yw), _color_burn_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 20:
                {
                    _0_blend = blend_overlay(_in.dst, _in.src);
                    continue;
                }

            case 21:
                {
                    _0_blend = _in.dst.w == 0.0 ? _in.src : float4(_soft_light_component(_in.src.xw, _in.dst.xw), _soft_light_component(_in.src.yw, _in.dst.yw), _soft_light_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 22:
                {
                    _0_blend = float4((_in.src.xyz + _in.dst.xyz) - 2.0 * min(_in.src.xyz * _in.dst.w, _in.dst.xyz * _in.src.w), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 23:
                {
                    _0_blend = float4((_in.dst.xyz + _in.src.xyz) - (2.0 * _in.dst.xyz) * _in.src.xyz, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 24:
                {
                    _0_blend = float4(((1.0 - _in.src.w) * _in.dst.xyz + (1.0 - _in.dst.w) * _in.src.xyz) + _in.src.xyz * _in.dst.xyz, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
                    continue;
                }

            case 25:
                float _4_alpha = _in.dst.w * _in.src.w;
                float3 _5_sda = _in.src.xyz * _in.dst.w;
                float3 _6_dsa = _in.dst.xyz * _in.src.w;
                {
                    _0_blend = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_5_sda, _6_dsa), _4_alpha, _6_dsa) + _in.dst.xyz) - _6_dsa) + _in.src.xyz) - _5_sda, (_in.src.w + _in.dst.w) - _4_alpha);
                    continue;
                }

            case 26:
                float _7_alpha = _in.dst.w * _in.src.w;
                float3 _8_sda = _in.src.xyz * _in.dst.w;
                float3 _9_dsa = _in.dst.xyz * _in.src.w;
                {
                    _0_blend = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_9_dsa, _8_sda), _7_alpha, _9_dsa) + _in.dst.xyz) - _9_dsa) + _in.src.xyz) - _8_sda, (_in.src.w + _in.dst.w) - _7_alpha);
                    continue;
                }

            case 27:
                float _10_alpha = _in.dst.w * _in.src.w;
                float3 _11_sda = _in.src.xyz * _in.dst.w;
                float3 _12_dsa = _in.dst.xyz * _in.src.w;
                {
                    _0_blend = float4((((_blend_set_color_luminance(_11_sda, _10_alpha, _12_dsa) + _in.dst.xyz) - _12_dsa) + _in.src.xyz) - _11_sda, (_in.src.w + _in.dst.w) - _10_alpha);
                    continue;
                }

            case 28:
                float _13_alpha = _in.dst.w * _in.src.w;
                float3 _14_sda = _in.src.xyz * _in.dst.w;
                float3 _15_dsa = _in.dst.xyz * _in.src.w;
                {
                    _0_blend = float4((((_blend_set_color_luminance(_15_dsa, _13_alpha, _14_sda) + _in.dst.xyz) - _15_dsa) + _in.src.xyz) - _14_sda, (_in.src.w + _in.dst.w) - _13_alpha);
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
