
out vec4 sk_FragColor;
uniform vec4 color;
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float _11_blend_color_luminance;
    float lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);

    float _12_blend_color_luminance;
    vec3 result = (lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor)) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        float _13_guarded_divide;
        float _14_d = lum - minComp;
        result = lum + (result - lum) * (lum / _14_d);

    }
    if (maxComp > alpha && maxComp != lum) {
        vec3 _15_guarded_divide;
        vec3 _16_n = (result - lum) * (alpha - lum);
        float _17_d = maxComp - lum;
        return lum + _16_n / _17_d;

    } else {
        return result;
    }
}
vec3 _blend_set_color_saturation_helper(vec3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        float _18_guarded_divide;
        float _19_n = sat * (minMidMax.y - minMidMax.x);
        float _20_d = minMidMax.z - minMidMax.x;
        return vec3(0.0, _19_n / _20_d, sat);

    } else {
        return vec3(0.0);
    }
}
vec3 _blend_set_color_saturation(vec3 hueLumColor, vec3 satColor) {
    float _21_blend_color_saturation;
    float sat = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);

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
void main() {
    float _1_fma;
    float _2_mul;

    float _7_add;
    float _8_a = color.x * color.y;
    float _9_c = _8_a + color.z;
    sk_FragColor = vec4(_9_c);


    float _3_singleuse;
    sk_FragColor *= 1.25;

    vec4 _4_blend_src_in;
    sk_FragColor *= color.xxyy * color.zzww.w;

    vec4 _5_blend_dst_in;
    vec4 _6_blend_src_in;

    sk_FragColor *= color.zzww * color.xxyy.w;

    sk_FragColor *= blend_hue(color, color.wwww);
    sk_FragColor *= blend_hue(color, color.wzyx);
}
