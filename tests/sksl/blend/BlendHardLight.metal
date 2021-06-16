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
float _blend_overlay_component_hh2h2(float2 s, float2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 _0_result = float4(_blend_overlay_component_hh2h2(_uniforms.dst.xw, _uniforms.src.xw), _blend_overlay_component_hh2h2(_uniforms.dst.yw, _uniforms.src.yw), _blend_overlay_component_hh2h2(_uniforms.dst.zw, _uniforms.src.zw), _uniforms.dst.w + (1.0 - _uniforms.dst.w) * _uniforms.src.w);
    _0_result.xyz = _0_result.xyz + _uniforms.src.xyz * (1.0 - _uniforms.dst.w) + _uniforms.dst.xyz * (1.0 - _uniforms.src.w);
    _out.sk_FragColor = _0_result;
    return _out;
}
