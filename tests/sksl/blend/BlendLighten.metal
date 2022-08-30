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
half4 blend_lighten_h4h4h4(half4 src, half4 dst);
half4 blend_src_over_h4h4h4(half4 src, half4 dst) {
    return src + (1.0h - src.w) * dst;
}
half4 blend_lighten_h4h4h4(half4 src, half4 dst) {
    half4 result = blend_src_over_h4h4h4(src, dst);
    result.xyz = max(result.xyz, (1.0h - dst.w) * src.xyz + dst.xyz);
    return result;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_lighten_h4h4h4(_uniforms.src, _uniforms.dst);
    return _out;
}
