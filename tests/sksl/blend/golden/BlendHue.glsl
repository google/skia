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
            float _7_blend_color_saturation;
            {
                _7_blend_color_saturation = max(max(_3_dsa.x, _3_dsa.y), _3_dsa.z) - min(min(_3_dsa.x, _3_dsa.y), _3_dsa.z);
            }
            float _6_sat = _7_blend_color_saturation;

            if (_5_hueLumColor.x <= _5_hueLumColor.y) {
                if (_5_hueLumColor.y <= _5_hueLumColor.z) {
                    vec3 _8_blend_set_color_saturation_helper;
                    {
                        _8_blend_set_color_saturation_helper = _5_hueLumColor.x < _5_hueLumColor.z ? vec3(0.0, (_6_sat * (_5_hueLumColor.y - _5_hueLumColor.x)) / (_5_hueLumColor.z - _5_hueLumColor.x), _6_sat) : vec3(0.0);
                    }
                    _5_hueLumColor.xyz = _8_blend_set_color_saturation_helper;

                } else if (_5_hueLumColor.x <= _5_hueLumColor.z) {
                    vec3 _9_blend_set_color_saturation_helper;
                    {
                        _9_blend_set_color_saturation_helper = _5_hueLumColor.x < _5_hueLumColor.y ? vec3(0.0, (_6_sat * (_5_hueLumColor.z - _5_hueLumColor.x)) / (_5_hueLumColor.y - _5_hueLumColor.x), _6_sat) : vec3(0.0);
                    }
                    _5_hueLumColor.xzy = _9_blend_set_color_saturation_helper;

                } else {
                    vec3 _10_blend_set_color_saturation_helper;
                    {
                        _10_blend_set_color_saturation_helper = _5_hueLumColor.z < _5_hueLumColor.y ? vec3(0.0, (_6_sat * (_5_hueLumColor.x - _5_hueLumColor.z)) / (_5_hueLumColor.y - _5_hueLumColor.z), _6_sat) : vec3(0.0);
                    }
                    _5_hueLumColor.zxy = _10_blend_set_color_saturation_helper;

                }
            } else if (_5_hueLumColor.x <= _5_hueLumColor.z) {
                vec3 _11_blend_set_color_saturation_helper;
                {
                    _11_blend_set_color_saturation_helper = _5_hueLumColor.y < _5_hueLumColor.z ? vec3(0.0, (_6_sat * (_5_hueLumColor.x - _5_hueLumColor.y)) / (_5_hueLumColor.z - _5_hueLumColor.y), _6_sat) : vec3(0.0);
                }
                _5_hueLumColor.yxz = _11_blend_set_color_saturation_helper;

            } else if (_5_hueLumColor.y <= _5_hueLumColor.z) {
                vec3 _12_blend_set_color_saturation_helper;
                {
                    _12_blend_set_color_saturation_helper = _5_hueLumColor.y < _5_hueLumColor.x ? vec3(0.0, (_6_sat * (_5_hueLumColor.z - _5_hueLumColor.y)) / (_5_hueLumColor.x - _5_hueLumColor.y), _6_sat) : vec3(0.0);
                }
                _5_hueLumColor.yzx = _12_blend_set_color_saturation_helper;

            } else {
                vec3 _13_blend_set_color_saturation_helper;
                {
                    _13_blend_set_color_saturation_helper = _5_hueLumColor.z < _5_hueLumColor.x ? vec3(0.0, (_6_sat * (_5_hueLumColor.y - _5_hueLumColor.z)) / (_5_hueLumColor.x - _5_hueLumColor.z), _6_sat) : vec3(0.0);
                }
                _5_hueLumColor.zyx = _13_blend_set_color_saturation_helper;

            }
            _4_blend_set_color_saturation = _5_hueLumColor;
        }
        vec3 _14_blend_set_color_luminance;
        {
            float _19_blend_color_luminance;
            {
                _19_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);
            }
            float _15_lum = _19_blend_color_luminance;

            float _20_blend_color_luminance;
            {
                _20_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _4_blend_set_color_saturation);
            }
            vec3 _16_result = (_15_lum - _20_blend_color_luminance) + _4_blend_set_color_saturation;

            float _17_minComp = min(min(_16_result.x, _16_result.y), _16_result.z);
            float _18_maxComp = max(max(_16_result.x, _16_result.y), _16_result.z);
            if (_17_minComp < 0.0 && _15_lum != _17_minComp) {
                _16_result = _15_lum + ((_16_result - _15_lum) * _15_lum) / (_15_lum - _17_minComp);
            }
            _14_blend_set_color_luminance = _18_maxComp > _1_alpha && _18_maxComp != _15_lum ? _15_lum + ((_16_result - _15_lum) * (_1_alpha - _15_lum)) / (_18_maxComp - _15_lum) : _16_result;
        }
        _0_blend_hue = vec4((((_14_blend_set_color_luminance + dst.xyz) - _3_dsa) + src.xyz) - _2_sda, (src.w + dst.w) - _1_alpha);


    }

    sk_FragColor = _0_blend_hue;

}
