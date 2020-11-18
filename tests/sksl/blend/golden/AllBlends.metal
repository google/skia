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
            float _48_guarded_divide;
            float _49_n = d.x * s.y;
            {
                _48_guarded_divide = _49_n / delta;
            }
            delta = min(d.y, _48_guarded_divide);

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
        float _50_guarded_divide;
        float _51_n = (d.y - d.x) * s.y;
        {
            _50_guarded_divide = _51_n / s.x;
        }
        float delta = max(0.0, d.y - _50_guarded_divide);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component(float2 s, float2 d) {
    if (2.0 * s.x <= s.y) {
        float _54_guarded_divide;
        float _55_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        {
            _54_guarded_divide = _55_n / d.y;
        }
        return (_54_guarded_divide + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);

    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _56_guarded_divide;
        float _57_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        {
            _56_guarded_divide = _57_n / DaSqd;
        }
        return _56_guarded_divide;

    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
float3 _blend_set_color_luminance(float3 hueSatColor, float alpha, float3 lumColor) {
    float _58_blend_color_luminance;
    {
        _58_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    }
    float lum = _58_blend_color_luminance;

    float _59_blend_color_luminance;
    {
        _59_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);
    }
    float3 result = (lum - _59_blend_color_luminance) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + ((result - lum) * lum) / (lum - minComp);
    }
    return maxComp > alpha && maxComp != lum ? lum + ((result - lum) * (alpha - lum)) / (maxComp - lum) : result;
}
float3 _blend_set_color_saturation(float3 hueLumColor, float3 satColor) {
    float _60_blend_color_saturation;
    {
        _60_blend_color_saturation = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    }
    float sat = _60_blend_color_saturation;

    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            float3 _61_blend_set_color_saturation_helper;
            {
                _61_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.z ? float3(0.0, (sat * (hueLumColor.y - hueLumColor.x)) / (hueLumColor.z - hueLumColor.x), sat) : float3(0.0);
            }
            hueLumColor.xyz = _61_blend_set_color_saturation_helper;

        } else if (hueLumColor.x <= hueLumColor.z) {
            float3 _62_blend_set_color_saturation_helper;
            {
                _62_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.y ? float3(0.0, (sat * (hueLumColor.z - hueLumColor.x)) / (hueLumColor.y - hueLumColor.x), sat) : float3(0.0);
            }
            hueLumColor.xzy = _62_blend_set_color_saturation_helper;

        } else {
            float3 _63_blend_set_color_saturation_helper;
            {
                _63_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.y ? float3(0.0, (sat * (hueLumColor.x - hueLumColor.z)) / (hueLumColor.y - hueLumColor.z), sat) : float3(0.0);
            }
            hueLumColor.zxy = _63_blend_set_color_saturation_helper;

        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        float3 _64_blend_set_color_saturation_helper;
        {
            _64_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.z ? float3(0.0, (sat * (hueLumColor.x - hueLumColor.y)) / (hueLumColor.z - hueLumColor.y), sat) : float3(0.0);
        }
        hueLumColor.yxz = _64_blend_set_color_saturation_helper;

    } else if (hueLumColor.y <= hueLumColor.z) {
        float3 _65_blend_set_color_saturation_helper;
        {
            _65_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.x ? float3(0.0, (sat * (hueLumColor.z - hueLumColor.y)) / (hueLumColor.x - hueLumColor.y), sat) : float3(0.0);
        }
        hueLumColor.yzx = _65_blend_set_color_saturation_helper;

    } else {
        float3 _66_blend_set_color_saturation_helper;
        {
            _66_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.x ? float3(0.0, (sat * (hueLumColor.y - hueLumColor.z)) / (hueLumColor.x - hueLumColor.z), sat) : float3(0.0);
        }
        hueLumColor.zyx = _66_blend_set_color_saturation_helper;

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
        float4 _67_blend_src_in;
        {
            _67_blend_src_in = _in.dst * _in.src.w;
        }
        _6_blend_dst_in = _67_blend_src_in;

    }

    _out->sk_FragColor = _6_blend_dst_in;

    float4 _7_blend_src_out;
    {
        _7_blend_src_out = (1.0 - _in.dst.w) * _in.src;
    }

    _out->sk_FragColor = _7_blend_src_out;

    float4 _8_blend_dst_out;
    {
        _8_blend_dst_out = (1.0 - _in.src.w) * _in.dst;
    }

    _out->sk_FragColor = _8_blend_dst_out;

    float4 _9_blend_src_atop;
    {
        _9_blend_src_atop = _in.dst.w * _in.src + (1.0 - _in.src.w) * _in.dst;
    }

    _out->sk_FragColor = _9_blend_src_atop;

    float4 _10_blend_dst_atop;
    {
        _10_blend_dst_atop = (1.0 - _in.dst.w) * _in.src + _in.src.w * _in.dst;
    }

    _out->sk_FragColor = _10_blend_dst_atop;

    float4 _11_blend_xor;
    {
        _11_blend_xor = (1.0 - _in.dst.w) * _in.src + (1.0 - _in.src.w) * _in.dst;
    }

    _out->sk_FragColor = _11_blend_xor;

    float4 _12_blend_plus;
    {
        _12_blend_plus = min(_in.src + _in.dst, 1.0);
    }

    _out->sk_FragColor = _12_blend_plus;

    float4 _13_blend_modulate;
    {
        _13_blend_modulate = _in.src * _in.dst;
    }

    _out->sk_FragColor = _13_blend_modulate;

    float4 _14_blend_screen;
    {
        _14_blend_screen = _in.src + (1.0 - _in.src) * _in.dst;
    }

    _out->sk_FragColor = _14_blend_screen;

    float4 _15_blend_overlay;
    {
        float _68_blend_overlay_component;
        {
            _68_blend_overlay_component = 2.0 * _in.dst.x <= _in.dst.w ? (2.0 * _in.src.x) * _in.dst.x : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.x)) * (_in.src.w - _in.src.x);
        }
        float _73_blend_overlay_component;
        {
            _73_blend_overlay_component = 2.0 * _in.dst.y <= _in.dst.w ? (2.0 * _in.src.y) * _in.dst.y : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.y)) * (_in.src.w - _in.src.y);
        }
        float _75_blend_overlay_component;
        {
            _75_blend_overlay_component = 2.0 * _in.dst.z <= _in.dst.w ? (2.0 * _in.src.z) * _in.dst.z : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.z)) * (_in.src.w - _in.src.z);
        }
        float4 _16_result = float4(_68_blend_overlay_component, _73_blend_overlay_component, _75_blend_overlay_component, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);



        _16_result.xyz = _16_result.xyz + _in.dst.xyz * (1.0 - _in.src.w) + _in.src.xyz * (1.0 - _in.dst.w);
        _15_blend_overlay = _16_result;
    }

    _out->sk_FragColor = _15_blend_overlay;

    float4 _17_blend_darken;
    {
        float4 _69_blend_src_over;
        {
            _69_blend_src_over = _in.src + (1.0 - _in.src.w) * _in.dst;
        }
        float4 _18_result = _69_blend_src_over;

        _18_result.xyz = min(_18_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
        _17_blend_darken = _18_result;
    }

    _out->sk_FragColor = _17_blend_darken;

    float4 _19_blend_lighten;
    {
        float4 _70_blend_src_over;
        {
            _70_blend_src_over = _in.src + (1.0 - _in.src.w) * _in.dst;
        }
        float4 _20_result = _70_blend_src_over;

        _20_result.xyz = max(_20_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
        _19_blend_lighten = _20_result;
    }

    _out->sk_FragColor = _19_blend_lighten;

    float4 _21_blend_color_dodge;
    {
        _21_blend_color_dodge = float4(_color_dodge_component(_in.src.xw, _in.dst.xw), _color_dodge_component(_in.src.yw, _in.dst.yw), _color_dodge_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }

    _out->sk_FragColor = _21_blend_color_dodge;

    float4 _22_blend_color_burn;
    {
        _22_blend_color_burn = float4(_color_burn_component(_in.src.xw, _in.dst.xw), _color_burn_component(_in.src.yw, _in.dst.yw), _color_burn_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }

    _out->sk_FragColor = _22_blend_color_burn;

    float4 _23_blend_hard_light;
    {
        float4 _71_blend_overlay;
        {
            float _74_blend_overlay_component;
            {
                _74_blend_overlay_component = 2.0 * _in.src.x <= _in.src.w ? (2.0 * _in.dst.x) * _in.src.x : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.x)) * (_in.dst.w - _in.dst.x);
            }
            float _76_blend_overlay_component;
            {
                _76_blend_overlay_component = 2.0 * _in.src.y <= _in.src.w ? (2.0 * _in.dst.y) * _in.src.y : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.y)) * (_in.dst.w - _in.dst.y);
            }
            float _77_blend_overlay_component;
            {
                _77_blend_overlay_component = 2.0 * _in.src.z <= _in.src.w ? (2.0 * _in.dst.z) * _in.src.z : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.z)) * (_in.dst.w - _in.dst.z);
            }
            float4 _72_result = float4(_74_blend_overlay_component, _76_blend_overlay_component, _77_blend_overlay_component, _in.dst.w + (1.0 - _in.dst.w) * _in.src.w);



            _72_result.xyz = _72_result.xyz + _in.src.xyz * (1.0 - _in.dst.w) + _in.dst.xyz * (1.0 - _in.src.w);
            _71_blend_overlay = _72_result;
        }
        _23_blend_hard_light = _71_blend_overlay;

    }

    _out->sk_FragColor = _23_blend_hard_light;

    float4 _24_blend_soft_light;
    {
        _24_blend_soft_light = _in.dst.w == 0.0 ? _in.src : float4(_soft_light_component(_in.src.xw, _in.dst.xw), _soft_light_component(_in.src.yw, _in.dst.yw), _soft_light_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }

    _out->sk_FragColor = _24_blend_soft_light;

    float4 _25_blend_difference;
    {
        _25_blend_difference = float4((_in.src.xyz + _in.dst.xyz) - 2.0 * min(_in.src.xyz * _in.dst.w, _in.dst.xyz * _in.src.w), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }

    _out->sk_FragColor = _25_blend_difference;

    float4 _26_blend_exclusion;
    {
        _26_blend_exclusion = float4((_in.dst.xyz + _in.src.xyz) - (2.0 * _in.dst.xyz) * _in.src.xyz, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }

    _out->sk_FragColor = _26_blend_exclusion;

    float4 _27_blend_multiply;
    {
        _27_blend_multiply = float4(((1.0 - _in.src.w) * _in.dst.xyz + (1.0 - _in.dst.w) * _in.src.xyz) + _in.src.xyz * _in.dst.xyz, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    }

    _out->sk_FragColor = _27_blend_multiply;

    float4 _28_blend_hue;
    {
        float _29_alpha = _in.dst.w * _in.src.w;
        float3 _30_sda = _in.src.xyz * _in.dst.w;
        float3 _31_dsa = _in.dst.xyz * _in.src.w;
        _28_blend_hue = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_30_sda, _31_dsa), _29_alpha, _31_dsa) + _in.dst.xyz) - _31_dsa) + _in.src.xyz) - _30_sda, (_in.src.w + _in.dst.w) - _29_alpha);
    }

    _out->sk_FragColor = _28_blend_hue;

    float4 _32_blend_saturation;
    {
        float _33_alpha = _in.dst.w * _in.src.w;
        float3 _34_sda = _in.src.xyz * _in.dst.w;
        float3 _35_dsa = _in.dst.xyz * _in.src.w;
        _32_blend_saturation = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_35_dsa, _34_sda), _33_alpha, _35_dsa) + _in.dst.xyz) - _35_dsa) + _in.src.xyz) - _34_sda, (_in.src.w + _in.dst.w) - _33_alpha);
    }

    _out->sk_FragColor = _32_blend_saturation;

    float4 _36_blend_color;
    {
        float _37_alpha = _in.dst.w * _in.src.w;
        float3 _38_sda = _in.src.xyz * _in.dst.w;
        float3 _39_dsa = _in.dst.xyz * _in.src.w;
        _36_blend_color = float4((((_blend_set_color_luminance(_38_sda, _37_alpha, _39_dsa) + _in.dst.xyz) - _39_dsa) + _in.src.xyz) - _38_sda, (_in.src.w + _in.dst.w) - _37_alpha);
    }

    _out->sk_FragColor = _36_blend_color;

    float4 _40_blend_luminosity;
    {
        float _41_alpha = _in.dst.w * _in.src.w;
        float3 _42_sda = _in.src.xyz * _in.dst.w;
        float3 _43_dsa = _in.dst.xyz * _in.src.w;
        _40_blend_luminosity = float4((((_blend_set_color_luminance(_43_dsa, _41_alpha, _42_sda) + _in.dst.xyz) - _43_dsa) + _in.src.xyz) - _42_sda, (_in.src.w + _in.dst.w) - _41_alpha);
    }

    _out->sk_FragColor = _40_blend_luminosity;

    return *_out;
}
