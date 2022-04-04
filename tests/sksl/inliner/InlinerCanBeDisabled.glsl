
out vec4 sk_FragColor;
uniform vec4 color;
vec4 blend_src_in_h4h4h4(vec4 src, vec4 dst) {
    return src * dst.w;
}
vec4 blend_dst_in_h4h4h4(vec4 src, vec4 dst) {
    return dst * src.w;
}
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
        vec3 _1_blend_set_color_saturation;
        float _2_sat = max(max(r.x, r.y), r.z) - min(min(r.x, r.y), r.z);
        if (l.x <= l.y) {
            if (l.y <= l.z) {
                _1_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l, _2_sat);
            } else if (l.x <= l.z) {
                _1_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.xzy, _2_sat).xzy;
            } else {
                _1_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.zxy, _2_sat).yzx;
            }
        } else if (l.x <= l.z) {
            _1_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.yxz, _2_sat).yxz;
        } else if (l.y <= l.z) {
            _1_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.yzx, _2_sat).zxy;
        } else {
            _1_blend_set_color_saturation = blend_set_color_saturation_helper_Qh3h3h(l.zyx, _2_sat).zyx;
        }
        l = _1_blend_set_color_saturation;
        r = dsa;
    }
    vec3 _3_blend_set_color_luminance;
    float _4_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), r);
    vec3 _5_result = (_4_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), l)) + l;
    float _6_minComp = min(min(_5_result.x, _5_result.y), _5_result.z);
    float _7_maxComp = max(max(_5_result.x, _5_result.y), _5_result.z);
    if (_6_minComp < 0.0 && _4_lum != _6_minComp) {
        _5_result = _4_lum + (_5_result - _4_lum) * (_4_lum / (_4_lum - _6_minComp));
    }
    if (_7_maxComp > alpha && _7_maxComp != _4_lum) {
        _3_blend_set_color_luminance = _4_lum + ((_5_result - _4_lum) * (alpha - _4_lum)) / (_7_maxComp - _4_lum);
    } else {
        _3_blend_set_color_luminance = _5_result;
    }
    return vec4((((_3_blend_set_color_luminance + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
vec4 blend_hue_h4h4h4(vec4 src, vec4 dst) {
    return blend_hslc_h4h4h4hb(src, dst, 0.0, true);
}
float singleuse_h() {
    return 1.25;
}
float add_hhh(float a, float b) {
    float c = a + b;
    return c;
}
float mul_hhh(float a, float b) {
    return a * b;
}
float fma_hhhh(float a, float b, float c) {
    return add_hhh(mul_hhh(a, b), c);
}
void main() {
    sk_FragColor = vec4(fma_hhhh(color.x, color.y, color.z));
    sk_FragColor *= singleuse_h();
    sk_FragColor *= blend_src_in_h4h4h4(color.xxyy, color.zzww);
    sk_FragColor *= blend_dst_in_h4h4h4(color.xxyy, color.zzww);
    sk_FragColor *= blend_hue_h4h4h4(color, color.wwww);
    sk_FragColor *= blend_hue_h4h4h4(color, color.wzyx);
}
