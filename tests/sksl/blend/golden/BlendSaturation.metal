#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 srcdst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

float _blend_color_luminance(float3 color) {
    return dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), color);
}
float3 _blend_set_color_luminance(float3 hueSatColor, float alpha, float3 lumColor) {
    float _0_blend_color_luminance;
    {
        _0_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    }
    float lum = _0_blend_color_luminance;

    float _1_blend_color_luminance;
    {
        _1_blend_color_luminance = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);
    }
    float3 result = (lum - _1_blend_color_luminance) + hueSatColor;

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
    float _2_blend_color_saturation;
    {
        _2_blend_color_saturation = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    }
    float sat = _2_blend_color_saturation;

    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            float3 _3_blend_set_color_saturation_helper;
            {
                _3_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.z ? float3(0.0, (sat * (hueLumColor.y - hueLumColor.x)) / (hueLumColor.z - hueLumColor.x), sat) : float3(0.0);
            }
            hueLumColor.xyz = _3_blend_set_color_saturation_helper;

        } else if (hueLumColor.x <= hueLumColor.z) {
            float3 _4_blend_set_color_saturation_helper;
            float3 _5_minMidMax = hueLumColor.xzy;
            {
                _4_blend_set_color_saturation_helper = _5_minMidMax.x < _5_minMidMax.z ? float3(0.0, (sat * (_5_minMidMax.y - _5_minMidMax.x)) / (_5_minMidMax.z - _5_minMidMax.x), sat) : float3(0.0);
            }
            hueLumColor.xzy = _4_blend_set_color_saturation_helper;

        } else {
            float3 _6_blend_set_color_saturation_helper;
            float3 _7_minMidMax = hueLumColor.zxy;
            {
                _6_blend_set_color_saturation_helper = _7_minMidMax.x < _7_minMidMax.z ? float3(0.0, (sat * (_7_minMidMax.y - _7_minMidMax.x)) / (_7_minMidMax.z - _7_minMidMax.x), sat) : float3(0.0);
            }
            hueLumColor.zxy = _6_blend_set_color_saturation_helper;

        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        float3 _8_blend_set_color_saturation_helper;
        float3 _9_minMidMax = hueLumColor.yxz;
        {
            _8_blend_set_color_saturation_helper = _9_minMidMax.x < _9_minMidMax.z ? float3(0.0, (sat * (_9_minMidMax.y - _9_minMidMax.x)) / (_9_minMidMax.z - _9_minMidMax.x), sat) : float3(0.0);
        }
        hueLumColor.yxz = _8_blend_set_color_saturation_helper;

    } else if (hueLumColor.y <= hueLumColor.z) {
        float3 _10_blend_set_color_saturation_helper;
        float3 _11_minMidMax = hueLumColor.yzx;
        {
            _10_blend_set_color_saturation_helper = _11_minMidMax.x < _11_minMidMax.z ? float3(0.0, (sat * (_11_minMidMax.y - _11_minMidMax.x)) / (_11_minMidMax.z - _11_minMidMax.x), sat) : float3(0.0);
        }
        hueLumColor.yzx = _10_blend_set_color_saturation_helper;

    } else {
        float3 _12_blend_set_color_saturation_helper;
        float3 _13_minMidMax = hueLumColor.zyx;
        {
            _12_blend_set_color_saturation_helper = _13_minMidMax.x < _13_minMidMax.z ? float3(0.0, (sat * (_13_minMidMax.y - _13_minMidMax.x)) / (_13_minMidMax.z - _13_minMidMax.x), sat) : float3(0.0);
        }
        hueLumColor.zyx = _12_blend_set_color_saturation_helper;

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
    _out->sk_FragColor = blend_saturation(_in.src, _in.dst);
    return *_out;
}
