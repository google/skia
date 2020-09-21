
uniform vec4 src, dst;
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
    if (maxComp > alpha && maxComp != lum) {
        return lum + ((result - lum) * (alpha - lum)) / (maxComp - lum);
    }
    return result;
}
vec3 _blend_set_color_saturation_helper(vec3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        return vec3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat);
    }
    return vec3(0.0);
}
float _blend_color_saturation(vec3 color) {
    return max(max(color.x, color.y), color.z) - min(min(color.x, color.y), color.z);
}
vec3 _blend_set_color_saturation(vec3 hueLumColor, vec3 satColor) {
    float _2_blend_color_saturation;
    {
        _2_blend_color_saturation = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    }
    float sat = _2_blend_color_saturation;

    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            hueLumColor.xyz = _blend_set_color_saturation_helper(hueLumColor, sat);
        } else if (hueLumColor.x <= hueLumColor.z) {
            hueLumColor.xzy = _blend_set_color_saturation_helper(hueLumColor.xzy, sat);
        } else {
            hueLumColor.zxy = _blend_set_color_saturation_helper(hueLumColor.zxy, sat);
        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        hueLumColor.yxz = _blend_set_color_saturation_helper(hueLumColor.yxz, sat);
    } else if (hueLumColor.y <= hueLumColor.z) {
        hueLumColor.yzx = _blend_set_color_saturation_helper(hueLumColor.yzx, sat);
    } else {
        hueLumColor.zyx = _blend_set_color_saturation_helper(hueLumColor.zyx, sat);
    }
    return hueLumColor;
}
vec4 blend_saturation(vec4 src, vec4 dst) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    return vec4((((_blend_set_color_luminance(_blend_set_color_saturation(dsa, sda), alpha, dsa) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
vec4 main() {
    return blend_saturation(src, dst);
}
