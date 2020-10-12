
out vec4 sk_FragColor;
uniform vec4 color;
vec4 blend_src_in(vec4 src, vec4 dst) {
    return src * dst.w;
}
vec4 blend_dst_in(vec4 src, vec4 dst) {
    vec4 _1_blend_src_in;
    {
        _1_blend_src_in = dst * src.w;
    }
    return _1_blend_src_in;

}
float _blend_color_luminance(vec3 color) {
    return dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), color);
}
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
float _blend_color_saturation(vec3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
vec3 _blend_set_color_saturation_helper(vec3 minMidMax, float sat) {
    return minMidMax.x < minMidMax.z ? vec3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat) : vec3(0.0);
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
vec4 blend_hue(vec4 src, vec4 dst) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    return vec4((((_blend_set_color_luminance(_blend_set_color_saturation(sda, dsa), alpha, dsa) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    float _11_fma;
    {
        float _12_0_mul;
        {
            _12_0_mul = color.x * color.y;
        }
        float _24_add;
        {
            float _25_c = _12_0_mul + color.z;
            _24_add = _25_c;
        }
        _11_fma = _24_add;


    }
    sk_FragColor = vec4(_11_fma);

    sk_FragColor *= 1.25;

    vec4 _14_blend_src_in;
    {
        _14_blend_src_in = color.xxyy * color.w;
    }
    sk_FragColor *= _14_blend_src_in;

    vec4 _15_blend_dst_in;
    {
        vec4 _26_blend_src_in;
        {
            _26_blend_src_in = color.zzww * color.y;
        }
        _15_blend_dst_in = _26_blend_src_in;

    }
    sk_FragColor *= _15_blend_dst_in;

    vec4 _16_blend_hue;
    {
        float _17_alpha = color.w * color.w;
        vec3 _18_sda = color.xyz * color.w;
        vec3 _19_dsa = color.www * color.w;
        _16_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_18_sda, _19_dsa), _17_alpha, _19_dsa) + color.www) - _19_dsa) + color.xyz) - _18_sda, (color.w + color.w) - _17_alpha);
    }
    sk_FragColor *= _16_blend_hue;

    vec4 _20_blend_hue;
    {
        float _21_alpha = color.x * color.w;
        vec3 _22_sda = color.xyz * color.x;
        vec3 _23_dsa = color.wzy * color.w;
        _20_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_22_sda, _23_dsa), _21_alpha, _23_dsa) + color.wzy) - _23_dsa) + color.xyz) - _22_sda, (color.w + color.x) - _21_alpha);
    }
    sk_FragColor *= _20_blend_hue;

}
