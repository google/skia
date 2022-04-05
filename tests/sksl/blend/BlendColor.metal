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
    half2 delta = minMidMax.yz - minMidMax.xx;
    return delta.y >= 9.9999997473787516e-06h ? half3(0.0h, (delta.x / delta.y) * sat, sat) : half3(0.0h);
}
half4 blend_hslc_h4h4h4hb(half4 src, half4 dst, half flip, bool saturate) {
    half alpha = dst.w * src.w;
    half3 sda = src.xyz * dst.w;
    half3 dsa = dst.xyz * src.w;
    half3 l = mix(sda, dsa, flip);
    half3 r = mix(dsa, sda, flip);
    if (saturate) {
        half3 _0_blend_set_color_saturation;
        half _1_sat = max(max(r.x, r.y), r.z) - min(min(r.x, r.y), r.z);
        if (l.x <= l.y) {
            if (l.y <= l.z) {
                _0_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l, _1_sat);
            } else if (l.x <= l.z) {
                _0_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.xzy, _1_sat).xzy;
            } else {
                _0_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.zxy, _1_sat).yzx;
            }
        } else if (l.x <= l.z) {
            _0_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.yxz, _1_sat).yxz;
        } else if (l.y <= l.z) {
            _0_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.yzx, _1_sat).zxy;
        } else {
            _0_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.zyx, _1_sat).zyx;
        }
        l = _0_blend_set_color_saturation;
        r = dsa;
    }
    half3 _2_blend_set_color_luminance;
    half _3_lum = dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), r);
    half3 _4_result = (_3_lum - dot(half3(0.30000001192092896h, 0.5899999737739563h, 0.10999999940395355h), l)) + l;
    half _5_minComp = min(min(_4_result.x, _4_result.y), _4_result.z);
    half _6_maxComp = max(max(_4_result.x, _4_result.y), _4_result.z);
    if (_5_minComp < 0.0h && _3_lum != _5_minComp) {
        _4_result = _3_lum + (_4_result - _3_lum) * (_3_lum / (_3_lum - _5_minComp));
    }
    if (_6_maxComp > alpha && _6_maxComp != _3_lum) {
        _2_blend_set_color_luminance = _3_lum + ((_4_result - _3_lum) * (alpha - _3_lum)) / (_6_maxComp - _3_lum);
    } else {
        _2_blend_set_color_luminance = _4_result;
    }
    return half4((((_2_blend_set_color_luminance + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = blend_hslc_h4h4h4hb(_uniforms.src, _uniforms.dst, 0.0h, false);
    return _out;
}
