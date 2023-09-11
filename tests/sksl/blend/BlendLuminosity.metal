#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const half sk_PrivkGuardedDivideEpsilon = half(false ? 1e-08 : 0.0);
struct Uniforms {
    half4 src;
    half4 dst;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
half blend_color_saturation_Qhh3(half3 color);
half4 blend_hslc_h4h2h4h4(half2 flipSat, half4 src, half4 dst);
half blend_color_saturation_Qhh3(half3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
half4 blend_hslc_h4h2h4h4(half2 flipSat, half4 src, half4 dst) {
    half alpha = dst.w * src.w;
    half3 sda = src.xyz * dst.w;
    half3 dsa = dst.xyz * src.w;
    half3 l = bool(flipSat.x) ? dsa : sda;
    half3 r = bool(flipSat.x) ? sda : dsa;
    if (bool(flipSat.y)) {
        half _2_mn = min(min(l.x, l.y), l.z);
        half _3_mx = max(max(l.x, l.y), l.z);
        l = _3_mx > _2_mn ? ((l - _2_mn) * blend_color_saturation_Qhh3(r)) / (_3_mx - _2_mn) : half3(0.0h);
        r = dsa;
    }
    half _4_lum = dot(half3(0.3h, 0.59h, 0.11h), r);
    half3 _5_result = (_4_lum - dot(half3(0.3h, 0.59h, 0.11h), l)) + l;
    half _6_minComp = min(min(_5_result.x, _5_result.y), _5_result.z);
    half _7_maxComp = max(max(_5_result.x, _5_result.y), _5_result.z);
    if (_6_minComp < 0.0h && _4_lum != _6_minComp) {
        _5_result = _4_lum + (_5_result - _4_lum) * (_4_lum / ((_4_lum - _6_minComp) + sk_PrivkGuardedDivideEpsilon));
    }
    if (_7_maxComp > alpha && _7_maxComp != _4_lum) {
        _5_result = _4_lum + ((_5_result - _4_lum) * (alpha - _4_lum)) / ((_7_maxComp - _4_lum) + sk_PrivkGuardedDivideEpsilon);
    }
    return half4((((_5_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_hslc_h4h2h4h4(half2(1.0h, 0.0h), _uniforms.src, _uniforms.dst);
    return _out;
}
