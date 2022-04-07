
out vec4 sk_FragColor;
uniform vec4 color;
vec3 blend_set_color_saturation_helper_Qh3h3h3(vec3 minMidMax, vec3 satColor) {
    if (minMidMax.z > minMidMax.x) {
        minMidMax.yz -= minMidMax.xx;
        float sat = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
        return vec3(0.0, sat * (minMidMax.y / minMidMax.z), sat);
    } else {
        return vec3(0.0);
    }
}
vec4 blend_hslc_h4h4h4bb(vec4 src, vec4 dst, bool flip, bool saturate) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    vec3 l = flip ? dsa : sda;
    vec3 r = flip ? sda : dsa;
    if (saturate) {
        vec3 _2_blend_set_color_saturation;
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
    float _3_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), r);
    vec3 _4_result = (_3_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), l)) + l;
    float _5_minComp = min(min(_4_result.x, _4_result.y), _4_result.z);
    float _6_maxComp = max(max(_4_result.x, _4_result.y), _4_result.z);
    if (_5_minComp < 0.0 && _3_lum != _5_minComp) {
        _4_result = _3_lum + (_4_result - _3_lum) * (_3_lum / (_3_lum - _5_minComp));
    }
    if (_6_maxComp > alpha && _6_maxComp != _3_lum) {
        _4_result = _3_lum + ((_4_result - _3_lum) * (alpha - _3_lum)) / (_6_maxComp - _3_lum);
    }
    return vec4((((_4_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    float _1_c = color.x * color.y + color.z;
    sk_FragColor = vec4(_1_c);
    sk_FragColor *= 1.25;
    sk_FragColor *= color.xxyy * color.w;
    sk_FragColor *= color.zzww * color.y;
    sk_FragColor *= blend_hslc_h4h4h4bb(color, color.wwww, false, true);
    sk_FragColor *= blend_hslc_h4h4h4bb(color, color.wzyx, false, true);
}
