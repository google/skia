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
    float4 _1_result = float4(_blend_overlay_component(_in.src.xw, _in.dst.xw), _blend_overlay_component(_in.src.yw, _in.dst.yw), _blend_overlay_component(_in.src.zw, _in.dst.zw), _in.src.w + (1.0 - _in.src.w) * _in.dst.w);
    _1_result.xyz = _1_result.xyz + _in.dst.xyz * (1.0 - _in.src.w) + _in.src.xyz * (1.0 - _in.dst.w);
    _out->sk_FragColor = _1_result;

    return *_out;
}
