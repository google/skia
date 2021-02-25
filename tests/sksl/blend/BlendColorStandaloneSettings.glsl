
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_color;
    float _1_alpha = dst.w * src.w;
    vec3 _2_sda = src.xyz * dst.w;
    vec3 _3_dsa = dst.xyz * src.w;
    vec3 _4_blend_set_color_luminance;
    float _5_blend_color_luminance;
    float _6_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);

    float _7_blend_color_luminance;
    vec3 _8_result = (_6_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _2_sda)) + _2_sda;

    float _9_minComp = min(min(_8_result.x, _8_result.y), _8_result.z);
    float _10_maxComp = max(max(_8_result.x, _8_result.y), _8_result.z);
    if (_9_minComp < 0.0 && _6_lum != _9_minComp) {
        float _11_guarded_divide;
        float _12_d = _6_lum - _9_minComp;
        _8_result = _6_lum + (_8_result - _6_lum) * (_6_lum / _12_d);

    }
    if (_10_maxComp > _1_alpha && _10_maxComp != _6_lum) {
        vec3 _13_guarded_divide;
        vec3 _14_n = (_8_result - _6_lum) * (_1_alpha - _6_lum);
        float _15_d = _10_maxComp - _6_lum;
        _4_blend_set_color_luminance = _6_lum + _14_n / _15_d;

    } else {
        _4_blend_set_color_luminance = _8_result;
    }
    sk_FragColor = vec4((((_4_blend_set_color_luminance + dst.xyz) - _3_dsa) + src.xyz) - _2_sda, (src.w + dst.w) - _1_alpha);


}
