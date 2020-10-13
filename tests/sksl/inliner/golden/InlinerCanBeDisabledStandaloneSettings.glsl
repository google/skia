
out vec4 sk_FragColor;
uniform vec4 color;
vec4 blend_src_in(vec4 src, vec4 dst) {
    return src * dst.w;
}
vec4 blend_dst_in(vec4 src, vec4 dst) {
    vec4 _18_blend_src_in;
    {
        _18_blend_src_in = dst * src.w;
    }
    return _18_blend_src_in;

}
float _blend_color_luminance(vec3 color) {
    return dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), color);
}
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float _19_blend_color_luminance;
    {
        _19_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    }
    float lum = _19_blend_color_luminance;

    float _20_blend_color_luminance;
    {
        _20_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);
    }
    vec3 result = (lum - _20_blend_color_luminance) + hueSatColor;

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
    float _21_blend_color_saturation;
    {
        _21_blend_color_saturation = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    }
    float sat = _21_blend_color_saturation;

    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            vec3 _22_blend_set_color_saturation_helper;
            {
                _22_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.x)) / (hueLumColor.z - hueLumColor.x), sat) : vec3(0.0);
            }
            hueLumColor.xyz = _22_blend_set_color_saturation_helper;

        } else if (hueLumColor.x <= hueLumColor.z) {
            vec3 _23_blend_set_color_saturation_helper;
            {
                _23_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.x)) / (hueLumColor.y - hueLumColor.x), sat) : vec3(0.0);
            }
            hueLumColor.xzy = _23_blend_set_color_saturation_helper;

        } else {
            vec3 _24_blend_set_color_saturation_helper;
            {
                _24_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.z)) / (hueLumColor.y - hueLumColor.z), sat) : vec3(0.0);
            }
            hueLumColor.zxy = _24_blend_set_color_saturation_helper;

        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        vec3 _25_blend_set_color_saturation_helper;
        {
            _25_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.y)) / (hueLumColor.z - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yxz = _25_blend_set_color_saturation_helper;

    } else if (hueLumColor.y <= hueLumColor.z) {
        vec3 _26_blend_set_color_saturation_helper;
        {
            _26_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.y)) / (hueLumColor.x - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yzx = _26_blend_set_color_saturation_helper;

    } else {
        vec3 _27_blend_set_color_saturation_helper;
        {
            _27_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.z)) / (hueLumColor.x - hueLumColor.z), sat) : vec3(0.0);
        }
        hueLumColor.zyx = _27_blend_set_color_saturation_helper;

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
        vec4 _28_blend_src_in;
        {
            _28_blend_src_in = color.zzww * color.y;
        }
        _9_blend_dst_in = _28_blend_src_in;

    }

    sk_FragColor *= _9_blend_dst_in;

    vec4 _10_blend_hue;
    {
        float _11_alpha = color.w * color.w;
        vec3 _12_sda = color.xyz * color.w;
        vec3 _13_dsa = color.www * color.w;
        _10_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_12_sda, _13_dsa), _11_alpha, _13_dsa) + color.www) - _13_dsa) + color.xyz) - _12_sda, (color.w + color.w) - _11_alpha);
    }

    sk_FragColor *= _10_blend_hue;

    vec4 _14_blend_hue;
    {
        float _15_alpha = color.x * color.w;
        vec3 _16_sda = color.xyz * color.x;
        vec3 _17_dsa = color.wzy * color.w;
        _14_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_16_sda, _17_dsa), _15_alpha, _17_dsa) + color.wzy) - _17_dsa) + color.xyz) - _16_sda, (color.w + color.x) - _15_alpha);
    }

    sk_FragColor *= _14_blend_hue;

}
