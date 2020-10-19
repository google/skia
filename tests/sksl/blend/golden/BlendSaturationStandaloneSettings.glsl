
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float _blend_color_luminance(vec3 color) {
    return dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), color);
}
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float lum = _blend_color_luminance(lumColor);
    vec3 result = (lum - _blend_color_luminance(hueSatColor)) + hueSatColor;
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
    float sat = _blend_color_saturation(satColor);
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
void main() {
    vec4 _0_blend_saturation;
    {
        float _1_alpha = dst.w * src.w;
        vec3 _2_sda = src.xyz * dst.w;
        vec3 _3_dsa = dst.xyz * src.w;
        _0_blend_saturation = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_3_dsa, _2_sda), _1_alpha, _3_dsa) + dst.xyz) - _3_dsa) + src.xyz) - _2_sda, (src.w + dst.w) - _1_alpha);
    }

    sk_FragColor = _0_blend_saturation;

}
