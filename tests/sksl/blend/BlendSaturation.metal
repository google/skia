#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 src;
    float4 dst;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float3 _blend_set_color_saturation_helper(float3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        float _7_n = sat * (minMidMax.y - minMidMax.x);
        float _8_d = minMidMax.z - minMidMax.x;
        return float3(0.0, _7_n / _8_d, sat);
    } else {
        return float3(0.0);
    }
}

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float _0_alpha = _uniforms.dst.w * _uniforms.src.w;
    float3 _1_sda = _uniforms.src.xyz * _uniforms.dst.w;
    float3 _2_dsa = _uniforms.dst.xyz * _uniforms.src.w;
    float3 _3_blend_set_color_saturation;
    float _4_sat = max(max(_1_sda.x, _1_sda.y), _1_sda.z) - min(min(_1_sda.x, _1_sda.y), _1_sda.z);
    if (_2_dsa.x <= _2_dsa.y) {
        if (_2_dsa.y <= _2_dsa.z) {
            _3_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_dsa, _4_sat);
        } else if (_2_dsa.x <= _2_dsa.z) {
            _3_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_dsa.xzy, _4_sat).xzy;
        } else {
            _3_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_dsa.zxy, _4_sat).yzx;
        }
    } else if (_2_dsa.x <= _2_dsa.z) {
        _3_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_dsa.yxz, _4_sat).yxz;
    } else if (_2_dsa.y <= _2_dsa.z) {
        _3_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_dsa.yzx, _4_sat).zxy;
    } else {
        _3_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_dsa.zyx, _4_sat).zyx;
    }
    float3 _5_blend_set_color_luminance;
    float _6_lum = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _2_dsa);
    float3 _7_result = (_6_lum - dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_blend_set_color_saturation)) + _3_blend_set_color_saturation;
    float _8_minComp = min(min(_7_result.x, _7_result.y), _7_result.z);
    float _9_maxComp = max(max(_7_result.x, _7_result.y), _7_result.z);
    if (_8_minComp < 0.0 && _6_lum != _8_minComp) {
        float _10_d = _6_lum - _8_minComp;
        _7_result = _6_lum + (_7_result - _6_lum) * (_6_lum / _10_d);
    }
    if (_9_maxComp > _0_alpha && _9_maxComp != _6_lum) {
        float3 _11_n = (_7_result - _6_lum) * (_0_alpha - _6_lum);
        float _12_d = _9_maxComp - _6_lum;
        _5_blend_set_color_luminance = _6_lum + _11_n / _12_d;
    } else {
        _5_blend_set_color_luminance = _7_result;
    }
    _out.sk_FragColor = float4((((_5_blend_set_color_luminance + _uniforms.dst.xyz) - _2_dsa) + _uniforms.src.xyz) - _1_sda, (_uniforms.src.w + _uniforms.dst.w) - _0_alpha);
    return _out;
}
