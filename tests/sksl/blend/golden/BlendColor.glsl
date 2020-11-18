#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _7_blend_color;
    {
        float _8_alpha = dst.w * src.w;
        vec3 _9_sda = src.xyz * dst.w;
        vec3 _10_dsa = dst.xyz * src.w;
        vec3 _11_blend_set_color_luminance;
        {
            float _16_blend_color_luminance;
            {
                _16_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _10_dsa);
            }
            float _12_lum = _16_blend_color_luminance;

            float _17_blend_color_luminance;
            {
                _17_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), _9_sda);
            }
            vec3 _13_result = (_12_lum - _17_blend_color_luminance) + _9_sda;

            float _14_minComp = min(min(_13_result.x, _13_result.y), _13_result.z);
            float _15_maxComp = max(max(_13_result.x, _13_result.y), _13_result.z);
            if (_14_minComp < 0.0 && _12_lum != _14_minComp) {
                _13_result = _12_lum + ((_13_result - _12_lum) * _12_lum) / (_12_lum - _14_minComp);
            }
            _11_blend_set_color_luminance = _15_maxComp > _8_alpha && _15_maxComp != _12_lum ? _12_lum + ((_13_result - _12_lum) * (_8_alpha - _12_lum)) / (_15_maxComp - _12_lum) : _13_result;
        }
        _7_blend_color = vec4((((_11_blend_set_color_luminance + dst.xyz) - _10_dsa) + src.xyz) - _9_sda, (src.w + dst.w) - _8_alpha);

    }
    sk_FragColor = _7_blend_color;

}
