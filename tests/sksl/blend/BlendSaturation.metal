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


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 _0_blend_saturation;
    float _1_alpha = _in.dst.w * _in.src.w;
    float3 _2_sda = _in.src.xyz * _in.dst.w;
    float3 _3_dsa = _in.dst.xyz * _in.src.w;
    float3 _4_blend_set_color_saturation;
    float _5_blend_color_saturation;
    float _6_sat = max(max(_2_sda.x, _2_sda.y), _2_sda.z) - min(min(_2_sda.x, _2_sda.y), _2_sda.z);

    if (_3_dsa.x <= _3_dsa.y) {
        if (_3_dsa.y <= _3_dsa.z) {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa, _6_sat);
        } else if (_3_dsa.x <= _3_dsa.z) {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa.xzy, _6_sat).xzy;
        } else {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa.zxy, _6_sat).yzx;
        }
    } else if (_3_dsa.x <= _3_dsa.z) {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa.yxz, _6_sat).yxz;
    } else if (_3_dsa.y <= _3_dsa.z) {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa.yzx, _6_sat).zxy;
    } else {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa.zyx, _6_sat).zyx;
    }
    float3 _7_blend_set_color_luminance;
    float _8_blend_color_luminance;
    float _9_lum = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);

    float _10_blend_color_luminance;
    float3 _11_result = (_9_lum - dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _4_blend_set_color_saturation)) + _4_blend_set_color_saturation;

    float _12_minComp = min(min(_11_result.x, _11_result.y), _11_result.z);
    float _13_maxComp = max(max(_11_result.x, _11_result.y), _11_result.z);
    if (_12_minComp < 0.0 && _9_lum != _12_minComp) {
        float _14_guarded_divide;
        float _15_d = _9_lum - _12_minComp;
        _11_result = _9_lum + (_11_result - _9_lum) * (_9_lum / _15_d);

    }
    if (_13_maxComp > _1_alpha && _13_maxComp != _9_lum) {
        float3 _16_guarded_divide;
        float3 _17_n = (_11_result - _9_lum) * (_1_alpha - _9_lum);
        float _18_d = _13_maxComp - _9_lum;
        _7_blend_set_color_luminance = _9_lum + _17_n / _18_d;

    } else {
        _7_blend_set_color_luminance = _11_result;
    }
    _out.sk_FragColor = float4((((_7_blend_set_color_luminance + _in.dst.xyz) - _3_dsa) + _in.src.xyz) - _2_sda, (_in.src.w + _in.dst.w) - _1_alpha);



    return _out;
}
