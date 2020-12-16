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
float _blend_overlay_component(float2 s, float2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_hard_light;
    float4 _1_blend_overlay;
    float4 _2_result = float4(_blend_overlay_component(_in.dst.xw, _in.src.xw), _blend_overlay_component(_in.dst.yw, _in.src.yw), _blend_overlay_component(_in.dst.zw, _in.src.zw), _in.dst.w + (1.0 - _in.dst.w) * _in.src.w);
    _2_result.xyz = _2_result.xyz + _in.src.xyz * (1.0 - _in.dst.w) + _in.dst.xyz * (1.0 - _in.src.w);
    _1_blend_overlay = _2_result;

    _0_blend_hard_light = _1_blend_overlay;


    _out->sk_FragColor = _0_blend_hard_light;

    return *_out;
}
