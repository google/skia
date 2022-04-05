#version 400
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec3 blend_set_color_saturation_helper_Qh3h3h(vec3 minMidMax, float sat) {
    vec2 delta = minMidMax.yz - minMidMax.xx;
    return delta.y >= 9.9999997473787516e-06 ? vec3(0.0, (delta.x / delta.y) * sat, sat) : vec3(0.0);
}
vec4 blend_hslc_h4h4h4hb(vec4 src, vec4 dst, float flip, bool saturate) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    vec3 l = mix(sda, dsa, flip);
    vec3 r = mix(dsa, sda, flip);
    if (saturate) {
        vec3 _0_blend_set_color_saturation;
        float _1_sat = max(max(r.x, r.y), r.z) - min(min(r.x, r.y), r.z);
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
    vec3 _2_blend_set_color_luminance;
    float _3_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), r);
    vec3 _4_result = (_3_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), l)) + l;
    float _5_minComp = min(min(_4_result.x, _4_result.y), _4_result.z);
    float _6_maxComp = max(max(_4_result.x, _4_result.y), _4_result.z);
    if (_5_minComp < 0.0 && _3_lum != _5_minComp) {
        _4_result = _3_lum + (_4_result - _3_lum) * (_3_lum / (_3_lum - _5_minComp));
    }
    if (_6_maxComp > alpha && _6_maxComp != _3_lum) {
        _2_blend_set_color_luminance = _3_lum + ((_4_result - _3_lum) * (alpha - _3_lum)) / (_6_maxComp - _3_lum);
    } else {
        _2_blend_set_color_luminance = _4_result;
    }
    return vec4((((_2_blend_set_color_luminance + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    sk_FragColor = blend_hslc_h4h4h4hb(src, dst, 0.0, true);
}
