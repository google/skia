#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 srcdst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

float _blend_overlay_component(float2 s, float2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
float4 blend_overlay(float4 src, float4 dst) {
    float _0_blend_overlay_component;
    float2 _1_s = src.xw;
    float2 _2_d = dst.xw;
    {
        _0_blend_overlay_component = 2.0 * _2_d.x <= _2_d.y ? (2.0 * _1_s.x) * _2_d.x : _1_s.y * _2_d.y - (2.0 * (_2_d.y - _2_d.x)) * (_1_s.y - _1_s.x);
    }
    float _3_blend_overlay_component;
    float2 _4_s = src.yw;
    float2 _5_d = dst.yw;
    {
        _3_blend_overlay_component = 2.0 * _5_d.x <= _5_d.y ? (2.0 * _4_s.x) * _5_d.x : _4_s.y * _5_d.y - (2.0 * (_5_d.y - _5_d.x)) * (_4_s.y - _4_s.x);
    }
    float _6_blend_overlay_component;
    float2 _7_s = src.zw;
    float2 _8_d = dst.zw;
    {
        _6_blend_overlay_component = 2.0 * _8_d.x <= _8_d.y ? (2.0 * _7_s.x) * _8_d.x : _7_s.y * _8_d.y - (2.0 * (_8_d.y - _8_d.x)) * (_7_s.y - _7_s.x);
    }
    float4 result = float4(_0_blend_overlay_component, _3_blend_overlay_component, _6_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



    result.xyz = result.xyz + dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = blend_overlay(_in.src, _in.dst);
    return *_out;
}
