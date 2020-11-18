
out vec4 sk_FragColor;
vec4 blend_src_in(vec4 src, vec4 dst) {
    return src * dst.w;
}
vec4 blend_dst_in(vec4 src, vec4 dst) {
    vec4 _0_blend_src_in;
    {
        _0_blend_src_in = dst * src.w;
    }
    return _0_blend_src_in;

}
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
vec4 blend_hue(vec4 src, vec4 dst) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    return vec4((((_blend_set_color_luminance(_blend_set_color_saturation(sda, dsa), alpha, dsa) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
uniform vec4 color;
float singleuse() {
    return 1.25;
}
float add(float a, float b) {
    float c = a + b;
    return c;
}
float mul(float a, float b) {
    return a * b;
}
float fma(float a, float b, float c) {
    return add(mul(a, b), c);
}
void main() {
    sk_FragColor = vec4(fma(color.x, color.y, color.z));
    sk_FragColor *= singleuse();
    sk_FragColor *= blend_src_in(color.xxyy, color.zzww);
    sk_FragColor *= blend_dst_in(color.xxyy, color.zzww);
    sk_FragColor *= blend_hue(color, color.wwww);
    sk_FragColor *= blend_hue(color, color.wzyx);
}
