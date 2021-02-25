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
    Outputs _out;
    (void)_out;
    float4 _0_blend_darken;
    float4 _1_blend_src_over;
    float4 _2_result = _in.src + (1.0 - _in.src.w) * _in.dst;

    _2_result.xyz = min(_2_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
    _out.sk_FragColor = _2_result;

    return _out;
}
