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
half3 blend_set_color_saturation_helper_Qh3h3h(half3 minMidMax, half sat) {
    if (minMidMax.x < minMidMax.z) {
        return half3(0.0h, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat);
    } else {
        return half3(0.0h);
    }
}
half4 blend_hslc_h4h4h4hb(half4 src, half4 dst, half flip, bool saturate) {
    half alpha = dst.w * src.w;
    half3 sda = src.xyz * dst.w;
    half3 dsa = dst.xyz * src.w;
    half3 l = mix(sda, dsa, flip);
    half3 r = mix(dsa, sda, flip);
    if (saturate) {
        half3 _2_blend_set_color_saturation;
        half _3_sat = max(max(r.x, r.y), r.z) - min(min(r.x, r.y), r.z);
        if (l.x <= l.y) {
            if (l.y <= l.z) {
                _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l, _3_sat);
            } else if (l.x <= l.z) {
                _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.xzy, _3_sat).xzy;
            } else {
                _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.zxy, _3_sat).yzx;
            }
        } else if (l.x <= l.z) {
            _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.yxz, _3_sat).yxz;
        } else if (l.y <= l.z) {
            _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.yzx, _3_sat).zxy;
        } else {
            _2_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.zyx, _3_sat).zyx;
        }
        l = _2_blend_set_color_saturation;
        r = dsa;
    }
    half3 _4_blend_set_color_luminance;
    half _5_lum = dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), r);
    half3 _6_result = (_5_lum - dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), l)) + l;
    half _7_minComp = min(min(_6_result.x, _6_result.y), _6_result.z);
    half _8_maxComp = max(max(_6_result.x, _6_result.y), _6_result.z);
    if (_7_minComp < 0.0h && _5_lum != _7_minComp) {
        _6_result = _5_lum + (_6_result - _5_lum) * (_5_lum / (_5_lum - _7_minComp));
    }
    if (_8_maxComp > alpha && _8_maxComp != _5_lum) {
        _4_blend_set_color_luminance = _5_lum + ((_6_result - _5_lum) * (alpha - _5_lum)) / (_8_maxComp - _5_lum);
    } else {
        _4_blend_set_color_luminance = _6_result;
    }
    return half4((((_4_blend_set_color_luminance + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_hslc_h4h4h4hb(_uniforms.src, _uniforms.dst, 0.0h, true);
    return _out;
}
