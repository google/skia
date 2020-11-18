
out vec4 sk_FragColor;
uniform vec4 color;
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float _2_blend_color_luminance;
    {
        _2_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    }
    float lum = _2_blend_color_luminance;

    float _3_blend_color_luminance;
    {
        _3_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);
    }
    vec3 result = (lum - _3_blend_color_luminance) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + ((result - lum) * lum) / (lum - minComp);
    }
    return maxComp > alpha && maxComp != lum ? lum + ((result - lum) * (alpha - lum)) / (maxComp - lum) : result;
}
vec3 _blend_set_color_saturation(vec3 hueLumColor, vec3 satColor) {
    float _4_blend_color_saturation;
    {
        _4_blend_color_saturation = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    }
    float sat = _4_blend_color_saturation;

    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            vec3 _5_blend_set_color_saturation_helper;
            {
                _5_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.x)) / (hueLumColor.z - hueLumColor.x), sat) : vec3(0.0);
            }
            hueLumColor.xyz = _5_blend_set_color_saturation_helper;

        } else if (hueLumColor.x <= hueLumColor.z) {
            vec3 _6_blend_set_color_saturation_helper;
            {
                _6_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.x)) / (hueLumColor.y - hueLumColor.x), sat) : vec3(0.0);
            }
            hueLumColor.xzy = _6_blend_set_color_saturation_helper;

        } else {
            vec3 _7_blend_set_color_saturation_helper;
            {
                _7_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.z)) / (hueLumColor.y - hueLumColor.z), sat) : vec3(0.0);
            }
            hueLumColor.zxy = _7_blend_set_color_saturation_helper;

        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        vec3 _8_blend_set_color_saturation_helper;
        {
            _8_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.y)) / (hueLumColor.z - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yxz = _8_blend_set_color_saturation_helper;

    } else if (hueLumColor.y <= hueLumColor.z) {
        vec3 _9_blend_set_color_saturation_helper;
        {
            _9_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.y)) / (hueLumColor.x - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yzx = _9_blend_set_color_saturation_helper;

    } else {
        vec3 _10_blend_set_color_saturation_helper;
        {
            _10_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.z)) / (hueLumColor.x - hueLumColor.z), sat) : vec3(0.0);
        }
        hueLumColor.zyx = _10_blend_set_color_saturation_helper;

    }
    return hueLumColor;
}
void main() {
    float _14_fma;
    {
        float _15_0_mul;
        {
            _15_0_mul = color.x * color.y;
        }
        float _27_add;
        {
            float _28_c = _15_0_mul + color.z;
            _27_add = _28_c;
        }
        _14_fma = _27_add;


    }
    sk_FragColor = vec4(_14_fma);

    sk_FragColor *= 1.25;

    vec4 _17_blend_src_in;
    {
        _17_blend_src_in = color.xxyy * color.w;
    }
    sk_FragColor *= _17_blend_src_in;

    vec4 _18_blend_dst_in;
    {
        vec4 _29_blend_src_in;
        {
            _29_blend_src_in = color.zzww * color.y;
        }
        _18_blend_dst_in = _29_blend_src_in;

    }
    sk_FragColor *= _18_blend_dst_in;

    vec4 _19_blend_hue;
    {
        float _20_alpha = color.w * color.w;
        vec3 _21_sda = color.xyz * color.w;
        vec3 _22_dsa = color.www * color.w;
        _19_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_21_sda, _22_dsa), _20_alpha, _22_dsa) + color.www) - _22_dsa) + color.xyz) - _21_sda, (color.w + color.w) - _20_alpha);
    }
    sk_FragColor *= _19_blend_hue;

    vec4 _23_blend_hue;
    {
        float _24_alpha = color.x * color.w;
        vec3 _25_sda = color.xyz * color.x;
        vec3 _26_dsa = color.wzy * color.w;
        _23_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_25_sda, _26_dsa), _24_alpha, _26_dsa) + color.wzy) - _26_dsa) + color.xyz) - _25_sda, (color.w + color.x) - _24_alpha);
    }
    sk_FragColor *= _23_blend_hue;

}
