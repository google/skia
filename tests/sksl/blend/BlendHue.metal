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
half3 blend_set_color_saturation_helper_Qh3h3h3(half3 minMidMax, half3 satColor) {
    if (minMidMax.z > minMidMax.x) {
        minMidMax.yz = minMidMax.yz - minMidMax.xx;
        half sat = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
        return half3(0.0h, sat * (minMidMax.y / minMidMax.z), sat);
    } else {
        return half3(0.0h);
    }
}
half4 blend_hslc_h4h4h4bb(half4 src, half4 dst, bool flip, bool saturate) {
    half alpha = dst.w * src.w;
    half3 sda = src.xyz * dst.w;
    half3 dsa = dst.xyz * src.w;
    half3 l = flip ? dsa : sda;
    half3 r = flip ? sda : dsa;
    if (saturate) {
        half3 _2_blend_set_color_saturation;
        if (l.x <= l.y) {
            if (l.y <= l.z) {
                _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h3(l, r);
            } else if (l.x <= l.z) {
                _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h3(l.xzy, r).xzy;
            } else {
                _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h3(l.zxy, r).yzx;
            }
        } else if (l.x <= l.z) {
            _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h3(l.yxz, r).yxz;
        } else if (l.y <= l.z) {
            _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h3(l.yzx, r).zxy;
        } else {
            _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h3(l.zyx, r).zyx;
        }
        l = _2_blend_set_color_saturation;
        r = dsa;
    }
    half _3_lum = dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), r);
    half3 _4_result = (_3_lum - dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), l)) + l;
    half _5_minComp = min(min(_4_result.x, _4_result.y), _4_result.z);
    half _6_maxComp = max(max(_4_result.x, _4_result.y), _4_result.z);
    if (_5_minComp < 0.0h && _3_lum != _5_minComp) {
        _4_result = _3_lum + (_4_result - _3_lum) * (_3_lum / (_3_lum - _5_minComp));
    }
    if (_6_maxComp > alpha && _6_maxComp != _3_lum) {
        _4_result = _3_lum + ((_4_result - _3_lum) * (alpha - _3_lum)) / (_6_maxComp - _3_lum);
    }
    return half4((((_4_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_hslc_h4h4h4bb(_uniforms.src, _uniforms.dst, false, true);
    return _out;
}
