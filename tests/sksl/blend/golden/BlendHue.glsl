#version 400
out vec4 sk_FragColor;
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
            float _6_17_blend_color_saturation;
            {
                _6_17_blend_color_saturation = max(max(_3_dsa.x, _3_dsa.y), _3_dsa.z) - min(min(_3_dsa.x, _3_dsa.y), _3_dsa.z);
            }
            float _7_sat = _6_17_blend_color_saturation;

            if (_5_hueLumColor.x <= _5_hueLumColor.y) {
                if (_5_hueLumColor.y <= _5_hueLumColor.z) {
                    vec3 _8_18_blend_set_color_saturation_helper;
                    {
                        _8_18_blend_set_color_saturation_helper = _5_hueLumColor.x < _5_hueLumColor.z ? vec3(0.0, (_7_sat * (_5_hueLumColor.y - _5_hueLumColor.x)) / (_5_hueLumColor.z - _5_hueLumColor.x), _7_sat) : vec3(0.0);
                    }
                    _5_hueLumColor.xyz = _8_18_blend_set_color_saturation_helper;

                } else if (_5_hueLumColor.x <= _5_hueLumColor.z) {
                    vec3 _9_19_blend_set_color_saturation_helper;
                    {
                        _9_19_blend_set_color_saturation_helper = _5_hueLumColor.x < _5_hueLumColor.y ? vec3(0.0, (_7_sat * (_5_hueLumColor.z - _5_hueLumColor.x)) / (_5_hueLumColor.y - _5_hueLumColor.x), _7_sat) : vec3(0.0);
                    }
                    _5_hueLumColor.xzy = _9_19_blend_set_color_saturation_helper;

                } else {
                    vec3 _10_20_blend_set_color_saturation_helper;
                    {
                        _10_20_blend_set_color_saturation_helper = _5_hueLumColor.z < _5_hueLumColor.y ? vec3(0.0, (_7_sat * (_5_hueLumColor.x - _5_hueLumColor.z)) / (_5_hueLumColor.y - _5_hueLumColor.z), _7_sat) : vec3(0.0);
                    }
                    _5_hueLumColor.zxy = _10_20_blend_set_color_saturation_helper;

                }
            } else if (_5_hueLumColor.x <= _5_hueLumColor.z) {
                vec3 _11_21_blend_set_color_saturation_helper;
                {
                    _11_21_blend_set_color_saturation_helper = _5_hueLumColor.y < _5_hueLumColor.z ? vec3(0.0, (_7_sat * (_5_hueLumColor.x - _5_hueLumColor.y)) / (_5_hueLumColor.z - _5_hueLumColor.y), _7_sat) : vec3(0.0);
                }
                _5_hueLumColor.yxz = _11_21_blend_set_color_saturation_helper;

            } else if (_5_hueLumColor.y <= _5_hueLumColor.z) {
                vec3 _12_22_blend_set_color_saturation_helper;
                {
                    _12_22_blend_set_color_saturation_helper = _5_hueLumColor.y < _5_hueLumColor.x ? vec3(0.0, (_7_sat * (_5_hueLumColor.z - _5_hueLumColor.y)) / (_5_hueLumColor.x - _5_hueLumColor.y), _7_sat) : vec3(0.0);
                }
                _5_hueLumColor.yzx = _12_22_blend_set_color_saturation_helper;

            } else {
                vec3 _13_23_blend_set_color_saturation_helper;
                {
                    _13_23_blend_set_color_saturation_helper = _5_hueLumColor.z < _5_hueLumColor.x ? vec3(0.0, (_7_sat * (_5_hueLumColor.y - _5_hueLumColor.z)) / (_5_hueLumColor.x - _5_hueLumColor.z), _7_sat) : vec3(0.0);
                }
                _5_hueLumColor.zyx = _13_23_blend_set_color_saturation_helper;

            }
            _4_blend_set_color_saturation = _5_hueLumColor;
        }
        vec3 _14_blend_set_color_luminance;
        {
            float _15_15_blend_color_luminance;
            {
                _15_15_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);
            }
            float _16_lum = _15_15_blend_color_luminance;

            float _17_16_blend_color_luminance;
            {
                _17_16_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _4_blend_set_color_saturation);
            }
            vec3 _18_result = (_16_lum - _17_16_blend_color_luminance) + _4_blend_set_color_saturation;

            float _19_minComp = min(min(_18_result.x, _18_result.y), _18_result.z);
            float _20_maxComp = max(max(_18_result.x, _18_result.y), _18_result.z);
            if (_19_minComp < 0.0 && _16_lum != _19_minComp) {
                _18_result = _16_lum + ((_18_result - _16_lum) * _16_lum) / (_16_lum - _19_minComp);
            }
            _14_blend_set_color_luminance = _20_maxComp > _1_alpha && _20_maxComp != _16_lum ? _16_lum + ((_18_result - _16_lum) * (_1_alpha - _16_lum)) / (_20_maxComp - _16_lum) : _18_result;
        }
        _0_blend_hue = vec4((((_14_blend_set_color_luminance + dst.xyz) - _3_dsa) + src.xyz) - _2_sda, (src.w + dst.w) - _1_alpha);


    }
    sk_FragColor = _0_blend_hue;

}
