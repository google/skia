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
    float _1_alpha = _in.dst.w * _in.src.w;
    float3 _2_sda = _in.src.xyz * _in.dst.w;
    float3 _3_dsa = _in.dst.xyz * _in.src.w;
    float _5_lum = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _2_sda);

    float3 _6_result = (_5_lum - dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa)) + _3_dsa;

    float _7_minComp = min(min(_6_result.x, _6_result.y), _6_result.z);
    float _8_maxComp = max(max(_6_result.x, _6_result.y), _6_result.z);
    if (_7_minComp < 0.0 && _5_lum != _7_minComp) {
        _6_result = _5_lum + ((_6_result - _5_lum) * _5_lum) / (_5_lum - _7_minComp);
    }
    _out->sk_FragColor = float4(((((_8_maxComp > _1_alpha && _8_maxComp != _5_lum ? _5_lum + ((_6_result - _5_lum) * (_1_alpha - _5_lum)) / (_8_maxComp - _5_lum) : _6_result) + _in.dst.xyz) - _3_dsa) + _in.src.xyz) - _2_sda, (_in.src.w + _in.dst.w) - _1_alpha);


    return *_out;
}
