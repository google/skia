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


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _7_blend_color;
    {
        float _8_alpha = _in.dst.w * _in.src.w;
        float3 _9_sda = _in.src.xyz * _in.dst.w;
        float3 _10_dsa = _in.dst.xyz * _in.src.w;
        float3 _11_blend_set_color_luminance;
        {
            float _16_blend_color_luminance;
            {
                _16_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _10_dsa);
            }
            float _12_lum = _16_blend_color_luminance;

            float _17_blend_color_luminance;
            {
                _17_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _9_sda);
            }
            float3 _13_result = (_12_lum - _17_blend_color_luminance) + _9_sda;

            float _14_minComp = min(min(_13_result.x, _13_result.y), _13_result.z);
            float _15_maxComp = max(max(_13_result.x, _13_result.y), _13_result.z);
            if (_14_minComp < 0.0 && _12_lum != _14_minComp) {
                _13_result = _12_lum + ((_13_result - _12_lum) * _12_lum) / (_12_lum - _14_minComp);
            }
            _11_blend_set_color_luminance = _15_maxComp > _8_alpha && _15_maxComp != _12_lum ? _12_lum + ((_13_result - _12_lum) * (_8_alpha - _12_lum)) / (_15_maxComp - _12_lum) : _13_result;
        }
        _7_blend_color = float4((((_11_blend_set_color_luminance + _in.dst.xyz) - _10_dsa) + _in.src.xyz) - _9_sda, (_in.src.w + _in.dst.w) - _8_alpha);

    }
    _out->sk_FragColor = _7_blend_color;

    return *_out;
}
