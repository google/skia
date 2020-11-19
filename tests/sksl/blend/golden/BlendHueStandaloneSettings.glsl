
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_hue;
    {
        float _1_alpha = dst.w * src.w;
        vec3 _2_sda = src.xyz * dst.w;
        vec3 _3_dsa = dst.xyz * src.w;
        vec3 _13_blend_set_color_saturation;
        {
            float _15_blend_color_saturation;
            {
                _15_blend_color_saturation = max(max(_3_dsa.x, _3_dsa.y), _3_dsa.z) - min(min(_3_dsa.x, _3_dsa.y), _3_dsa.z);
            }
            float _14_sat = _15_blend_color_saturation;

            if (_2_sda.x <= _2_sda.y) {
                if (_2_sda.y <= _2_sda.z) {
                    vec3 _16_blend_set_color_saturation_helper;
                    {
                        _16_blend_set_color_saturation_helper = _2_sda.x < _2_sda.z ? vec3(0.0, (_14_sat * (_2_sda.y - _2_sda.x)) / (_2_sda.z - _2_sda.x), _14_sat) : vec3(0.0);
                    }
                    _13_blend_set_color_saturation = _16_blend_set_color_saturation_helper;

                } else if (_2_sda.x <= _2_sda.z) {
                    vec3 _17_blend_set_color_saturation_helper;
                    {
                        _17_blend_set_color_saturation_helper = _2_sda.x < _2_sda.y ? vec3(0.0, (_14_sat * (_2_sda.z - _2_sda.x)) / (_2_sda.y - _2_sda.x), _14_sat) : vec3(0.0);
                    }
                    _13_blend_set_color_saturation = _17_blend_set_color_saturation_helper.xzy;

                } else {
                    vec3 _18_blend_set_color_saturation_helper;
                    {
                        _18_blend_set_color_saturation_helper = _2_sda.z < _2_sda.y ? vec3(0.0, (_14_sat * (_2_sda.x - _2_sda.z)) / (_2_sda.y - _2_sda.z), _14_sat) : vec3(0.0);
                    }
                    _13_blend_set_color_saturation = _18_blend_set_color_saturation_helper.yzx;

                }
            } else if (_2_sda.x <= _2_sda.z) {
                vec3 _19_blend_set_color_saturation_helper;
                {
                    _19_blend_set_color_saturation_helper = _2_sda.y < _2_sda.z ? vec3(0.0, (_14_sat * (_2_sda.x - _2_sda.y)) / (_2_sda.z - _2_sda.y), _14_sat) : vec3(0.0);
                }
                _13_blend_set_color_saturation = _19_blend_set_color_saturation_helper.yxz;

            } else if (_2_sda.y <= _2_sda.z) {
                vec3 _20_blend_set_color_saturation_helper;
                {
                    _20_blend_set_color_saturation_helper = _2_sda.y < _2_sda.x ? vec3(0.0, (_14_sat * (_2_sda.z - _2_sda.y)) / (_2_sda.x - _2_sda.y), _14_sat) : vec3(0.0);
                }
                _13_blend_set_color_saturation = _20_blend_set_color_saturation_helper.zxy;

            } else {
                vec3 _21_blend_set_color_saturation_helper;
                {
                    _21_blend_set_color_saturation_helper = _2_sda.z < _2_sda.x ? vec3(0.0, (_14_sat * (_2_sda.y - _2_sda.z)) / (_2_sda.x - _2_sda.z), _14_sat) : vec3(0.0);
                }
                _13_blend_set_color_saturation = _21_blend_set_color_saturation_helper.zyx;

            }
        }
        vec3 _22_blend_set_color_luminance;
        {
            float _27_blend_color_luminance;
            {
                _27_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _3_dsa);
            }
            float _23_lum = _27_blend_color_luminance;

            float _28_blend_color_luminance;
            {
                _28_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _13_blend_set_color_saturation);
            }
            vec3 _24_result = (_23_lum - _28_blend_color_luminance) + _13_blend_set_color_saturation;

            float _25_minComp = min(min(_24_result.x, _24_result.y), _24_result.z);
            float _26_maxComp = max(max(_24_result.x, _24_result.y), _24_result.z);
            if (_25_minComp < 0.0 && _23_lum != _25_minComp) {
                _24_result = _23_lum + ((_24_result - _23_lum) * _23_lum) / (_23_lum - _25_minComp);
            }
            _22_blend_set_color_luminance = _26_maxComp > _1_alpha && _26_maxComp != _23_lum ? _23_lum + ((_24_result - _23_lum) * (_1_alpha - _23_lum)) / (_26_maxComp - _23_lum) : _24_result;
        }
        _0_blend_hue = vec4((((_22_blend_set_color_luminance + dst.xyz) - _3_dsa) + src.xyz) - _2_sda, (src.w + dst.w) - _1_alpha);


    }

    sk_FragColor = _0_blend_hue;

}
