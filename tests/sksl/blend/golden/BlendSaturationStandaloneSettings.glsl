
out vec4 sk_FragColor;
in vec4 src, dst;
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
            vec3 _5_minMidMax = hueLumColor.xzy;
            {
                _4_blend_set_color_saturation_helper = _5_minMidMax.x < _5_minMidMax.z ? vec3(0.0, (sat * (_5_minMidMax.y - _5_minMidMax.x)) / (_5_minMidMax.z - _5_minMidMax.x), sat) : vec3(0.0);
            }
            hueLumColor.xzy = _4_blend_set_color_saturation_helper;

        } else {
            vec3 _6_blend_set_color_saturation_helper;
            vec3 _7_minMidMax = hueLumColor.zxy;
            {
                _6_blend_set_color_saturation_helper = _7_minMidMax.x < _7_minMidMax.z ? vec3(0.0, (sat * (_7_minMidMax.y - _7_minMidMax.x)) / (_7_minMidMax.z - _7_minMidMax.x), sat) : vec3(0.0);
            }
            hueLumColor.zxy = _6_blend_set_color_saturation_helper;

        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        vec3 _8_blend_set_color_saturation_helper;
        vec3 _9_minMidMax = hueLumColor.yxz;
        {
            _8_blend_set_color_saturation_helper = _9_minMidMax.x < _9_minMidMax.z ? vec3(0.0, (sat * (_9_minMidMax.y - _9_minMidMax.x)) / (_9_minMidMax.z - _9_minMidMax.x), sat) : vec3(0.0);
        }
        hueLumColor.yxz = _8_blend_set_color_saturation_helper;

    } else if (hueLumColor.y <= hueLumColor.z) {
        vec3 _10_blend_set_color_saturation_helper;
        vec3 _11_minMidMax = hueLumColor.yzx;
        {
            _10_blend_set_color_saturation_helper = _11_minMidMax.x < _11_minMidMax.z ? vec3(0.0, (sat * (_11_minMidMax.y - _11_minMidMax.x)) / (_11_minMidMax.z - _11_minMidMax.x), sat) : vec3(0.0);
        }
        hueLumColor.yzx = _10_blend_set_color_saturation_helper;

    } else {
        vec3 _12_blend_set_color_saturation_helper;
        vec3 _13_minMidMax = hueLumColor.zyx;
        {
            _12_blend_set_color_saturation_helper = _13_minMidMax.x < _13_minMidMax.z ? vec3(0.0, (sat * (_13_minMidMax.y - _13_minMidMax.x)) / (_13_minMidMax.z - _13_minMidMax.x), sat) : vec3(0.0);
        }
        hueLumColor.zyx = _12_blend_set_color_saturation_helper;

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
    sk_FragColor = blend_saturation(src, dst);
}
