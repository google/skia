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
half4 blend_src_over_h4h4h4(half4 src, half4 dst);
half4 blend_darken_h4h4h4h(half4 src, half4 dst, half mode);
half4 blend_darken_h4h4h4(half4 src, half4 dst);
half4 blend_src_over_h4h4h4(half4 src, half4 dst) {
    return src + (1.0h - src.w) * dst;
}
half4 blend_darken_h4h4h4h(half4 src, half4 dst, half mode) {
    half4 a = blend_src_over_h4h4h4(src, dst);
    half3 b = (1.0h - dst.w) * src.xyz + dst.xyz;
    a.xyz = mode * min(a.xyz * mode, b * mode);
    return a;
}
half4 blend_darken_h4h4h4(half4 src, half4 dst) {
    return blend_darken_h4h4h4h(src, dst, 1.0h);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_darken_h4h4h4(_uniforms.src, _uniforms.dst);
    return _out;
}
