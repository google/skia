#version 400
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec3 blend_set_color_luminance_Qh3h3hh3(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    vec3 result = (lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor)) + hueSatColor;
    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + (result - lum) * (lum / (lum - minComp));
    }
    if (maxComp > alpha && maxComp != lum) {
        return lum + ((result - lum) * (alpha - lum)) / (maxComp - lum);
    } else {
        return result;
    }
}
vec3 blend_set_color_saturation_helper_Qh3h3h(vec3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        return vec3(0.0, (sat * (minMidMax.y - minMidMax.x)) / (minMidMax.z - minMidMax.x), sat);
    } else {
        return vec3(0.0);
    }
}
vec3 blend_set_color_saturation_Qh3h3h3(vec3 hueLumColor, vec3 satColor) {
    float sat = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            return blend_set_color_saturation_helper_Qh3h3h(hueLumColor, sat);
        } else if (hueLumColor.x <= hueLumColor.z) {
            return blend_set_color_saturation_helper_Qh3h3h(hueLumColor.xzy, sat).xzy;
        } else {
            return blend_set_color_saturation_helper_Qh3h3h(hueLumColor.zxy, sat).yzx;
        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        return blend_set_color_saturation_helper_Qh3h3h(hueLumColor.yxz, sat).yxz;
    } else if (hueLumColor.y <= hueLumColor.z) {
        return blend_set_color_saturation_helper_Qh3h3h(hueLumColor.yzx, sat).zxy;
    } else {
        return blend_set_color_saturation_helper_Qh3h3h(hueLumColor.zyx, sat).zyx;
    }
}
void main() {
    float _0_alpha = dst.w * src.w;
    vec3 _1_sda = src.xyz * dst.w;
    vec3 _2_dsa = dst.xyz * src.w;
    sk_FragColor = vec4((((blend_set_color_luminance_Qh3h3hh3(blend_set_color_saturation_Qh3h3h3(_2_dsa, _1_sda), _0_alpha, _2_dsa) + dst.xyz) - _2_dsa) + src.xyz) - _1_sda, (src.w + dst.w) - _0_alpha);
}
