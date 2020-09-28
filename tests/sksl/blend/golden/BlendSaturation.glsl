#version 400
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _17_blend_saturation;
    {
        float _18_alpha = dst.w * src.w;
        vec3 _19_sda = src.xyz * dst.w;
        vec3 _20_dsa = dst.xyz * src.w;
        vec3 _21_blend_set_color_saturation;
        vec3 _22_hueLumColor = _20_dsa;
        {
            float _24_blend_color_saturation;
            {
                _24_blend_color_saturation = max(max(_19_sda.x, _19_sda.y), _19_sda.z) - min(min(_19_sda.x, _19_sda.y), _19_sda.z);
            }
            float _23_sat = _24_blend_color_saturation;

            if (_22_hueLumColor.x <= _22_hueLumColor.y) {
                if (_22_hueLumColor.y <= _22_hueLumColor.z) {
                    vec3 _25_blend_set_color_saturation_helper;
                    {
                        _25_blend_set_color_saturation_helper = _22_hueLumColor.x < _22_hueLumColor.z ? vec3(0.0, (_23_sat * (_22_hueLumColor.y - _22_hueLumColor.x)) / (_22_hueLumColor.z - _22_hueLumColor.x), _23_sat) : vec3(0.0);
                    }
                    _22_hueLumColor.xyz = _25_blend_set_color_saturation_helper;

                } else if (_22_hueLumColor.x <= _22_hueLumColor.z) {
                    vec3 _26_blend_set_color_saturation_helper;
                    vec3 _27_minMidMax = _22_hueLumColor.xzy;
                    {
                        _26_blend_set_color_saturation_helper = _27_minMidMax.x < _27_minMidMax.z ? vec3(0.0, (_23_sat * (_27_minMidMax.y - _27_minMidMax.x)) / (_27_minMidMax.z - _27_minMidMax.x), _23_sat) : vec3(0.0);
                    }
                    _22_hueLumColor.xzy = _26_blend_set_color_saturation_helper;

                } else {
                    vec3 _28_blend_set_color_saturation_helper;
                    vec3 _29_minMidMax = _22_hueLumColor.zxy;
                    {
                        _28_blend_set_color_saturation_helper = _29_minMidMax.x < _29_minMidMax.z ? vec3(0.0, (_23_sat * (_29_minMidMax.y - _29_minMidMax.x)) / (_29_minMidMax.z - _29_minMidMax.x), _23_sat) : vec3(0.0);
                    }
                    _22_hueLumColor.zxy = _28_blend_set_color_saturation_helper;

                }
            } else if (_22_hueLumColor.x <= _22_hueLumColor.z) {
                vec3 _30_blend_set_color_saturation_helper;
                vec3 _31_minMidMax = _22_hueLumColor.yxz;
                {
                    _30_blend_set_color_saturation_helper = _31_minMidMax.x < _31_minMidMax.z ? vec3(0.0, (_23_sat * (_31_minMidMax.y - _31_minMidMax.x)) / (_31_minMidMax.z - _31_minMidMax.x), _23_sat) : vec3(0.0);
                }
                _22_hueLumColor.yxz = _30_blend_set_color_saturation_helper;

            } else if (_22_hueLumColor.y <= _22_hueLumColor.z) {
                vec3 _32_blend_set_color_saturation_helper;
                vec3 _33_minMidMax = _22_hueLumColor.yzx;
                {
                    _32_blend_set_color_saturation_helper = _33_minMidMax.x < _33_minMidMax.z ? vec3(0.0, (_23_sat * (_33_minMidMax.y - _33_minMidMax.x)) / (_33_minMidMax.z - _33_minMidMax.x), _23_sat) : vec3(0.0);
                }
                _22_hueLumColor.yzx = _32_blend_set_color_saturation_helper;

            } else {
                vec3 _34_blend_set_color_saturation_helper;
                vec3 _35_minMidMax = _22_hueLumColor.zyx;
                {
                    _34_blend_set_color_saturation_helper = _35_minMidMax.x < _35_minMidMax.z ? vec3(0.0, (_23_sat * (_35_minMidMax.y - _35_minMidMax.x)) / (_35_minMidMax.z - _35_minMidMax.x), _23_sat) : vec3(0.0);
                }
                _22_hueLumColor.zyx = _34_blend_set_color_saturation_helper;

            }
            _21_blend_set_color_saturation = _22_hueLumColor;
        }
        vec3 _36_blend_set_color_luminance;
        {
            float _41_blend_color_luminance;
            {
                _41_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _20_dsa);
            }
            float _37_lum = _41_blend_color_luminance;

            float _42_blend_color_luminance;
            {
                _42_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _21_blend_set_color_saturation);
            }
            vec3 _38_result = (_37_lum - _42_blend_color_luminance) + _21_blend_set_color_saturation;

            float _39_minComp = min(min(_38_result.x, _38_result.y), _38_result.z);
            float _40_maxComp = max(max(_38_result.x, _38_result.y), _38_result.z);
            if (_39_minComp < 0.0 && _37_lum != _39_minComp) {
                _38_result = _37_lum + ((_38_result - _37_lum) * _37_lum) / (_37_lum - _39_minComp);
            }
            _36_blend_set_color_luminance = _40_maxComp > _18_alpha && _40_maxComp != _37_lum ? _37_lum + ((_38_result - _37_lum) * (_18_alpha - _37_lum)) / (_40_maxComp - _37_lum) : _38_result;
        }
        _17_blend_saturation = vec4((((_36_blend_set_color_luminance + dst.xyz) - _20_dsa) + src.xyz) - _19_sda, (src.w + dst.w) - _18_alpha);


    }
    sk_FragColor = _17_blend_saturation;

}
