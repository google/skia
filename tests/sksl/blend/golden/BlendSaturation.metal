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
    return minMidMax.x < minMidMax.z ? float3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat) : float3(0.0);
}


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float _1_alpha = _in.dst.w * _in.src.w;
    float3 _2_sda = _in.src.xyz * _in.dst.w;
    float3 _3_dsa = _in.dst.xyz * _in.src.w;
    float3 _4_blend_set_color_saturation;
    float _5_sat = max(max(_2_sda.x, _2_sda.y), _2_sda.z) - min(min(_2_sda.x, _2_sda.y), _2_sda.z);

    if (_3_dsa.x <= _3_dsa.y) {
        if (_3_dsa.y <= _3_dsa.z) {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa, _5_sat);
        } else if (_3_dsa.x <= _3_dsa.z) {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa.xzy, _5_sat).xzy;
        } else {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa.zxy, _5_sat).yzx;
        }
    } else if (_3_dsa.x <= _3_dsa.z) {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa.yxz, _5_sat).yxz;
    } else if (_3_dsa.y <= _3_dsa.z) {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa.yzx, _5_sat).zxy;
    } else {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_3_dsa.zyx, _5_sat).zyx;
    }
    float _7_lum = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);

    float3 _8_result = (_7_lum - dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _4_blend_set_color_saturation)) + _4_blend_set_color_saturation;

    float _9_minComp = min(min(_8_result.x, _8_result.y), _8_result.z);
    float _10_maxComp = max(max(_8_result.x, _8_result.y), _8_result.z);
    if (_9_minComp < 0.0 && _7_lum != _9_minComp) {
        _8_result = _7_lum + ((_8_result - _7_lum) * _7_lum) / (_7_lum - _9_minComp);
    }
    _out->sk_FragColor = float4(((((_10_maxComp > _1_alpha && _10_maxComp != _7_lum ? _7_lum + ((_8_result - _7_lum) * (_1_alpha - _7_lum)) / (_10_maxComp - _7_lum) : _8_result) + _in.dst.xyz) - _3_dsa) + _in.src.xyz) - _2_sda, (_in.src.w + _in.dst.w) - _1_alpha);



    return *_out;
}
