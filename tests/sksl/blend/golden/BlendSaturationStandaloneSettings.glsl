
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float _blend_color_luminance(vec3 color) {
    return dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), color);
}
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float _0_blend_color_luminance;
    {
        _0_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    }
    float lum = _0_blend_color_luminance;

    float _1_blend_color_luminance;
    {
        _1_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);
    }
    vec3 result = (lum - _1_blend_color_luminance) + hueSatColor;

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
    float _2_blend_color_saturation;
    {
        _2_blend_color_saturation = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    }
    float sat = _2_blend_color_saturation;

    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            vec3 _3_blend_set_color_saturation_helper;
            {
                _3_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.x)) / (hueLumColor.z - hueLumColor.x), sat) : vec3(0.0);
            }
            hueLumColor.xyz = _3_blend_set_color_saturation_helper;

        } else if (hueLumColor.x <= hueLumColor.z) {
            vec3 _4_blend_set_color_saturation_helper;
            {
                _4_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.x)) / (hueLumColor.y - hueLumColor.x), sat) : vec3(0.0);
            }
            hueLumColor.xzy = _4_blend_set_color_saturation_helper;

        } else {
            vec3 _5_blend_set_color_saturation_helper;
            {
                _5_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.z)) / (hueLumColor.y - hueLumColor.z), sat) : vec3(0.0);
            }
            hueLumColor.zxy = _5_blend_set_color_saturation_helper;

        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        vec3 _6_blend_set_color_saturation_helper;
        {
            _6_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.y)) / (hueLumColor.z - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yxz = _6_blend_set_color_saturation_helper;

    } else if (hueLumColor.y <= hueLumColor.z) {
        vec3 _7_blend_set_color_saturation_helper;
        {
            _7_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.y)) / (hueLumColor.x - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yzx = _7_blend_set_color_saturation_helper;

    } else {
        vec3 _8_blend_set_color_saturation_helper;
        {
            _8_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.z)) / (hueLumColor.x - hueLumColor.z), sat) : vec3(0.0);
        }
        hueLumColor.zyx = _8_blend_set_color_saturation_helper;

    }
    return hueLumColor;
}
vec4 blend_saturation(vec4 src, vec4 dst) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    return vec4((((_blend_set_color_luminance(_blend_set_color_saturation(dsa, sda), alpha, dsa) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    vec4 _9_blend_saturation;
    {
        float _10_alpha = dst.w * src.w;
        vec3 _11_sda = src.xyz * dst.w;
        vec3 _12_dsa = dst.xyz * src.w;
        _9_blend_saturation = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_12_dsa, _11_sda), _10_alpha, _12_dsa) + dst.xyz) - _12_dsa) + src.xyz) - _11_sda, (src.w + dst.w) - _10_alpha);
    }
    sk_FragColor = _9_blend_saturation;

}
