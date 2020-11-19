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
float _color_dodge_component(float2 s, float2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            float _4_guarded_divide;
            float _5_n = d.x * s.y;
            {
                _4_guarded_divide = _5_n / delta;
            }
            delta = min(d.y, _4_guarded_divide);

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
        float _6_guarded_divide;
        float _7_n = (d.y - d.x) * s.y;
        {
            _6_guarded_divide = _7_n / s.x;
        }
        float delta = max(0.0, d.y - _6_guarded_divide);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component(float2 s, float2 d) {
    if (2.0 * s.x <= s.y) {
        float _11_guarded_divide;
        float _12_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        {
            _11_guarded_divide = _12_n / d.y;
        }
        return (_11_guarded_divide + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);

    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _13_guarded_divide;
        float _14_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        {
            _13_guarded_divide = _14_n / DaSqd;
        }
        return _13_guarded_divide;

    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
float3 _blend_set_color_luminance(float3 hueSatColor, float alpha, float3 lumColor) {
    float _15_blend_color_luminance;
    {
        _15_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    }
    float lum = _15_blend_color_luminance;

    float _16_blend_color_luminance;
    {
        _16_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);
    }
    float3 result = (lum - _16_blend_color_luminance) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + ((result - lum) * lum) / (lum - minComp);
    }
    return maxComp > alpha && maxComp != lum ? lum + ((result - lum) * (alpha - lum)) / (maxComp - lum) : result;
}
float3 _blend_set_color_saturation(float3 hueLumColor, float3 satColor) {
    float _17_blend_color_saturation;
    {
        _17_blend_color_saturation = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    }
    float sat = _17_blend_color_saturation;

    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            float3 _18_blend_set_color_saturation_helper;
            {
                _18_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.z ? float3(0.0, (sat * (hueLumColor.y - hueLumColor.x)) / (hueLumColor.z - hueLumColor.x), sat) : float3(0.0);
            }
            hueLumColor.xyz = _18_blend_set_color_saturation_helper;

        } else if (hueLumColor.x <= hueLumColor.z) {
            float3 _19_blend_set_color_saturation_helper;
            {
                _19_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.y ? float3(0.0, (sat * (hueLumColor.z - hueLumColor.x)) / (hueLumColor.y - hueLumColor.x), sat) : float3(0.0);
            }
            hueLumColor.xzy = _19_blend_set_color_saturation_helper;

        } else {
            float3 _20_blend_set_color_saturation_helper;
            {
                _20_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.y ? float3(0.0, (sat * (hueLumColor.x - hueLumColor.z)) / (hueLumColor.y - hueLumColor.z), sat) : float3(0.0);
            }
            hueLumColor.zxy = _20_blend_set_color_saturation_helper;

        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        float3 _21_blend_set_color_saturation_helper;
        {
            _21_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.z ? float3(0.0, (sat * (hueLumColor.x - hueLumColor.y)) / (hueLumColor.z - hueLumColor.y), sat) : float3(0.0);
        }
        hueLumColor.yxz = _21_blend_set_color_saturation_helper;

    } else if (hueLumColor.y <= hueLumColor.z) {
        float3 _22_blend_set_color_saturation_helper;
        {
            _22_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.x ? float3(0.0, (sat * (hueLumColor.z - hueLumColor.y)) / (hueLumColor.x - hueLumColor.y), sat) : float3(0.0);
        }
        hueLumColor.yzx = _22_blend_set_color_saturation_helper;

    } else {
        float3 _23_blend_set_color_saturation_helper;
        {
            _23_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.x ? float3(0.0, (sat * (hueLumColor.y - hueLumColor.z)) / (hueLumColor.x - hueLumColor.z), sat) : float3(0.0);
        }
        hueLumColor.zyx = _23_blend_set_color_saturation_helper;

    }
    return hueLumColor;
}


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = float4(0.0);

    float4 _1_blend_src;
    {
        _1_blend_src = _in.src;
    }
    _out->sk_FragColor = _1_blend_src;

    float4 _2_blend_dst;
    {
        _2_blend_dst = _in.dst;
    }
    _out->sk_FragColor = _2_blend_dst;

    float4 _3_blend_src_over;
    {
        _3_blend_src_over = _in.src + (1.0 - _in.src.w) * _in.dst;
    }
    _out->sk_FragColor = _3_blend_src_over;

    float4 _4_blend_dst_over;
    {
        _4_blend_dst_over = (1.0 - _in.dst.w) * _in.src + _in.dst;
    }
    _out->sk_FragColor = _4_blend_dst_over;

    float4 _5_blend_src_in;
    {
        _5_blend_src_in = _in.src * _in.dst.w;
    }
    _out->sk_FragColor = _5_blend_src_in;

    float4 _6_blend_dst_in;
    {
        float4 _7_0_blend_src_in;
        {
            _7_0_blend_src_in = _in.dst * _in.src.w;
        }
        _6_blend_dst_in = _7_0_blend_src_in;

    }
    _out->sk_FragColor = _6_blend_dst_in;

    float4 _8_blend_src_out;
    {
        _8_blend_src_out = (1.0 - _in.dst.w) * _in.src;
    }
    _out->sk_FragColor = _8_blend_src_out;

    float4 _9_blend_dst_out;
    {
        _9_blend_dst_out = (1.0 - _in.src.w) * _in.dst;
    }
    _out->sk_FragColor = _9_blend_dst_out;

    float4 _10_blend_src_atop;
    {
        _10_blend_src_atop = _in.dst.w * _in.src + (1.0 - _in.src.w) * _in.dst;
    }
    _out->sk_FragColor = _10_blend_src_atop;

    float4 _11_blend_dst_atop;
    {
        _11_blend_dst_atop = (1.0 - _in.dst.w) * _in.src + _in.src.w * _in.dst;
    }
    _out->sk_FragColor = _11_blend_dst_atop;

    float4 _12_blend_xor;
    {
        _12_blend_xor = (1.0 - _in.dst.w) * _in.src + (1.0 - _in.src.w) * _in.dst;
    }
    _out->sk_FragColor = _12_blend_xor;

    float4 _13_blend_plus;
    {
        _13_blend_plus = min(_in.src + _in.dst, 1.0);
    }
    _out->sk_FragColor = _13_blend_plus;

    float4 _14_blend_modulate;
    {
        _14_blend_modulate = _in.src * _in.dst;
    }
    _out->sk_FragColor = _14_blend_modulate;

    float4 _15_blend_screen;
    {
        _15_blend_screen = _in.src + (1.0 - _in.src) * _in.dst;
    }
    _out->sk_FragColor = _15_blend_screen;

    float4 _16_blend_overlay;
    {
        float _17_1_blend_overlay_component;
        {
            _17_1_blend_overlay_component = 2.0 * _in.dst.x <= _in.dst.w ? (2.0 * _in.src.x) * _in.dst.x : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.x)) * (_in.src.w - _in.src.x);
        }
        float _18_75_blend_overlay_component;
        {
            _18_75_blend_overlay_component = 2.0 * _in.dst.y <= _in.dst.w ? (2.0 * _in.src.y) * _in.dst.y : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.y)) * (_in.src.w - _in.src.y);
        }
        float _19_79_blend_overlay_component;
        {
            _19_79_blend_overlay_component = 2.0 * _in.dst.z <= _in.dst.w ? (2.0 * _in.src.z) * _in.dst.z : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.z)) * (_in.src.w - _in.src.z);
        }
        float4 _20_result = float4(_17_1_blend_overlay_component, _18_75_blend_overlay_component, _19_79_blend_overlay_component, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);



        _20_result.xyz = _20_result.xyz + _in.dst.xyz * (1.0 - _in.src.w) + _in.src.xyz * (1.0 - _in.dst.w);
        _16_blend_overlay = _20_result;
    }
    _out->sk_FragColor = _16_blend_overlay;

    float4 _21_blend_darken;
    {
        float4 _22_2_blend_src_over;
        {
            _22_2_blend_src_over = _in.src + (1.0 - _in.src.w) * _in.dst;
        }
        float4 _23_result = _22_2_blend_src_over;

        _23_result.xyz = min(_23_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
        _21_blend_darken = _23_result;
    }
    _out->sk_FragColor = _21_blend_darken;

    float4 _24_blend_lighten;
    {
        float4 _25_3_blend_src_over;
        {
            _25_3_blend_src_over = _in.src + (1.0 - _in.src.w) * _in.dst;
        }
        float4 _26_result = _25_3_blend_src_over;

        _26_result.xyz = max(_26_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
        _24_blend_lighten = _26_result;
    }
    _out->sk_FragColor = _24_blend_lighten;

    float4 _27_blend_color_dodge;
    {
        _27_blend_color_dodge = float4(_color_dodge_component(_in.src.xw, _in.dst.xw), _color_dodge_component(_in.src.yw, _in.dst.yw), _color_dodge_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }
    _out->sk_FragColor = _27_blend_color_dodge;

    float4 _28_blend_color_burn;
    {
        _28_blend_color_burn = float4(_color_burn_component(_in.src.xw, _in.dst.xw), _color_burn_component(_in.src.yw, _in.dst.yw), _color_burn_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }
    _out->sk_FragColor = _28_blend_color_burn;

    float4 _29_blend_hard_light;
    {
        float4 _30_8_blend_overlay;
        {
            float _31_9_1_blend_overlay_component;
            {
                _31_9_1_blend_overlay_component = 2.0 * _in.src.x <= _in.src.w ? (2.0 * _in.dst.x) * _in.src.x : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.x)) * (_in.dst.w - _in.dst.x);
            }
            float _32_76_blend_overlay_component;
            {
                _32_76_blend_overlay_component = 2.0 * _in.src.y <= _in.src.w ? (2.0 * _in.dst.y) * _in.src.y : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.y)) * (_in.dst.w - _in.dst.y);
            }
            float _33_80_blend_overlay_component;
            {
                _33_80_blend_overlay_component = 2.0 * _in.src.z <= _in.src.w ? (2.0 * _in.dst.z) * _in.src.z : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.z)) * (_in.dst.w - _in.dst.z);
            }
            float4 _34_10_result = float4(_31_9_1_blend_overlay_component, _32_76_blend_overlay_component, _33_80_blend_overlay_component, _in.dst.w + (1.0 - _in.dst.w) * _in.src.w);



            _34_10_result.xyz = _34_10_result.xyz + _in.src.xyz * (1.0 - _in.dst.w) + _in.dst.xyz * (1.0 - _in.src.w);
            _30_8_blend_overlay = _34_10_result;
        }
        _29_blend_hard_light = _30_8_blend_overlay;

    }
    _out->sk_FragColor = _29_blend_hard_light;

    float4 _35_blend_soft_light;
    {
        _35_blend_soft_light = _in.dst.w == 0.0 ? _in.src : float4(_soft_light_component(_in.src.xw, _in.dst.xw), _soft_light_component(_in.src.yw, _in.dst.yw), _soft_light_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }
    _out->sk_FragColor = _35_blend_soft_light;

    float4 _36_blend_difference;
    {
        _36_blend_difference = float4((_in.src.xyz + _in.dst.xyz) - 2.0 * min(_in.src.xyz * _in.dst.w, _in.dst.xyz * _in.src.w), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }
    _out->sk_FragColor = _36_blend_difference;

    float4 _37_blend_exclusion;
    {
        _37_blend_exclusion = float4((_in.dst.xyz + _in.src.xyz) - (2.0 * _in.dst.xyz) * _in.src.xyz, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }
    _out->sk_FragColor = _37_blend_exclusion;

    float4 _38_blend_multiply;
    {
        _38_blend_multiply = float4(((1.0 - _in.src.w) * _in.dst.xyz + (1.0 - _in.dst.w) * _in.src.xyz) + _in.src.xyz * _in.dst.xyz, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }
    _out->sk_FragColor = _38_blend_multiply;

    float4 _39_blend_hue;
    {
        float _40_alpha = _in.dst.w * _in.src.w;
        float3 _41_sda = _in.src.xyz * _in.dst.w;
        float3 _42_dsa = _in.dst.xyz * _in.src.w;
        _39_blend_hue = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_41_sda, _42_dsa), _40_alpha, _42_dsa) + _in.dst.xyz) - _42_dsa) + _in.src.xyz) - _41_sda, (_in.src.w + _in.dst.w) - _40_alpha);
    }
    _out->sk_FragColor = _39_blend_hue;

    float4 _43_blend_saturation;
    {
        float _44_alpha = _in.dst.w * _in.src.w;
        float3 _45_sda = _in.src.xyz * _in.dst.w;
        float3 _46_dsa = _in.dst.xyz * _in.src.w;
        _43_blend_saturation = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_46_dsa, _45_sda), _44_alpha, _46_dsa) + _in.dst.xyz) - _46_dsa) + _in.src.xyz) - _45_sda, (_in.src.w + _in.dst.w) - _44_alpha);
    }
    _out->sk_FragColor = _43_blend_saturation;

    float4 _47_blend_color;
    {
        float _48_alpha = _in.dst.w * _in.src.w;
        float3 _49_sda = _in.src.xyz * _in.dst.w;
        float3 _50_dsa = _in.dst.xyz * _in.src.w;
        _47_blend_color = float4((((_blend_set_color_luminance(_49_sda, _48_alpha, _50_dsa) + _in.dst.xyz) - _50_dsa) + _in.src.xyz) - _49_sda, (_in.src.w + _in.dst.w) - _48_alpha);
    }
    _out->sk_FragColor = _47_blend_color;

    float4 _51_blend_luminosity;
    {
        float _52_alpha = _in.dst.w * _in.src.w;
        float3 _53_sda = _in.src.xyz * _in.dst.w;
        float3 _54_dsa = _in.dst.xyz * _in.src.w;
        _51_blend_luminosity = float4((((_blend_set_color_luminance(_54_dsa, _52_alpha, _53_sda) + _in.dst.xyz) - _54_dsa) + _in.src.xyz) - _53_sda, (_in.src.w + _in.dst.w) - _52_alpha);
    }
    _out->sk_FragColor = _51_blend_luminosity;

    return *_out;
}
