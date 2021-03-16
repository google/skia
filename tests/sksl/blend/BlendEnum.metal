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
float _blend_overlay_component(float2 s, float2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
float4 blend_overlay(float4 src, float4 dst) {
    float4 result = float4(_blend_overlay_component(src.xw, dst.xw), _blend_overlay_component(src.yw, dst.yw), _blend_overlay_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    result.xyz = result.xyz + dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
float _color_dodge_component(float2 s, float2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            float _0_n = d.x * s.y;
            delta = min(d.y, _0_n / delta);
            return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        }
    }
}
float _color_burn_component(float2 s, float2 d) {
    if (d.y == d.x) {
        return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    } else if (s.x == 0.0) {
        return d.x * (1.0 - s.y);
    } else {
        float _1_n = (d.y - d.x) * s.y;
        float delta = max(0.0, d.y - _1_n / s.x);
        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component(float2 s, float2 d) {
    if (2.0 * s.x <= s.y) {
        float _2_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        return (_2_n / d.y + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);
    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _3_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        return _3_n / DaSqd;
    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
float3 _blend_set_color_luminance(float3 hueSatColor, float alpha, float3 lumColor) {
    float lum = dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    float3 result = (lum - dot(float3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor)) + hueSatColor;
    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        float _4_d = lum - minComp;
        result = lum + (result - lum) * (lum / _4_d);
    }
    if (maxComp > alpha && maxComp != lum) {
        float3 _5_n = (result - lum) * (alpha - lum);
        float _6_d = maxComp - lum;
        return lum + _5_n / _6_d;
    } else {
        return result;
    }
}
float3 _blend_set_color_saturation_helper(float3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        float _7_n = sat * (minMidMax.y - minMidMax.x);
        float _8_d = minMidMax.z - minMidMax.x;
        return float3(0.0, _7_n / _8_d, sat);
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

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 _0_blend;
    {
        _0_blend = _uniforms.src * _uniforms.dst;
        _0_blend = _uniforms.src + (1.0 - _uniforms.src) * _uniforms.dst;
        _0_blend = blend_overlay(_uniforms.src, _uniforms.dst);
        float4 _1_result = _uniforms.src + (1.0 - _uniforms.src.w) * _uniforms.dst;
        _1_result.xyz = min(_1_result.xyz, (1.0 - _uniforms.dst.w) * _uniforms.src.xyz + _uniforms.dst.xyz);
        _0_blend = _1_result;
        float4 _2_result = _uniforms.src + (1.0 - _uniforms.src.w) * _uniforms.dst;
        _2_result.xyz = max(_2_result.xyz, (1.0 - _uniforms.dst.w) * _uniforms.src.xyz + _uniforms.dst.xyz);
        _0_blend = _2_result;
        _0_blend = float4(_color_dodge_component(_uniforms.src.xw, _uniforms.dst.xw), _color_dodge_component(_uniforms.src.yw, _uniforms.dst.yw), _color_dodge_component(_uniforms.src.zw, _uniforms.dst.zw), _uniforms.src.w + (1.0 - _uniforms.src.w) * _uniforms.dst.w);
        _0_blend = float4(_color_burn_component(_uniforms.src.xw, _uniforms.dst.xw), _color_burn_component(_uniforms.src.yw, _uniforms.dst.yw), _color_burn_component(_uniforms.src.zw, _uniforms.dst.zw), _uniforms.src.w + (1.0 - _uniforms.src.w) * _uniforms.dst.w);
        _0_blend = blend_overlay(_uniforms.dst, _uniforms.src);
        _0_blend = _uniforms.dst.w == 0.0 ? _uniforms.src : float4(_soft_light_component(_uniforms.src.xw, _uniforms.dst.xw), _soft_light_component(_uniforms.src.yw, _uniforms.dst.yw), _soft_light_component(_uniforms.src.zw, _uniforms.dst.zw), _uniforms.src.w + (1.0 - _uniforms.src.w) * _uniforms.dst.w);
        _0_blend = float4((_uniforms.src.xyz + _uniforms.dst.xyz) - 2.0 * min(_uniforms.src.xyz * _uniforms.dst.w, _uniforms.dst.xyz * _uniforms.src.w), _uniforms.src.w + (1.0 - _uniforms.src.w) * _uniforms.dst.w);
        _0_blend = float4((_uniforms.dst.xyz + _uniforms.src.xyz) - (2.0 * _uniforms.dst.xyz) * _uniforms.src.xyz, _uniforms.src.w + (1.0 - _uniforms.src.w) * _uniforms.dst.w);
        _0_blend = float4(((1.0 - _uniforms.src.w) * _uniforms.dst.xyz + (1.0 - _uniforms.dst.w) * _uniforms.src.xyz) + _uniforms.src.xyz * _uniforms.dst.xyz, _uniforms.src.w + (1.0 - _uniforms.src.w) * _uniforms.dst.w);
        float _3_alpha = _uniforms.dst.w * _uniforms.src.w;
        float3 _4_sda = _uniforms.src.xyz * _uniforms.dst.w;
        float3 _5_dsa = _uniforms.dst.xyz * _uniforms.src.w;
        _0_blend = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_4_sda, _5_dsa), _3_alpha, _5_dsa) + _uniforms.dst.xyz) - _5_dsa) + _uniforms.src.xyz) - _4_sda, (_uniforms.src.w + _uniforms.dst.w) - _3_alpha);
        float _6_alpha = _uniforms.dst.w * _uniforms.src.w;
        float3 _7_sda = _uniforms.src.xyz * _uniforms.dst.w;
        float3 _8_dsa = _uniforms.dst.xyz * _uniforms.src.w;
        _0_blend = float4((((_blend_set_color_luminance(_blend_set_color_saturation(_8_dsa, _7_sda), _6_alpha, _8_dsa) + _uniforms.dst.xyz) - _8_dsa) + _uniforms.src.xyz) - _7_sda, (_uniforms.src.w + _uniforms.dst.w) - _6_alpha);
        float _9_alpha = _uniforms.dst.w * _uniforms.src.w;
        float3 _10_sda = _uniforms.src.xyz * _uniforms.dst.w;
        float3 _11_dsa = _uniforms.dst.xyz * _uniforms.src.w;
        _0_blend = float4((((_blend_set_color_luminance(_10_sda, _9_alpha, _11_dsa) + _uniforms.dst.xyz) - _11_dsa) + _uniforms.src.xyz) - _10_sda, (_uniforms.src.w + _uniforms.dst.w) - _9_alpha);
        float _12_alpha = _uniforms.dst.w * _uniforms.src.w;
        float3 _13_sda = _uniforms.src.xyz * _uniforms.dst.w;
        float3 _14_dsa = _uniforms.dst.xyz * _uniforms.src.w;
        _0_blend = float4((((_blend_set_color_luminance(_14_dsa, _12_alpha, _13_sda) + _uniforms.dst.xyz) - _14_dsa) + _uniforms.src.xyz) - _13_sda, (_uniforms.src.w + _uniforms.dst.w) - _12_alpha);
        _0_blend = float4(0.0);
    }
    _out.sk_FragColor = _0_blend;
    return _out;
}
