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
half3 _blend_set_color_luminance_h3h3hh3(half3 hueSatColor, half alpha, half3 lumColor) {
    half lum = dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), lumColor);
    half3 result = (lum - dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), hueSatColor)) + hueSatColor;
    half minComp = min(min(result.x, result.y), result.z);
    half maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0h && lum != minComp) {
        result = lum + (result - lum) * (lum / (lum - minComp));
    }
    if (maxComp > alpha && maxComp != lum) {
        return lum + ((result - lum) * (alpha - lum)) / (maxComp - lum);
    } else {
        return result;
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half _0_alpha = _uniforms.dst.w * _uniforms.src.w;
    half3 _1_sda = _uniforms.src.xyz * _uniforms.dst.w;
    half3 _2_dsa = _uniforms.dst.xyz * _uniforms.src.w;
    _out.sk_FragColor = half4((((_blend_set_color_luminance_h3h3hh3(_1_sda, _0_alpha, _2_dsa) + _uniforms.dst.xyz) - _2_dsa) + _uniforms.src.xyz) - _1_sda, (_uniforms.src.w + _uniforms.dst.w) - _0_alpha);
    return _out;
}
