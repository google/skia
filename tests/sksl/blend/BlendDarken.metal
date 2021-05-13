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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 _0_result = _uniforms.src + (1.0 - _uniforms.src.w) * _uniforms.dst;
    _0_result.xyz = min(_0_result.xyz, (1.0 - _uniforms.dst.w) * _uniforms.src.xyz + _uniforms.dst.xyz);
    _out.sk_FragColor = _0_result;
    return _out;
}
