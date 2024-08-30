
const float sk_PrivkGuardedDivideEpsilon = false ? 1e-08 : 0.0;
const float sk_PrivkMinNormalHalf = 6.10351562e-05;
out vec4 sk_FragColor;
uniform vec4 color;
float blend_color_saturation_Qhh3(vec3 color);
vec4 blend_hslc_h4h2h4h4(vec2 flipSat, vec4 src, vec4 dst);
float blend_color_saturation_Qhh3(vec3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
vec4 blend_hslc_h4h2h4h4(vec2 flipSat, vec4 src, vec4 dst) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    vec3 l = bool(flipSat.x) ? dsa : sda;
    vec3 r = bool(flipSat.x) ? sda : dsa;
    if (bool(flipSat.y)) {
        float _2_mn = min(min(l.x, l.y), l.z);
        float _3_mx = max(max(l.x, l.y), l.z);
        l = _3_mx > _2_mn ? ((l - _2_mn) * blend_color_saturation_Qhh3(r)) / (_3_mx - _2_mn) : vec3(0.0);
        r = dsa;
    }
    float _4_lum = dot(vec3(0.3, 0.59, 0.11), r);
    vec3 _5_result = (_4_lum - dot(vec3(0.3, 0.59, 0.11), l)) + l;
    float _6_minComp = min(min(_5_result.x, _5_result.y), _5_result.z);
    float _7_maxComp = max(max(_5_result.x, _5_result.y), _5_result.z);
    if (_6_minComp < 0.0 && _4_lum != _6_minComp) {
        _5_result = _4_lum + (_5_result - _4_lum) * (_4_lum / (((_4_lum - _6_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
    }
    if (_7_maxComp > alpha && _7_maxComp != _4_lum) {
        _5_result = _4_lum + ((_5_result - _4_lum) * (alpha - _4_lum)) / (((_7_maxComp - _4_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
    }
    return vec4((((_5_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    float _1_c = color.x * color.y + color.z;
    sk_FragColor = vec4(_1_c);
    sk_FragColor *= 1.25;
    sk_FragColor *= color.xxyy * color.w;
    sk_FragColor *= color.zzww * color.y;
    sk_FragColor *= blend_hslc_h4h2h4h4(vec2(0.0, 1.0), color, color.wwww);
    sk_FragColor *= blend_hslc_h4h2h4h4(vec2(0.0, 1.0), color, color.wzyx);
}
