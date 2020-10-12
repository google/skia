
out vec4 sk_FragColor;
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
vec4 blend_src_in(vec4 src, vec4 dst) {
    return src * dst.w;
}
vec4 blend_dst_in(vec4 src, vec4 dst) {
    return blend_src_in(dst, src);
}
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
vec4 blend_hue(vec4 src, vec4 dst) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    return vec4((((_blend_set_color_luminance(_blend_set_color_saturation(sda, dsa), alpha, dsa) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
void main() {
    sk_FragColor = vec4(fma(color.x, color.y, color.z));
    sk_FragColor *= singleuse();
    sk_FragColor *= blend_src_in(color.xxyy, color.zzww);
    sk_FragColor *= blend_dst_in(color.xxyy, color.zzww);
    sk_FragColor *= blend_hue(color, color.wwww);
    sk_FragColor *= blend_hue(color, color.wzyx);
}
