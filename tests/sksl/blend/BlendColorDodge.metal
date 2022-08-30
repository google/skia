#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
constant const half sk_PrivGuardedDivideEpsilon = half(false ? 9.9999999392252903e-09 : 0.0);
struct Uniforms {
    half4 src;
    half4 dst;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
half guarded_divide_Qhhh(half n, half d);
half color_dodge_component_Qhh2h2(half2 s, half2 d);
half4 blend_color_dodge_h4h4h4(half4 src, half4 dst);
half guarded_divide_Qhhh(half n, half d) {
    return n / (d + sk_PrivGuardedDivideEpsilon);
}
half color_dodge_component_Qhh2h2(half2 s, half2 d) {
    if (d.x == 0.0h) {
        return s.x * (1.0h - d.y);
    } else {
        half delta = s.y - s.x;
        if (delta == 0.0h) {
            return (s.y * d.y + s.x * (1.0h - d.y)) + d.x * (1.0h - s.y);
        } else {
            delta = min(d.y, guarded_divide_Qhhh(d.x * s.y, delta));
            return (delta * s.y + s.x * (1.0h - d.y)) + d.x * (1.0h - s.y);
        }
    }
}
half4 blend_color_dodge_h4h4h4(half4 src, half4 dst) {
    return half4(color_dodge_component_Qhh2h2(src.xw, dst.xw), color_dodge_component_Qhh2h2(src.yw, dst.yw), color_dodge_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0h - src.w) * dst.w);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_color_dodge_h4h4h4(_uniforms.src, _uniforms.dst);
    return _out;
}
