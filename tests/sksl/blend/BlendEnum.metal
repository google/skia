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
float _blend_overlay_component_hh2h2(float2 s, float2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
float4 blend_overlay_h4h4h4(float4 src, float4 dst) {
    float4 result = float4(_blend_overlay_component_hh2h2(src.xw, dst.xw), _blend_overlay_component_hh2h2(src.yw, dst.yw), _blend_overlay_component_hh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    result.xyz = result.xyz + dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
float _color_dodge_component_hh2h2(float2 s, float2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            delta = min(d.y, (d.x * s.y) / delta);
            return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        }
    }
}
float _color_burn_component_hh2h2(float2 s, float2 d) {
    if (d.y == d.x) {
        return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    } else if (s.x == 0.0) {
        return d.x * (1.0 - s.y);
    } else {
        float delta = max(0.0, d.y - ((d.y - d.x) * s.y) / s.x);
        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component_hh2h2(float2 s, float2 d) {
    if (2.0 * s.x <= s.y) {
        return (((d.x * d.x) * (s.y - 2.0 * s.x)) / d.y + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);
    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        return (((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x) / DaSqd;
    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
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
float4 blend_h4eh4h4(int mode, float4 src, float4 dst) {
    switch (mode) {
        case 0:
            return float4(0.0);
        case 1:
            return src;
        case 2:
            return dst;
        case 3:
            return src + (1.0 - src.w) * dst;
        case 4:
            return (1.0 - dst.w) * src + dst;
        case 5:
            return src * dst.w;
        case 6:
            return dst * src.w;
        case 7:
            return (1.0 - dst.w) * src;
        case 8:
            return (1.0 - src.w) * dst;
        case 9:
            return dst.w * src + (1.0 - src.w) * dst;
        case 10:
            return (1.0 - dst.w) * src + src.w * dst;
        case 11:
            return (1.0 - dst.w) * src + (1.0 - src.w) * dst;
        case 12:
            return min(src + dst, 1.0);
        case 13:
            return src * dst;
        case 14:
            return src + (1.0 - src) * dst;
        case 15:
            return blend_overlay_h4h4h4(src, dst);
        case 16:
            float4 _0_result = src + (1.0 - src.w) * dst;
            _0_result.xyz = min(_0_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
            return _0_result;
        case 17:
            float4 _1_result = src + (1.0 - src.w) * dst;
            _1_result.xyz = max(_1_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
            return _1_result;
        case 18:
            return float4(_color_dodge_component_hh2h2(src.xw, dst.xw), _color_dodge_component_hh2h2(src.yw, dst.yw), _color_dodge_component_hh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
        case 19:
            return float4(_color_burn_component_hh2h2(src.xw, dst.xw), _color_burn_component_hh2h2(src.yw, dst.yw), _color_burn_component_hh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
        case 20:
            return blend_overlay_h4h4h4(dst, src);
        case 21:
            return dst.w == 0.0 ? src : float4(_soft_light_component_hh2h2(src.xw, dst.xw), _soft_light_component_hh2h2(src.yw, dst.yw), _soft_light_component_hh2h2(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
        case 22:
            return float4((src.xyz + dst.xyz) - 2.0 * min(src.xyz * dst.w, dst.xyz * src.w), src.w + (1.0 - src.w) * dst.w);
        case 23:
            return float4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);
        case 24:
            return float4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
        case 25:
            float _2_alpha = dst.w * src.w;
            float3 _3_sda = src.xyz * dst.w;
            float3 _4_dsa = dst.xyz * src.w;
            return float4((((_blend_set_color_luminance_h3h3hh3(_blend_set_color_saturation_h3h3h3(_3_sda, _4_dsa), _2_alpha, _4_dsa) + dst.xyz) - _4_dsa) + src.xyz) - _3_sda, (src.w + dst.w) - _2_alpha);
        case 26:
            float _5_alpha = dst.w * src.w;
            float3 _6_sda = src.xyz * dst.w;
            float3 _7_dsa = dst.xyz * src.w;
            return float4((((_blend_set_color_luminance_h3h3hh3(_blend_set_color_saturation_h3h3h3(_7_dsa, _6_sda), _5_alpha, _7_dsa) + dst.xyz) - _7_dsa) + src.xyz) - _6_sda, (src.w + dst.w) - _5_alpha);
        case 27:
            float _8_alpha = dst.w * src.w;
            float3 _9_sda = src.xyz * dst.w;
            float3 _10_dsa = dst.xyz * src.w;
            return float4((((_blend_set_color_luminance_h3h3hh3(_9_sda, _8_alpha, _10_dsa) + dst.xyz) - _10_dsa) + src.xyz) - _9_sda, (src.w + dst.w) - _8_alpha);
        case 28:
            float _11_alpha = dst.w * src.w;
            float3 _12_sda = src.xyz * dst.w;
            float3 _13_dsa = dst.xyz * src.w;
            return float4((((_blend_set_color_luminance_h3h3hh3(_13_dsa, _11_alpha, _12_sda) + dst.xyz) - _13_dsa) + src.xyz) - _12_sda, (src.w + dst.w) - _11_alpha);
        default:
            return float4(0.0);
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_h4eh4h4(13, _uniforms.src, _uniforms.dst);
    return _out;
}
