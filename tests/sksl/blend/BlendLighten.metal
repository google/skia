#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 src;
    half4 dst;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 _0_result = _uniforms.src + (1.0h - _uniforms.src.w) * _uniforms.dst;
    _0_result.xyz = max(_0_result.xyz, (1.0h - _uniforms.dst.w) * _uniforms.src.xyz + _uniforms.dst.xyz);
    _out.sk_FragColor = _0_result;
    return _out;
}
