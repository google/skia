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
float _guarded_divide(float n, float d) {
    return n / d;
}
float3 _guarded_divide(float3 n, float d) {
    return n / d;
}
float3 _blend_set_color_luminance(float3 hueSatColor, float alpha, float3 lumColor) {
    float lum = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    float3 result = (lum - dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor)) + hueSatColor;
    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + (result - lum) * _guarded_divide(lum, lum - minComp);
    }
    if (maxComp > alpha && maxComp != lum) {
        return lum + _guarded_divide((result - lum) * (alpha - lum), maxComp - lum);
    } else {
        return result;
    }
}
float3 _blend_set_color_saturation_helper(float3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        return float3(0.0, _guarded_divide(sat * (minMidMax.y - minMidMax.x), minMidMax.z - minMidMax.x), sat);
    } else {
        return float3(0.0);
    }
}
float3 _blend_set_color_saturation(float3 hueLumColor, float3 satColor) {
    float sat = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            return _blend_set_color_saturation_helper(hueLumColor, sat);
        } else if (hueLumColor.x <= hueLumColor.z) {
            return _blend_set_color_saturation_helper(hueLumColor.xzy, sat).xzy;
        } else {
            return _blend_set_color_saturation_helper(hueLumColor.zxy, sat).yzx;
        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        return _blend_set_color_saturation_helper(hueLumColor.yxz, sat).yxz;
    } else if (hueLumColor.y <= hueLumColor.z) {
        return _blend_set_color_saturation_helper(hueLumColor.yzx, sat).zxy;
    } else {
        return _blend_set_color_saturation_helper(hueLumColor.zyx, sat).zyx;
    }
}
float4 blend_saturation(float4 src, float4 dst) {
    float alpha = dst.w * src.w;
    float3 sda = src.xyz * dst.w;
    float3 dsa = dst.xyz * src.w;
    return float4((((_blend_set_color_luminance(_blend_set_color_saturation(dsa, sda), alpha, dsa) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_saturation(_in.src, _in.dst);
    return _out;
}
