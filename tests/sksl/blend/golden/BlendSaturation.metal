#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 src;
    float4 dst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


float _blend_color_luminance(float3 color) {
    return dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), color);
}
float3 _blend_set_color_luminance(float3 hueSatColor, float alpha, float3 lumColor) {
    float lum = _blend_color_luminance(lumColor);
    float3 result = (lum - _blend_color_luminance(hueSatColor)) + hueSatColor;
    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + ((result - lum) * lum) / (lum - minComp);
    }
    return maxComp > alpha && maxComp != lum ? lum + ((result - lum) * (alpha - lum)) / (maxComp - lum) : result;
}
float _blend_color_saturation(float3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
float3 _blend_set_color_saturation_helper(float3 minMidMax, float sat) {
    return minMidMax.x < minMidMax.z ? float3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat) : float3(0.0);
}
float3 _blend_set_color_saturation(float3 hueLumColor, float3 satColor) {
    float sat = _blend_color_saturation(satColor);
    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            hueLumColor.xyz = _blend_set_color_saturation_helper(hueLumColor, sat);
        } else if (hueLumColor.x <= hueLumColor.z) {
            hueLumColor.xzy = _blend_set_color_saturation_helper(hueLumColor.xzy, sat);
        } else {
            hueLumColor.zxy = _blend_set_color_saturation_helper(hueLumColor.zxy, sat);
        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        hueLumColor.yxz = _blend_set_color_saturation_helper(hueLumColor.yxz, sat);
    } else if (hueLumColor.y <= hueLumColor.z) {
        hueLumColor.yzx = _blend_set_color_saturation_helper(hueLumColor.yzx, sat);
    } else {
        hueLumColor.zyx = _blend_set_color_saturation_helper(hueLumColor.zyx, sat);
    }
    return hueLumColor;
}
float4 blend_saturation(float4 src, float4 dst) {
    float alpha = dst.w * src.w;
    float3 sda = src.xyz * dst.w;
    float3 dsa = dst.xyz * src.w;
    return float4((((_blend_set_color_luminance(_blend_set_color_saturation(dsa, sda), alpha, dsa) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_saturation;
    {
        float _1_alpha = _in.dst.w * _in.src.w;
        float3 _2_sda = _in.src.xyz * _in.dst.w;
        float3 _3_dsa = _in.dst.xyz * _in.src.w;
        _0_blend_saturation = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_3_dsa, _2_sda), _1_alpha, _3_dsa) + _in.dst.xyz) - _3_dsa) + _in.src.xyz) - _2_sda, (_in.src.w + _in.dst.w) - _1_alpha);
    }

    _out->sk_FragColor = _0_blend_saturation;

    return *_out;
}
