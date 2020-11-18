
out vec4 sk_FragColor;
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float _15_blend_color_luminance;
    {
        _15_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    }
    float lum = _15_blend_color_luminance;

    float _16_blend_color_luminance;
    {
        _16_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);
    }
    vec3 result = (lum - _16_blend_color_luminance) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + ((result - lum) * lum) / (lum - minComp);
    }
    return maxComp > alpha && maxComp != lum ? lum + ((result - lum) * (alpha - lum)) / (maxComp - lum) : result;
}
vec3 _blend_set_color_saturation(vec3 hueLumColor, vec3 satColor) {
    float _17_blend_color_saturation;
    {
        _17_blend_color_saturation = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    }
    float sat = _17_blend_color_saturation;

    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            vec3 _18_blend_set_color_saturation_helper;
            {
                _18_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.x)) / (hueLumColor.z - hueLumColor.x), sat) : vec3(0.0);
            }
            return _18_blend_set_color_saturation_helper;

        } else if (hueLumColor.x <= hueLumColor.z) {
            vec3 _19_blend_set_color_saturation_helper;
            {
                _19_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.x)) / (hueLumColor.y - hueLumColor.x), sat) : vec3(0.0);
            }
            return _19_blend_set_color_saturation_helper.xzy;

        } else {
            vec3 _20_blend_set_color_saturation_helper;
            {
                _20_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.z)) / (hueLumColor.y - hueLumColor.z), sat) : vec3(0.0);
            }
            return _20_blend_set_color_saturation_helper.yzx;

        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        vec3 _21_blend_set_color_saturation_helper;
        {
            _21_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.y)) / (hueLumColor.z - hueLumColor.y), sat) : vec3(0.0);
        }
        return _21_blend_set_color_saturation_helper.yxz;

    } else if (hueLumColor.y <= hueLumColor.z) {
        vec3 _22_blend_set_color_saturation_helper;
        {
            _22_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.y)) / (hueLumColor.x - hueLumColor.y), sat) : vec3(0.0);
        }
        return _22_blend_set_color_saturation_helper.zxy;

    } else {
        vec3 _23_blend_set_color_saturation_helper;
        {
            _23_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.z)) / (hueLumColor.x - hueLumColor.z), sat) : vec3(0.0);
        }
        return _23_blend_set_color_saturation_helper.zyx;

    }
}
uniform vec4 color;
void main() {
    float _3_fma;
    {
        float _4_0_mul;
        {
            _4_0_mul = color.x * color.y;
        }

        float _5_1_add;
        {
            float _6_2_c = _4_0_mul + color.z;
            _5_1_add = _6_2_c;
        }

        _3_fma = _5_1_add;

    }

    sk_FragColor = vec4(_3_fma);

    sk_FragColor *= 1.25;

    vec4 _8_blend_src_in;
    {
        _8_blend_src_in = color.xxyy * color.w;
    }

    sk_FragColor *= _8_blend_src_in;

    vec4 _9_blend_dst_in;
    {
        vec4 _10_0_blend_src_in;
        {
            _10_0_blend_src_in = color.zzww * color.y;
        }
        _9_blend_dst_in = _10_0_blend_src_in;

    }

    sk_FragColor *= _9_blend_dst_in;

    vec4 _11_blend_hue;
    {
        float _12_alpha = color.w * color.w;
        vec3 _13_sda = color.xyz * color.w;
        vec3 _14_dsa = color.www * color.w;
        _11_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_13_sda, _14_dsa), _12_alpha, _14_dsa) + color.www) - _14_dsa) + color.xyz) - _13_sda, (color.w + color.w) - _12_alpha);
    }

    sk_FragColor *= _11_blend_hue;

    vec4 _15_blend_hue;
    {
        float _16_alpha = color.x * color.w;
        vec3 _17_sda = color.xyz * color.x;
        vec3 _18_dsa = color.wzy * color.w;
        _15_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_17_sda, _18_dsa), _16_alpha, _18_dsa) + color.wzy) - _18_dsa) + color.xyz) - _17_sda, (color.w + color.x) - _16_alpha);
    }

    sk_FragColor *= _15_blend_hue;

}
