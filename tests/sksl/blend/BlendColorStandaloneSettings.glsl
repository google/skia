
const float sk_PrivkGuardedDivideEpsilon = false ? 1e-08 : 0.0;
const float sk_PrivkHalfEpsilon = 0.000244140625;
const float sk_PrivkMinNormalHalf = 6.10351562e-05;
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
float blend_color_saturation_Qhh3(vec3 color);
vec3 guarded_divide_Qh3h3h(vec3 n, float d);
vec4 blend_hslc_h4h2h4h4(vec2 flipSat, vec4 src, vec4 dst);
float blend_color_saturation_Qhh3(vec3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
vec3 guarded_divide_Qh3h3h(vec3 n, float d) {
    return n / (d + sk_PrivkGuardedDivideEpsilon);
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
        float _4_diff = _3_mx - _2_mn;
        l = _4_diff >= sk_PrivkHalfEpsilon ? guarded_divide_Qh3h3h((l - _2_mn) * blend_color_saturation_Qhh3(r), _4_diff) : vec3(0.0);
        r = dsa;
    }
    float _5_lum = dot(vec3(0.3, 0.59, 0.11), r);
    vec3 _6_result = (_5_lum - dot(vec3(0.3, 0.59, 0.11), l)) + l;
    float _7_minComp = min(min(_6_result.x, _6_result.y), _6_result.z);
    float _8_maxComp = max(max(_6_result.x, _6_result.y), _6_result.z);
    if (_7_minComp < 0.0 && _5_lum != _7_minComp) {
        _6_result = _5_lum + (_6_result - _5_lum) * (_5_lum / (((_5_lum - _7_minComp) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon));
    }
    if (_8_maxComp > alpha && _8_maxComp != _5_lum) {
        _6_result = _5_lum + ((_6_result - _5_lum) * (alpha - _5_lum)) / (((_8_maxComp - _5_lum) + sk_PrivkMinNormalHalf) + sk_PrivkGuardedDivideEpsilon);
    }
    return vec4((((_6_result + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    sk_FragColor = blend_hslc_h4h2h4h4(vec2(0.0), src, dst);
}
