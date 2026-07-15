#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
constant const half sk_PrivkGuardedDivideEpsilon = half(false ? 1e-08 : 0.0);
constant const half sk_PrivkHalfEpsilon = 0.000244140625h;
constant const half sk_PrivkMinNormalHalf = 6.10351562e-05h;
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
half3 guarded_divide_Qh3h3h(half3 n, half d);
half4 blend_hslc_h4h2h4h4(half2 flipSat, half4 src, half4 dst);
half blend_color_saturation_Qhh3(half3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
half3 guarded_divide_Qh3h3h(half3 n, half d) {
    return n / (d + sk_PrivkGuardedDivideEpsilon);
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
        half _4_diff = _3_mx - _2_mn;
        l = _4_diff >= sk_PrivkHalfEpsilon ? guarded_divide_Qh3h3h((l - _2_mn) * blend_color_saturation_Qhh3(r), _4_diff) : half3(0.0h);
        r = dsa;
    }
    half _5_lum = dot(half3(0.3h, 0.59h, 0.11h), r);
    half3 _6_result = (_5_lum - dot(half3(0.3h, 0.59h, 0.11h), l)) + l;
    half _7_minComp = min(min(_6_result.x, _6_result.y), _6_result.z);
    half _8_maxComp = max(max(_6_result.x, _6_result.y), _6_result.z);
    if (_7_minComp < 0.0h && _5_lum != _7_minComp) {
        _6_result = _5_lum + (_6_result - _5_lum) * (_5_lum / (((_5_lum - _7_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
    }
    if (_8_maxComp > alpha && _8_maxComp != _5_lum) {
        _6_result = _5_lum + ((_6_result - _5_lum) * (alpha - _5_lum)) / (((_8_maxComp - _5_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
    }
    return half4((((_6_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_hslc_h4h2h4h4(half2(0.0h), _uniforms.src, _uniforms.dst);
    return _out;
}
