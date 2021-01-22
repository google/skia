#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_saturation;
    {
        float _1_alpha = dst.w * src.w;
        vec3 _2_sda = src.xyz * dst.w;
        vec3 _3_dsa = dst.xyz * src.w;
        vec3 _13_blend_set_color_saturation;
        vec3 _14_hueLumColor = _3_dsa;
        {
            float _16_blend_color_saturation;
            {
                _16_blend_color_saturation = max(max(_2_sda.x, _2_sda.y), _2_sda.z) - min(min(_2_sda.x, _2_sda.y), _2_sda.z);
            }
            float _15_sat = _16_blend_color_saturation;

            if (_14_hueLumColor.x <= _14_hueLumColor.y) {
                if (_14_hueLumColor.y <= _14_hueLumColor.z) {
                    vec3 _17_blend_set_color_saturation_helper;
                    {
                        _17_blend_set_color_saturation_helper = _14_hueLumColor.x < _14_hueLumColor.z ? vec3(0.0, (_15_sat * (_14_hueLumColor.y - _14_hueLumColor.x)) / (_14_hueLumColor.z - _14_hueLumColor.x), _15_sat) : vec3(0.0);
                    }
                    _14_hueLumColor.xyz = _17_blend_set_color_saturation_helper;

                } else if (_14_hueLumColor.x <= _14_hueLumColor.z) {
                    vec3 _18_blend_set_color_saturation_helper;
                    {
                        _18_blend_set_color_saturation_helper = _14_hueLumColor.x < _14_hueLumColor.y ? vec3(0.0, (_15_sat * (_14_hueLumColor.z - _14_hueLumColor.x)) / (_14_hueLumColor.y - _14_hueLumColor.x), _15_sat) : vec3(0.0);
                    }
                    _14_hueLumColor.xzy = _18_blend_set_color_saturation_helper;

                } else {
                    vec3 _19_blend_set_color_saturation_helper;
                    {
                        _19_blend_set_color_saturation_helper = _14_hueLumColor.z < _14_hueLumColor.y ? vec3(0.0, (_15_sat * (_14_hueLumColor.x - _14_hueLumColor.z)) / (_14_hueLumColor.y - _14_hueLumColor.z), _15_sat) : vec3(0.0);
                    }
                    _14_hueLumColor.zxy = _19_blend_set_color_saturation_helper;

                }
            } else if (_14_hueLumColor.x <= _14_hueLumColor.z) {
                vec3 _20_blend_set_color_saturation_helper;
                {
                    _20_blend_set_color_saturation_helper = _14_hueLumColor.y < _14_hueLumColor.z ? vec3(0.0, (_15_sat * (_14_hueLumColor.x - _14_hueLumColor.y)) / (_14_hueLumColor.z - _14_hueLumColor.y), _15_sat) : vec3(0.0);
                }
                _14_hueLumColor.yxz = _20_blend_set_color_saturation_helper;

            } else if (_14_hueLumColor.y <= _14_hueLumColor.z) {
                vec3 _21_blend_set_color_saturation_helper;
                {
                    _21_blend_set_color_saturation_helper = _14_hueLumColor.y < _14_hueLumColor.x ? vec3(0.0, (_15_sat * (_14_hueLumColor.z - _14_hueLumColor.y)) / (_14_hueLumColor.x - _14_hueLumColor.y), _15_sat) : vec3(0.0);
                }
                _14_hueLumColor.yzx = _21_blend_set_color_saturation_helper;

            } else {
                vec3 _22_blend_set_color_saturation_helper;
                {
                    _22_blend_set_color_saturation_helper = _14_hueLumColor.z < _14_hueLumColor.x ? vec3(0.0, (_15_sat * (_14_hueLumColor.y - _14_hueLumColor.z)) / (_14_hueLumColor.x - _14_hueLumColor.z), _15_sat) : vec3(0.0);
                }
                _14_hueLumColor.zyx = _22_blend_set_color_saturation_helper;

            }
            _13_blend_set_color_saturation = _14_hueLumColor;
        }
        vec3 _23_blend_set_color_luminance;
        {
            float _28_blend_color_luminance;
            {
                _28_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);
            }
            float _24_lum = _28_blend_color_luminance;

            float _29_blend_color_luminance;
            {
                _29_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _13_blend_set_color_saturation);
            }
            vec3 _25_result = (_24_lum - _29_blend_color_luminance) + _13_blend_set_color_saturation;

            float _26_minComp = min(min(_25_result.x, _25_result.y), _25_result.z);
            float _27_maxComp = max(max(_25_result.x, _25_result.y), _25_result.z);
            if (_26_minComp < 0.0 && _24_lum != _26_minComp) {
                _25_result = _24_lum + ((_25_result - _24_lum) * _24_lum) / (_24_lum - _26_minComp);
            }
            _23_blend_set_color_luminance = _27_maxComp > _1_alpha && _27_maxComp != _24_lum ? _24_lum + ((_25_result - _24_lum) * (_1_alpha - _24_lum)) / (_27_maxComp - _24_lum) : _25_result;
        }
        _0_blend_saturation = vec4((((_23_blend_set_color_luminance + dst.xyz) - _3_dsa) + src.xyz) - _2_sda, (src.w + dst.w) - _1_alpha);


    }

    sk_FragColor = _0_blend_saturation;

}
