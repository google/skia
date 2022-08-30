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
half soft_light_component_Qhh2h2(half2 s, half2 d);
half4 blend_soft_light_h4h4h4(half4 src, half4 dst);
half guarded_divide_Qhhh(half n, half d) {
    return n / (d + sk_PrivGuardedDivideEpsilon);
}
half soft_light_component_Qhh2h2(half2 s, half2 d) {
    if (2.0h * s.x <= s.y) {
        return (guarded_divide_Qhhh((d.x * d.x) * (s.y - 2.0h * s.x), d.y) + (1.0h - d.y) * s.x) + d.x * ((-s.y + 2.0h * s.x) + 1.0h);
    } else if (4.0h * d.x <= d.y) {
        half DSqd = d.x * d.x;
        half DCub = DSqd * d.x;
        half DaSqd = d.y * d.y;
        half DaCub = DaSqd * d.y;
        return guarded_divide_Qhhh(((DaSqd * (s.x - d.x * ((3.0h * s.y - 6.0h * s.x) - 1.0h)) + ((12.0h * d.y) * DSqd) * (s.y - 2.0h * s.x)) - (16.0h * DCub) * (s.y - 2.0h * s.x)) - DaCub * s.x, DaSqd);
    } else {
        return ((d.x * ((s.y - 2.0h * s.x) + 1.0h) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0h * s.x)) - d.y * s.x;
    }
}
half4 blend_soft_light_h4h4h4(half4 src, half4 dst) {
    return dst.w == 0.0h ? src : half4(soft_light_component_Qhh2h2(src.xw, dst.xw), soft_light_component_Qhh2h2(src.yw, dst.yw), soft_light_component_Qhh2h2(src.zw, dst.zw), src.w + (1.0h - src.w) * dst.w);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_soft_light_h4h4h4(_uniforms.src, _uniforms.dst);
    return _out;
}
