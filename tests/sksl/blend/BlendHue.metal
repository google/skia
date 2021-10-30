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
half3 _blend_set_color_saturation_helper_h3h3h(half3 minMidMax, half sat) {
    if (minMidMax.x < minMidMax.z) {
        return half3(0.0h, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat);
    } else {
        return half3(0.0h);
    }
}
half3 _blend_set_color_saturation_h3h3h3(half3 hueLumColor, half3 satColor) {
    half sat = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            return _blend_set_color_saturation_helper_h3h3h(hueLumColor, sat);
        } else if (hueLumColor.x <= hueLumColor.z) {
            return _blend_set_color_saturation_helper_h3h3h(hueLumColor.xzy, sat).xzy;
        } else {
            return _blend_set_color_saturation_helper_h3h3h(hueLumColor.zxy, sat).yzx;
        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        return _blend_set_color_saturation_helper_h3h3h(hueLumColor.yxz, sat).yxz;
    } else if (hueLumColor.y <= hueLumColor.z) {
        return _blend_set_color_saturation_helper_h3h3h(hueLumColor.yzx, sat).zxy;
    } else {
        return _blend_set_color_saturation_helper_h3h3h(hueLumColor.zyx, sat).zyx;
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half _0_alpha = _uniforms.dst.w * _uniforms.src.w;
    half3 _1_sda = _uniforms.src.xyz * _uniforms.dst.w;
    half3 _2_dsa = _uniforms.dst.xyz * _uniforms.src.w;
    _out.sk_FragColor = half4((((_blend_set_color_luminance_h3h3hh3(_blend_set_color_saturation_h3h3h3(_1_sda, _2_dsa), _0_alpha, _2_dsa) + _uniforms.dst.xyz) - _2_dsa) + _uniforms.src.xyz) - _1_sda, (_uniforms.src.w + _uniforms.dst.w) - _0_alpha);
    return _out;
}
