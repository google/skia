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
float3 _blend_set_color_luminance_h3h3hh3(float3 hueSatColor, float alpha, float3 lumColor) {
    float lum = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    float3 result = (lum - dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor)) + hueSatColor;
    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + (result - lum) * (lum / (lum - minComp));
    }
    if (maxComp > alpha && maxComp != lum) {
        return lum + ((result - lum) * (alpha - lum)) / (maxComp - lum);
    } else {
        return result;
    }
}
float3 _blend_set_color_saturation_helper_h3h3h(float3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        return float3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat);
    } else {
        return float3(0.0);
    }
}
float3 _blend_set_color_saturation_h3h3h3(float3 hueLumColor, float3 satColor) {
    float sat = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
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
    float _0_alpha = _uniforms.dst.w * _uniforms.src.w;
    float3 _1_sda = _uniforms.src.xyz * _uniforms.dst.w;
    float3 _2_dsa = _uniforms.dst.xyz * _uniforms.src.w;
    _out.sk_FragColor = float4((((_blend_set_color_luminance_h3h3hh3(_blend_set_color_saturation_h3h3h3(_2_dsa, _1_sda), _0_alpha, _2_dsa) + _uniforms.dst.xyz) - _2_dsa) + _uniforms.src.xyz) - _1_sda, (_uniforms.src.w + _uniforms.dst.w) - _0_alpha);
    return _out;
}
