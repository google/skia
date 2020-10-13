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
    float _4_blend_color_luminance;
    {
        _4_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    }
    float lum = _4_blend_color_luminance;

    float _5_blend_color_luminance;
    {
        _5_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);
    }
    float3 result = (lum - _5_blend_color_luminance) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + ((result - lum) * lum) / (lum - minComp);
    }
    return maxComp > alpha && maxComp != lum ? lum + ((result - lum) * (alpha - lum)) / (maxComp - lum) : result;
}
float4 blend_luminosity(float4 src, float4 dst) {
    float alpha = dst.w * src.w;
    float3 sda = src.xyz * dst.w;
    float3 dsa = dst.xyz * src.w;
    return float4((((_blend_set_color_luminance(dsa, alpha, sda) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_luminosity;
    {
        float _1_alpha = _in.dst.w * _in.src.w;
        float3 _2_sda = _in.src.xyz * _in.dst.w;
        float3 _3_dsa = _in.dst.xyz * _in.src.w;
        _0_blend_luminosity = float4((((_blend_set_color_luminance(_3_dsa, _1_alpha, _2_sda) + _in.dst.xyz) - _3_dsa) + _in.src.xyz) - _2_sda, (_in.src.w + _in.dst.w) - _1_alpha);
    }

    _out->sk_FragColor = _0_blend_luminosity;

    return *_out;
}
