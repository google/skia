
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
        vec3 _2_blend_set_color_saturation;
        float _3_sat = max(max(r.x, r.y), r.z) - min(min(r.x, r.y), r.z);
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
    vec3 _4_blend_set_color_luminance;
    float _5_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), r);
    vec3 _6_result = (_5_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), l)) + l;
    float _7_minComp = min(min(_6_result.x, _6_result.y), _6_result.z);
    float _8_maxComp = max(max(_6_result.x, _6_result.y), _6_result.z);
    if (_7_minComp < 0.0 && _5_lum != _7_minComp) {
        _6_result = _5_lum + (_6_result - _5_lum) * (_5_lum / (_5_lum - _7_minComp));
    }
    if (_8_maxComp > alpha && _8_maxComp != _5_lum) {
        _4_blend_set_color_luminance = _5_lum + ((_6_result - _5_lum) * (alpha - _5_lum)) / (_8_maxComp - _5_lum);
    } else {
        _4_blend_set_color_luminance = _6_result;
    }
    return vec4((((_4_blend_set_color_luminance + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    sk_FragColor = blend_hslc_h4h4h4hb(src, dst, 0.0, true);
}
