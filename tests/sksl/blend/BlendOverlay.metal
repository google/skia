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
half blend_overlay_component_Qhh2h2(half2 s, half2 d) {
    return 2.0h * d.x <= d.y ? (2.0h * s.x) * d.x : s.y * d.y - (2.0h * (d.y - d.x)) * (s.y - s.x);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 _0_result = half4(blend_overlay_component_Qhh2h2(_uniforms.src.xw, _uniforms.dst.xw), blend_overlay_component_Qhh2h2(_uniforms.src.yw, _uniforms.dst.yw), blend_overlay_component_Qhh2h2(_uniforms.src.zw, _uniforms.dst.zw), _uniforms.src.w + (1.0h - _uniforms.src.w) * _uniforms.dst.w);
    _0_result.xyz = _0_result.xyz + (_uniforms.dst.xyz * (1.0h - _uniforms.src.w) + _uniforms.src.xyz * (1.0h - _uniforms.dst.w));
    _out.sk_FragColor = _0_result;
    return _out;
}
