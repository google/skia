
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
void main() {
    float _0_alpha = dst.w * src.w;
    vec3 _1_sda = src.xyz * dst.w;
    vec3 _2_dsa = dst.xyz * src.w;
    vec3 _3_blend_set_color_luminance;
    float _4_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _2_dsa);
    vec3 _5_result = (_4_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _1_sda)) + _1_sda;
    float _6_minComp = min(min(_5_result.x, _5_result.y), _5_result.z);
    float _7_maxComp = max(max(_5_result.x, _5_result.y), _5_result.z);
    if (_6_minComp < 0.0 && _4_lum != _6_minComp) {
        float _8_d = _4_lum - _6_minComp;
        _5_result = _4_lum + (_5_result - _4_lum) * (_4_lum / _8_d);
    }
    if (_7_maxComp > _0_alpha && _7_maxComp != _4_lum) {
        vec3 _9_n = (_5_result - _4_lum) * (_0_alpha - _4_lum);
        float _10_d = _7_maxComp - _4_lum;
        _3_blend_set_color_luminance = _4_lum + _9_n / _10_d;
    } else {
        _3_blend_set_color_luminance = _5_result;
    }
    sk_FragColor = vec4((((_3_blend_set_color_luminance + dst.xyz) - _2_dsa) + src.xyz) - _1_sda, (src.w + dst.w) - _0_alpha);
}
