
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
            hueLumColor.xyz = _18_blend_set_color_saturation_helper;

        } else if (hueLumColor.x <= hueLumColor.z) {
            vec3 _19_blend_set_color_saturation_helper;
            {
                _19_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.x)) / (hueLumColor.y - hueLumColor.x), sat) : vec3(0.0);
            }
            hueLumColor.xzy = _19_blend_set_color_saturation_helper;

        } else {
            vec3 _20_blend_set_color_saturation_helper;
            {
                _20_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.z)) / (hueLumColor.y - hueLumColor.z), sat) : vec3(0.0);
            }
            hueLumColor.zxy = _20_blend_set_color_saturation_helper;

        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        vec3 _21_blend_set_color_saturation_helper;
        {
            _21_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.y)) / (hueLumColor.z - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yxz = _21_blend_set_color_saturation_helper;

    } else if (hueLumColor.y <= hueLumColor.z) {
        vec3 _22_blend_set_color_saturation_helper;
        {
            _22_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.y)) / (hueLumColor.x - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yzx = _22_blend_set_color_saturation_helper;

    } else {
        vec3 _23_blend_set_color_saturation_helper;
        {
            _23_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.z)) / (hueLumColor.x - hueLumColor.z), sat) : vec3(0.0);
        }
        hueLumColor.zyx = _23_blend_set_color_saturation_helper;

    }
    return hueLumColor;
}
uniform vec4 color;
void main() {
    float _1_fma;
    {
        float _2_0_mul;
        {
            _2_0_mul = color.x * color.y;
        }
        float _15_add;
        {
            float _16_c = _2_0_mul + color.z;
            _15_add = _16_c;
        }
        _1_fma = _15_add;


    }
    sk_FragColor = vec4(_1_fma);

    sk_FragColor *= 1.25;

    vec4 _4_blend_src_in;
    {
        _4_blend_src_in = color.xxyy * color.w;
    }
    sk_FragColor *= _4_blend_src_in;

    vec4 _5_blend_dst_in;
    {
        vec4 _6_0_blend_src_in;
        {
            _6_0_blend_src_in = color.zzww * color.y;
        }
        _5_blend_dst_in = _6_0_blend_src_in;

    }
    sk_FragColor *= _5_blend_dst_in;

    vec4 _7_blend_hue;
    {
        float _8_alpha = color.w * color.w;
        vec3 _9_sda = color.xyz * color.w;
        vec3 _10_dsa = color.www * color.w;
        _7_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_9_sda, _10_dsa), _8_alpha, _10_dsa) + color.www) - _10_dsa) + color.xyz) - _9_sda, (color.w + color.w) - _8_alpha);
    }
    sk_FragColor *= _7_blend_hue;

    vec4 _11_blend_hue;
    {
        float _12_alpha = color.x * color.w;
        vec3 _13_sda = color.xyz * color.x;
        vec3 _14_dsa = color.wzy * color.w;
        _11_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_13_sda, _14_dsa), _12_alpha, _14_dsa) + color.wzy) - _14_dsa) + color.xyz) - _13_sda, (color.w + color.x) - _12_alpha);
    }
    sk_FragColor *= _11_blend_hue;

}
