
out vec4 sk_FragColor;
vec3 _blend_set_color_saturation_helper(vec3 minMidMax, float sat) {
    return minMidMax.x < minMidMax.z ? vec3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat) : vec3(0.0);
}
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_hue;
    float _1_alpha = dst.w * src.w;
    vec3 _2_sda = src.xyz * dst.w;
    vec3 _3_dsa = dst.xyz * src.w;
    vec3 _4_blend_set_color_saturation;
    float _5_blend_color_saturation;
    _5_blend_color_saturation = max(max(_3_dsa.x, _3_dsa.y), _3_dsa.z) - min(min(_3_dsa.x, _3_dsa.y), _3_dsa.z);

    float _6_sat = _5_blend_color_saturation;

    if (_2_sda.x <= _2_sda.y) {
        if (_2_sda.y <= _2_sda.z) {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda, _6_sat);
        } else if (_2_sda.x <= _2_sda.z) {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda.xzy, _6_sat).xzy;
        } else {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda.zxy, _6_sat).yzx;
        }
    } else if (_2_sda.x <= _2_sda.z) {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda.yxz, _6_sat).yxz;
    } else if (_2_sda.y <= _2_sda.z) {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda.yzx, _6_sat).zxy;
    } else {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda.zyx, _6_sat).zyx;
    }

    vec3 _7_blend_set_color_luminance;
    float _8_blend_color_luminance;
    _8_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);

    float _9_lum = _8_blend_color_luminance;

    float _10_blend_color_luminance;
    _10_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _4_blend_set_color_saturation);

    vec3 _11_result = (_9_lum - _10_blend_color_luminance) + _4_blend_set_color_saturation;

    float _12_minComp = min(min(_11_result.x, _11_result.y), _11_result.z);
    float _13_maxComp = max(max(_11_result.x, _11_result.y), _11_result.z);
    if (_12_minComp < 0.0 && _9_lum != _12_minComp) {
        _11_result = _9_lum + ((_11_result - _9_lum) * _9_lum) / (_9_lum - _12_minComp);
    }
    _7_blend_set_color_luminance = _13_maxComp > _1_alpha && _13_maxComp != _9_lum ? _9_lum + ((_11_result - _9_lum) * (_1_alpha - _9_lum)) / (_13_maxComp - _9_lum) : _11_result;

    _0_blend_hue = vec4((((_7_blend_set_color_luminance + dst.xyz) - _3_dsa) + src.xyz) - _2_sda, (src.w + dst.w) - _1_alpha);



    sk_FragColor = _0_blend_hue;

}
