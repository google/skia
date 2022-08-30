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
half4 blend_difference_h4h4h4(half4 src, half4 dst);
half4 blend_difference_h4h4h4(half4 src, half4 dst) {
    return half4((src.xyz + dst.xyz) - 2.0h * min(src.xyz * dst.w, dst.xyz * src.w), src.w + (1.0h - src.w) * dst.w);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_difference_h4h4h4(_uniforms.src, _uniforms.dst);
    return _out;
}
