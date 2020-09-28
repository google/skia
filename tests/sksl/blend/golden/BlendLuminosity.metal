#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 srcdst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_luminosity;
    {
        float _1_alpha = _in.dst.w * _in.src.w;
        float3 _2_sda = _in.src.xyz * _in.dst.w;
        float3 _3_dsa = _in.dst.xyz * _in.src.w;
        float3 _6_blend_set_color_luminance;
        {
            float _11_blend_color_luminance;
            {
                _11_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _2_sda);
            }
            float _7_lum = _11_blend_color_luminance;

            float _12_blend_color_luminance;
            {
                _12_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);
            }
            float3 _8_result = (_7_lum - _12_blend_color_luminance) + _3_dsa;

            float _9_minComp = min(min(_8_result.x, _8_result.y), _8_result.z);
            float _10_maxComp = max(max(_8_result.x, _8_result.y), _8_result.z);
            if (_9_minComp < 0.0 && _7_lum != _9_minComp) {
                _8_result = _7_lum + ((_8_result - _7_lum) * _7_lum) / (_7_lum - _9_minComp);
            }
            _6_blend_set_color_luminance = _10_maxComp > _1_alpha && _10_maxComp != _7_lum ? _7_lum + ((_8_result - _7_lum) * (_1_alpha - _7_lum)) / (_10_maxComp - _7_lum) : _8_result;
        }
        _0_blend_luminosity = float4((((_6_blend_set_color_luminance + _in.dst.xyz) - _3_dsa) + _in.src.xyz) - _2_sda, (_in.src.w + _in.dst.w) - _1_alpha);

    }

    _out->sk_FragColor = _0_blend_luminosity;

    return *_out;
}
