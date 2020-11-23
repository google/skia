
out vec4 sk_FragColor;
vec3 _blend_set_color_saturation_helper(vec3 minMidMax, float sat) {
    return minMidMax.x < minMidMax.z ? vec3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat) : vec3(0.0);
}
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_hue;
    {
        float _1_alpha = dst.w * src.w;
        vec3 _2_sda = src.xyz * dst.w;
        vec3 _3_dsa = dst.xyz * src.w;
        vec3 _4_blend_set_color_saturation;
        vec3 _5_hueLumColor = _2_sda;
        {
            float _6_13_blend_color_saturation;
            {
                _6_13_blend_color_saturation = max(max(_3_dsa.x, _3_dsa.y), _3_dsa.z) - min(min(_3_dsa.x, _3_dsa.y), _3_dsa.z);
            }
            float _7_sat = _6_13_blend_color_saturation;

            if (_5_hueLumColor.x <= _5_hueLumColor.y) {
                if (_5_hueLumColor.y <= _5_hueLumColor.z) {
                    _5_hueLumColor.xyz = _blend_set_color_saturation_helper(_5_hueLumColor, _7_sat);
                } else if (_5_hueLumColor.x <= _5_hueLumColor.z) {
                    _5_hueLumColor.xzy = _blend_set_color_saturation_helper(_5_hueLumColor.xzy, _7_sat);
                } else {
                    _5_hueLumColor.zxy = _blend_set_color_saturation_helper(_5_hueLumColor.zxy, _7_sat);
                }
            } else if (_5_hueLumColor.x <= _5_hueLumColor.z) {
                _5_hueLumColor.yxz = _blend_set_color_saturation_helper(_5_hueLumColor.yxz, _7_sat);
            } else if (_5_hueLumColor.y <= _5_hueLumColor.z) {
                _5_hueLumColor.yzx = _blend_set_color_saturation_helper(_5_hueLumColor.yzx, _7_sat);
            } else {
                _5_hueLumColor.zyx = _blend_set_color_saturation_helper(_5_hueLumColor.zyx, _7_sat);
            }
            _4_blend_set_color_saturation = _5_hueLumColor;
        }
        vec3 _8_blend_set_color_luminance;
        {
            float _9_11_blend_color_luminance;
            {
                _9_11_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);
            }
            float _10_lum = _9_11_blend_color_luminance;

            float _11_12_blend_color_luminance;
            {
                _11_12_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _4_blend_set_color_saturation);
            }
            vec3 _12_result = (_10_lum - _11_12_blend_color_luminance) + _4_blend_set_color_saturation;

            float _13_minComp = min(min(_12_result.x, _12_result.y), _12_result.z);
            float _14_maxComp = max(max(_12_result.x, _12_result.y), _12_result.z);
            if (_13_minComp < 0.0 && _10_lum != _13_minComp) {
                _12_result = _10_lum + ((_12_result - _10_lum) * _10_lum) / (_10_lum - _13_minComp);
            }
            _8_blend_set_color_luminance = _14_maxComp > _1_alpha && _14_maxComp != _10_lum ? _10_lum + ((_12_result - _10_lum) * (_1_alpha - _10_lum)) / (_14_maxComp - _10_lum) : _12_result;
        }
        _0_blend_hue = vec4((((_8_blend_set_color_luminance + dst.xyz) - _3_dsa) + src.xyz) - _2_sda, (src.w + dst.w) - _1_alpha);


    }
    sk_FragColor = _0_blend_hue;

}
