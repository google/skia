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
            _3_guarded_divide = _4_n / delta;

            delta = min(d.y, _3_guarded_divide);

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
        _5_guarded_divide = _6_n / s.x;

        float delta = max(0.0, d.y - _5_guarded_divide);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component(float2 s, float2 d) {
    if (2.0 * s.x <= s.y) {
        float _7_guarded_divide;
        float _8_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        _7_guarded_divide = _8_n / d.y;

        return (_7_guarded_divide + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);

    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _9_guarded_divide;
        float _10_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        _9_guarded_divide = _10_n / DaSqd;

        return _9_guarded_divide;

    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
float3 _blend_set_color_luminance(float3 hueSatColor, float alpha, float3 lumColor) {
    float _11_blend_color_luminance;
    _11_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);

    float lum = _11_blend_color_luminance;

    float _12_blend_color_luminance;
    _12_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);

    float3 result = (lum - _12_blend_color_luminance) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + ((result - lum) * lum) / (lum - minComp);
    }
    return maxComp > alpha && maxComp != lum ? lum + ((result - lum) * (alpha - lum)) / (maxComp - lum) : result;
}
float3 _blend_set_color_saturation_helper(float3 minMidMax, float sat) {
    return minMidMax.x < minMidMax.z ? float3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat) : float3(0.0);
}
float3 _blend_set_color_saturation(float3 hueLumColor, float3 satColor) {
    float _13_blend_color_saturation;
    _13_blend_color_saturation = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);

    float sat = _13_blend_color_saturation;

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
float4 blend(int mode, float4 src, float4 dst) {
    switch (mode) {
        case 0:
            return float4(0.0);

        case 1:
            float4 _15_blend_src;
            _15_blend_src = src;

            return _15_blend_src;

        case 2:
            float4 _16_blend_dst;
            _16_blend_dst = dst;

            return _16_blend_dst;

        case 3:
            float4 _17_blend_src_over;
            _17_blend_src_over = src + (1.0 - src.w) * dst;

            return _17_blend_src_over;

        case 4:
            float4 _18_blend_dst_over;
            _18_blend_dst_over = (1.0 - dst.w) * src + dst;

            return _18_blend_dst_over;

        case 5:
            float4 _19_blend_src_in;
            _19_blend_src_in = src * dst.w;

            return _19_blend_src_in;

        case 6:
            float4 _20_blend_dst_in;
            float4 _21_blend_src_in;
            _21_blend_src_in = dst * src.w;

            _20_blend_dst_in = _21_blend_src_in;


            return _20_blend_dst_in;

        case 7:
            float4 _22_blend_src_out;
            _22_blend_src_out = (1.0 - dst.w) * src;

            return _22_blend_src_out;

        case 8:
            float4 _23_blend_dst_out;
            _23_blend_dst_out = (1.0 - src.w) * dst;

            return _23_blend_dst_out;

        case 9:
            float4 _24_blend_src_atop;
            _24_blend_src_atop = dst.w * src + (1.0 - src.w) * dst;

            return _24_blend_src_atop;

        case 10:
            float4 _25_blend_dst_atop;
            _25_blend_dst_atop = (1.0 - dst.w) * src + src.w * dst;

            return _25_blend_dst_atop;

        case 11:
            float4 _26_blend_xor;
            _26_blend_xor = (1.0 - dst.w) * src + (1.0 - src.w) * dst;

            return _26_blend_xor;

        case 12:
            float4 _27_blend_plus;
            _27_blend_plus = min(src + dst, 1.0);

            return _27_blend_plus;

        case 13:
            float4 _28_blend_modulate;
            _28_blend_modulate = src * dst;

            return _28_blend_modulate;

        case 14:
            float4 _29_blend_screen;
            _29_blend_screen = src + (1.0 - src) * dst;

            return _29_blend_screen;

        case 15:
            return blend_overlay(src, dst);
        case 16:
            float4 _30_blend_darken;
            float4 _31_blend_src_over;
            _31_blend_src_over = src + (1.0 - src.w) * dst;

            float4 _32_result = _31_blend_src_over;

            _32_result.xyz = min(_32_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
            _30_blend_darken = _32_result;

            return _30_blend_darken;

        case 17:
            float4 _33_blend_lighten;
            float4 _34_blend_src_over;
            _34_blend_src_over = src + (1.0 - src.w) * dst;

            float4 _35_result = _34_blend_src_over;

            _35_result.xyz = max(_35_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
            _33_blend_lighten = _35_result;

            return _33_blend_lighten;

        case 18:
            float4 _36_blend_color_dodge;
            _36_blend_color_dodge = float4(_color_dodge_component(src.xw, dst.xw), _color_dodge_component(src.yw, dst.yw), _color_dodge_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);

            return _36_blend_color_dodge;

        case 19:
            float4 _37_blend_color_burn;
            _37_blend_color_burn = float4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);

            return _37_blend_color_burn;

        case 20:
            float4 _38_blend_hard_light;
            _38_blend_hard_light = blend_overlay(dst, src);

            return _38_blend_hard_light;

        case 21:
            float4 _39_blend_soft_light;
            _39_blend_soft_light = dst.w == 0.0 ? src : float4(_soft_light_component(src.xw, dst.xw), _soft_light_component(src.yw, dst.yw), _soft_light_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);

            return _39_blend_soft_light;

        case 22:
            float4 _40_blend_difference;
            _40_blend_difference = float4((src.xyz + dst.xyz) - 2.0 * min(src.xyz * dst.w, dst.xyz * src.w), src.w + (1.0 - src.w) * dst.w);

            return _40_blend_difference;

        case 23:
            float4 _41_blend_exclusion;
            _41_blend_exclusion = float4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);

            return _41_blend_exclusion;

        case 24:
            float4 _42_blend_multiply;
            _42_blend_multiply = float4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);

            return _42_blend_multiply;

        case 25:
            float4 _43_blend_hue;
            float _44_alpha = dst.w * src.w;
            float3 _45_sda = src.xyz * dst.w;
            float3 _46_dsa = dst.xyz * src.w;
            _43_blend_hue = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_45_sda, _46_dsa), _44_alpha, _46_dsa) + dst.xyz) - _46_dsa) + src.xyz) - _45_sda, (src.w + dst.w) - _44_alpha);

            return _43_blend_hue;

        case 26:
            float4 _47_blend_saturation;
            float _48_alpha = dst.w * src.w;
            float3 _49_sda = src.xyz * dst.w;
            float3 _50_dsa = dst.xyz * src.w;
            _47_blend_saturation = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_50_dsa, _49_sda), _48_alpha, _50_dsa) + dst.xyz) - _50_dsa) + src.xyz) - _49_sda, (src.w + dst.w) - _48_alpha);

            return _47_blend_saturation;

        case 27:
            float4 _51_blend_color;
            float _52_alpha = dst.w * src.w;
            float3 _53_sda = src.xyz * dst.w;
            float3 _54_dsa = dst.xyz * src.w;
            _51_blend_color = float4((((_blend_set_color_luminance(_53_sda, _52_alpha, _54_dsa) + dst.xyz) - _54_dsa) + src.xyz) - _53_sda, (src.w + dst.w) - _52_alpha);

            return _51_blend_color;

        case 28:
            float4 _55_blend_luminosity;
            float _56_alpha = dst.w * src.w;
            float3 _57_sda = src.xyz * dst.w;
            float3 _58_dsa = dst.xyz * src.w;
            _55_blend_luminosity = float4((((_blend_set_color_luminance(_58_dsa, _56_alpha, _57_sda) + dst.xyz) - _58_dsa) + src.xyz) - _57_sda, (src.w + dst.w) - _56_alpha);

            return _55_blend_luminosity;

    }
    return float4(0.0);
}


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = blend(13, _in.src, _in.dst);
    return *_out;
}
