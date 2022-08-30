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
half blend_color_luminance_Qhh3(half3 color);
half blend_color_saturation_Qhh3(half3 color);
half guarded_divide_Qhhh(half n, half d);
half3 guarded_divide_Qh3h3h(half3 n, half d);
half3 blend_set_color_luminance_Qh3h3hh3(half3 hueSatColor, half alpha, half3 lumColor);
half3 blend_set_color_saturation_Qh3h3h3(half3 color, half3 satColor);
half4 blend_hslc_h4h4h4h2(half4 src, half4 dst, half2 flipSat);
half4 blend_saturation_h4h4h4(half4 src, half4 dst);
half blend_color_luminance_Qhh3(half3 color) {
    return dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), color);
}
half blend_color_saturation_Qhh3(half3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
half guarded_divide_Qhhh(half n, half d) {
    return n / (d + sk_PrivGuardedDivideEpsilon);
}
half3 guarded_divide_Qh3h3h(half3 n, half d) {
    return n / (d + sk_PrivGuardedDivideEpsilon);
}
half3 blend_set_color_luminance_Qh3h3hh3(half3 hueSatColor, half alpha, half3 lumColor) {
    half lum = blend_color_luminance_Qhh3(lumColor);
    half3 result = (lum - blend_color_luminance_Qhh3(hueSatColor)) + hueSatColor;
    half minComp = min(min(result.x, result.y), result.z);
    half maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0h && lum != minComp) {
        result = lum + (result - lum) * guarded_divide_Qhhh(lum, lum - minComp);
    }
    if (maxComp > alpha && maxComp != lum) {
        result = lum + guarded_divide_Qh3h3h((result - lum) * (alpha - lum), maxComp - lum);
    }
    return result;
}
half3 blend_set_color_saturation_Qh3h3h3(half3 color, half3 satColor) {
    half mn = min(min(color.x, color.y), color.z);
    half mx = max(max(color.x, color.y), color.z);
    return mx > mn ? ((color - mn) * blend_color_saturation_Qhh3(satColor)) / (mx - mn) : half3(0.0h);
}
half4 blend_hslc_h4h4h4h2(half4 src, half4 dst, half2 flipSat) {
    half alpha = dst.w * src.w;
    half3 sda = src.xyz * dst.w;
    half3 dsa = dst.xyz * src.w;
    half3 l = bool(flipSat.x) ? dsa : sda;
    half3 r = bool(flipSat.x) ? sda : dsa;
    if (bool(flipSat.y)) {
        l = blend_set_color_saturation_Qh3h3h3(l, r);
        r = dsa;
    }
    return half4((((blend_set_color_luminance_Qh3h3hh3(l, alpha, r) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
half4 blend_saturation_h4h4h4(half4 src, half4 dst) {
    return blend_hslc_h4h4h4h2(src, dst, half2(1.0h));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_saturation_h4h4h4(_uniforms.src, _uniforms.dst);
    return _out;
}
