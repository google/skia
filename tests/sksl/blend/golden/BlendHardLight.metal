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
    float _1_blend_overlay_component;
    float2 _2_s = src.xw;
    float2 _3_d = dst.xw;
    {
        _1_blend_overlay_component = 2.0 * _3_d.x <= _3_d.y ? (2.0 * _2_s.x) * _3_d.x : _2_s.y * _3_d.y - (2.0 * (_3_d.y - _3_d.x)) * (_2_s.y - _2_s.x);
    }
    float _4_blend_overlay_component;
    float2 _5_s = src.yw;
    float2 _6_d = dst.yw;
    {
        _4_blend_overlay_component = 2.0 * _6_d.x <= _6_d.y ? (2.0 * _5_s.x) * _6_d.x : _5_s.y * _6_d.y - (2.0 * (_6_d.y - _6_d.x)) * (_5_s.y - _5_s.x);
    }
    float _7_blend_overlay_component;
    float2 _8_s = src.zw;
    float2 _9_d = dst.zw;
    {
        _7_blend_overlay_component = 2.0 * _9_d.x <= _9_d.y ? (2.0 * _8_s.x) * _9_d.x : _8_s.y * _9_d.y - (2.0 * (_9_d.y - _9_d.x)) * (_8_s.y - _8_s.x);
    }
    float4 result = float4(_1_blend_overlay_component, _4_blend_overlay_component, _7_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



    result.xyz = result.xyz + dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
float4 blend_hard_light(float4 src, float4 dst) {
    return blend_overlay(dst, src);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_hard_light;
    {
        _0_blend_hard_light = blend_overlay(_in.dst, _in.src);
    }

    _out->sk_FragColor = _0_blend_hard_light;

    return *_out;
}
