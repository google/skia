
out vec4 sk_FragColor;
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float _11_blend_color_luminance;
    _11_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);

    float lum = _11_blend_color_luminance;

    float _12_blend_color_luminance;
    _12_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);

    vec3 result = (lum - _12_blend_color_luminance) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + ((result - lum) * lum) / (lum - minComp);
    }
    return maxComp > alpha && maxComp != lum ? lum + ((result - lum) * (alpha - lum)) / (maxComp - lum) : result;
}
vec3 _blend_set_color_saturation_helper(vec3 minMidMax, float sat) {
    return minMidMax.x < minMidMax.z ? vec3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat) : vec3(0.0);
}
vec3 _blend_set_color_saturation(vec3 hueLumColor, vec3 satColor) {
    float _13_blend_color_saturation;
    _13_blend_color_saturation = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);

    float sat = _13_blend_color_saturation;

    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            return _blend_set_color_saturation_helper(hueLumColor, sat);
        } else if (hueLumColor.x <= hueLumColor.z) {
            return _blend_set_color_saturation_helper(hueLumColor.xzy, sat).xzy;
        } else {
            return _blend_set_color_saturation_helper(hueLumColor.zxy, sat).yzx;
        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        return _blend_set_color_saturation_helper(hueLumColor.yxz, sat).yxz;
    } else if (hueLumColor.y <= hueLumColor.z) {
        return _blend_set_color_saturation_helper(hueLumColor.yzx, sat).zxy;
    } else {
        return _blend_set_color_saturation_helper(hueLumColor.zyx, sat).zyx;
    }
}
vec4 blend_hue(vec4 src, vec4 dst) {
    float alpha = dst.w * src.w;
    vec3 sda = src.xyz * dst.w;
    vec3 dsa = dst.xyz * src.w;
    return vec4((((_blend_set_color_luminance(_blend_set_color_saturation(sda, dsa), alpha, dsa) + dst.xyz) - dsa) + src.xyz) - sda, (src.w + dst.w) - alpha);
}
uniform vec4 color;
void main() {
    float _1_fma;
    float _2_mul;
    _2_mul = color.x * color.y;

    float _7_add;
    float _8_c = _2_mul + color.z;
    _7_add = _8_c;

    _1_fma = _7_add;



    sk_FragColor = vec4(_1_fma);

    sk_FragColor *= 1.25;

    vec4 _4_blend_src_in;
    _4_blend_src_in = color.xxyy * color.w;

    sk_FragColor *= _4_blend_src_in;

    vec4 _5_blend_dst_in;
    vec4 _6_blend_src_in;
    _6_blend_src_in = color.zzww * color.y;

    _5_blend_dst_in = _6_blend_src_in;


    sk_FragColor *= _5_blend_dst_in;

    sk_FragColor *= blend_hue(color, color.wwww);
    sk_FragColor *= blend_hue(color, color.wzyx);
}
