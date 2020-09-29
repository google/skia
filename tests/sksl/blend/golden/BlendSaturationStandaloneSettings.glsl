
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _12_blend_saturation;
    {
        float _13_alpha = dst.w * src.w;
        vec3 _14_sda = src.xyz * dst.w;
        vec3 _15_dsa = dst.xyz * src.w;
        vec3 _16_blend_set_color_saturation;
        vec3 _17_hueLumColor = _15_dsa;
        {
            float _19_blend_color_saturation;
            {
                _19_blend_color_saturation = max(max(_14_sda.x, _14_sda.y), _14_sda.z) - min(min(_14_sda.x, _14_sda.y), _14_sda.z);
            }
            float _18_sat = _19_blend_color_saturation;

            if (_17_hueLumColor.x <= _17_hueLumColor.y) {
                if (_17_hueLumColor.y <= _17_hueLumColor.z) {
                    vec3 _20_blend_set_color_saturation_helper;
                    {
                        _20_blend_set_color_saturation_helper = _17_hueLumColor.x < _17_hueLumColor.z ? vec3(0.0, (_18_sat * (_17_hueLumColor.y - _17_hueLumColor.x)) / (_17_hueLumColor.z - _17_hueLumColor.x), _18_sat) : vec3(0.0);
                    }
                    _17_hueLumColor.xyz = _20_blend_set_color_saturation_helper;

                } else if (_17_hueLumColor.x <= _17_hueLumColor.z) {
                    vec3 _21_blend_set_color_saturation_helper;
                    {
                        _21_blend_set_color_saturation_helper = _17_hueLumColor.x < _17_hueLumColor.y ? vec3(0.0, (_18_sat * (_17_hueLumColor.z - _17_hueLumColor.x)) / (_17_hueLumColor.y - _17_hueLumColor.x), _18_sat) : vec3(0.0);
                    }
                    _17_hueLumColor.xzy = _21_blend_set_color_saturation_helper;

                } else {
                    vec3 _22_blend_set_color_saturation_helper;
                    {
                        _22_blend_set_color_saturation_helper = _17_hueLumColor.z < _17_hueLumColor.y ? vec3(0.0, (_18_sat * (_17_hueLumColor.x - _17_hueLumColor.z)) / (_17_hueLumColor.y - _17_hueLumColor.z), _18_sat) : vec3(0.0);
                    }
                    _17_hueLumColor.zxy = _22_blend_set_color_saturation_helper;

                }
            } else if (_17_hueLumColor.x <= _17_hueLumColor.z) {
                vec3 _23_blend_set_color_saturation_helper;
                {
                    _23_blend_set_color_saturation_helper = _17_hueLumColor.y < _17_hueLumColor.z ? vec3(0.0, (_18_sat * (_17_hueLumColor.x - _17_hueLumColor.y)) / (_17_hueLumColor.z - _17_hueLumColor.y), _18_sat) : vec3(0.0);
                }
                _17_hueLumColor.yxz = _23_blend_set_color_saturation_helper;

            } else if (_17_hueLumColor.y <= _17_hueLumColor.z) {
                vec3 _24_blend_set_color_saturation_helper;
                {
                    _24_blend_set_color_saturation_helper = _17_hueLumColor.y < _17_hueLumColor.x ? vec3(0.0, (_18_sat * (_17_hueLumColor.z - _17_hueLumColor.y)) / (_17_hueLumColor.x - _17_hueLumColor.y), _18_sat) : vec3(0.0);
                }
                _17_hueLumColor.yzx = _24_blend_set_color_saturation_helper;

            } else {
                vec3 _25_blend_set_color_saturation_helper;
                {
                    _25_blend_set_color_saturation_helper = _17_hueLumColor.z < _17_hueLumColor.x ? vec3(0.0, (_18_sat * (_17_hueLumColor.y - _17_hueLumColor.z)) / (_17_hueLumColor.x - _17_hueLumColor.z), _18_sat) : vec3(0.0);
                }
                _17_hueLumColor.zyx = _25_blend_set_color_saturation_helper;

            }
            _16_blend_set_color_saturation = _17_hueLumColor;
        }
        vec3 _26_blend_set_color_luminance;
        {
            float _31_blend_color_luminance;
            {
                _31_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _15_dsa);
            }
            float _27_lum = _31_blend_color_luminance;

            float _32_blend_color_luminance;
            {
                _32_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _16_blend_set_color_saturation);
            }
            vec3 _28_result = (_27_lum - _32_blend_color_luminance) + _16_blend_set_color_saturation;

            float _29_minComp = min(min(_28_result.x, _28_result.y), _28_result.z);
            float _30_maxComp = max(max(_28_result.x, _28_result.y), _28_result.z);
            if (_29_minComp < 0.0 && _27_lum != _29_minComp) {
                _28_result = _27_lum + ((_28_result - _27_lum) * _27_lum) / (_27_lum - _29_minComp);
            }
            _26_blend_set_color_luminance = _30_maxComp > _13_alpha && _30_maxComp != _27_lum ? _27_lum + ((_28_result - _27_lum) * (_13_alpha - _27_lum)) / (_30_maxComp - _27_lum) : _28_result;
        }
        _12_blend_saturation = vec4((((_26_blend_set_color_luminance + dst.xyz) - _15_dsa) + src.xyz) - _14_sda, (src.w + dst.w) - _13_alpha);


    }
    sk_FragColor = _12_blend_saturation;

}
