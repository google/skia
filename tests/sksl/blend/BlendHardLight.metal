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
half blend_overlay_component_Qhh2h2(half2 s, half2 d);
half4 blend_overlay_h4h4h4(half4 src, half4 dst);
half blend_overlay_component_Qhh2h2(half2 s, half2 d) {
    return 2.0h * d.x <= d.y ? (2.0h * s.x) * d.x : s.y * d.y - (2.0h * (d.y - d.x)) * (s.y - s.x);
}
half4 blend_overlay_h4h4h4(half4 src, half4 dst) {
    half4 result = half4(blend_overlay_component_Qhh2h2(src.xw, dst.xw), blend_overlay_component_Qhh2h2(src.yw, dst.yw), blend_overlay_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0h - src.w) * dst.w);
    result.xyz = result.xyz + (dst.xyz * (1.0h - src.w) + src.xyz * (1.0h - dst.w));
    return result;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_overlay_h4h4h4(_uniforms.dst, _uniforms.src);
    return _out;
}
