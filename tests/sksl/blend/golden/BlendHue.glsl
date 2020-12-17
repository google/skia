#version 400
out vec4 sk_FragColor;
vec3 _blend_set_color_saturation_helper(vec3 minMidMax, float sat) {
    return minMidMax.x < minMidMax.z ? vec3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat) : vec3(0.0);
}
in vec4 src;
in vec4 dst;
void main() {
    float _1_alpha = dst.w * src.w;
    vec3 _2_sda = src.xyz * dst.w;
    vec3 _3_dsa = dst.xyz * src.w;
    vec3 _4_blend_set_color_saturation;
    float _5_sat = max(max(_3_dsa.x, _3_dsa.y), _3_dsa.z) - min(min(_3_dsa.x, _3_dsa.y), _3_dsa.z);

    if (_2_sda.x <= _2_sda.y) {
        if (_2_sda.y <= _2_sda.z) {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda, _5_sat);
        } else if (_2_sda.x <= _2_sda.z) {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda.xzy, _5_sat).xzy;
        } else {
            _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda.zxy, _5_sat).yzx;
        }
    } else if (_2_sda.x <= _2_sda.z) {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda.yxz, _5_sat).yxz;
    } else if (_2_sda.y <= _2_sda.z) {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda.yzx, _5_sat).zxy;
    } else {
        _4_blend_set_color_saturation = _blend_set_color_saturation_helper(_2_sda.zyx, _5_sat).zyx;
    }
    float _7_lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);

    vec3 _8_result = (_7_lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _4_blend_set_color_saturation)) + _4_blend_set_color_saturation;

    float _9_minComp = min(min(_8_result.x, _8_result.y), _8_result.z);
    float _10_maxComp = max(max(_8_result.x, _8_result.y), _8_result.z);
    if (_9_minComp < 0.0 && _7_lum != _9_minComp) {
        _8_result = _7_lum + ((_8_result - _7_lum) * _7_lum) / (_7_lum - _9_minComp);
    }
    sk_FragColor = vec4(((((_10_maxComp > _1_alpha && _10_maxComp != _7_lum ? _7_lum + ((_8_result - _7_lum) * (_1_alpha - _7_lum)) / (_10_maxComp - _7_lum) : _8_result) + dst.xyz) - _3_dsa) + src.xyz) - _2_sda, (src.w + dst.w) - _1_alpha);



}
