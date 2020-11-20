
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
        {
            float _5_17_blend_color_saturation;
            {
                _5_17_blend_color_saturation = max(max(_3_dsa.x, _3_dsa.y), _3_dsa.z) - min(min(_3_dsa.x, _3_dsa.y), _3_dsa.z);
            }
            float _6_sat = _5_17_blend_color_saturation;

            if (_2_sda.x <= _2_sda.y) {
                if (_2_sda.y <= _2_sda.z) {
                    vec3 _7_18_blend_set_color_saturation_helper;
                    {
                        _7_18_blend_set_color_saturation_helper = _2_sda.x < _2_sda.z ? vec3(0.0, (_6_sat * (_2_sda.y - _2_sda.x)) / (_2_sda.z - _2_sda.x), _6_sat) : vec3(0.0);
                    }
                    _4_blend_set_color_saturation = _7_18_blend_set_color_saturation_helper;

                } else if (_2_sda.x <= _2_sda.z) {
                    vec3 _8_19_blend_set_color_saturation_helper;
                    {
                        _8_19_blend_set_color_saturation_helper = _2_sda.x < _2_sda.y ? vec3(0.0, (_6_sat * (_2_sda.z - _2_sda.x)) / (_2_sda.y - _2_sda.x), _6_sat) : vec3(0.0);
                    }
                    _4_blend_set_color_saturation = _8_19_blend_set_color_saturation_helper.xzy;

                } else {
                    vec3 _9_20_blend_set_color_saturation_helper;
                    {
                        _9_20_blend_set_color_saturation_helper = _2_sda.z < _2_sda.y ? vec3(0.0, (_6_sat * (_2_sda.x - _2_sda.z)) / (_2_sda.y - _2_sda.z), _6_sat) : vec3(0.0);
                    }
                    _4_blend_set_color_saturation = _9_20_blend_set_color_saturation_helper.yzx;

                }
            } else if (_2_sda.x <= _2_sda.z) {
                vec3 _10_21_blend_set_color_saturation_helper;
                {
                    _10_21_blend_set_color_saturation_helper = _2_sda.y < _2_sda.z ? vec3(0.0, (_6_sat * (_2_sda.x - _2_sda.y)) / (_2_sda.z - _2_sda.y), _6_sat) : vec3(0.0);
                }
                _4_blend_set_color_saturation = _10_21_blend_set_color_saturation_helper.yxz;

            } else if (_2_sda.y <= _2_sda.z) {
                vec3 _11_22_blend_set_color_saturation_helper;
                {
                    _11_22_blend_set_color_saturation_helper = _2_sda.y < _2_sda.x ? vec3(0.0, (_6_sat * (_2_sda.z - _2_sda.y)) / (_2_sda.x - _2_sda.y), _6_sat) : vec3(0.0);
                }
                _4_blend_set_color_saturation = _11_22_blend_set_color_saturation_helper.zxy;

            } else {
                vec3 _12_23_blend_set_color_saturation_helper;
                {
                    _12_23_blend_set_color_saturation_helper = _2_sda.z < _2_sda.x ? vec3(0.0, (_6_sat * (_2_sda.y - _2_sda.z)) / (_2_sda.x - _2_sda.z), _6_sat) : vec3(0.0);
                }
                _4_blend_set_color_saturation = _12_23_blend_set_color_saturation_helper.zyx;

            }
        }
        vec3 _13_blend_set_color_luminance;
        {
            float _14_15_blend_color_luminance;
            {
                _14_15_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);
            }
            float _15_lum = _14_15_blend_color_luminance;

            float _16_16_blend_color_luminance;
            {
                _16_16_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _4_blend_set_color_saturation);
            }
            vec3 _17_result = (_15_lum - _16_16_blend_color_luminance) + _4_blend_set_color_saturation;

            float _18_minComp = min(min(_17_result.x, _17_result.y), _17_result.z);
            float _19_maxComp = max(max(_17_result.x, _17_result.y), _17_result.z);
            if (_18_minComp < 0.0 && _15_lum != _18_minComp) {
                _17_result = _15_lum + ((_17_result - _15_lum) * _15_lum) / (_15_lum - _18_minComp);
            }
            _13_blend_set_color_luminance = _19_maxComp > _1_alpha && _19_maxComp != _15_lum ? _15_lum + ((_17_result - _15_lum) * (_1_alpha - _15_lum)) / (_19_maxComp - _15_lum) : _17_result;
        }
        _0_blend_hue = vec4((((_13_blend_set_color_luminance + dst.xyz) - _3_dsa) + src.xyz) - _2_sda, (src.w + dst.w) - _1_alpha);


    }
    sk_FragColor = _0_blend_hue;

}
