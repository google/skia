#include <metal_stdlib>
#include <simd/simd.h>
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
    _out.sk_FragColor = half4((_uniforms.src.xyz + _uniforms.dst.xyz) - 2.0h * min(_uniforms.src.xyz * _uniforms.dst.w, _uniforms.dst.xyz * _uniforms.src.w), _uniforms.src.w + (1.0h - _uniforms.src.w) * _uniforms.dst.w);
    return _out;
}
