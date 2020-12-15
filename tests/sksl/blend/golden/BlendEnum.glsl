#version 400
out vec4 sk_FragColor;
float _blend_overlay_component(vec2 s, vec2 d) {
    return 2.0 * d.x <= d.y ? (2.0 * s.x) * d.x : s.y * d.y - (2.0 * (d.y - d.x)) * (s.y - s.x);
}
vec4 blend_overlay(vec4 src, vec4 dst) {
    vec4 result = vec4(_blend_overlay_component(src.xw, dst.xw), _blend_overlay_component(src.yw, dst.yw), _blend_overlay_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
    return result;
}
float _color_dodge_component(vec2 s, vec2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            float _4_n = d.x * s.y;
            delta = min(d.y, _4_n / delta);

            return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        }
    }
}
float _color_burn_component(vec2 s, vec2 d) {
    if (d.y == d.x) {
        return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    } else if (s.x == 0.0) {
        return d.x * (1.0 - s.y);
    } else {
        float _6_n = (d.y - d.x) * s.y;
        float delta = max(0.0, d.y - _6_n / s.x);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component(vec2 s, vec2 d) {
    if (2.0 * s.x <= s.y) {
        float _8_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        return (_8_n / d.y + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);

    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _10_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        return _10_n / DaSqd;

    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float lum = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);

    vec3 result = (lum - dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor)) + hueSatColor;

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
vec4 blend(int mode, vec4 src, vec4 dst) {
    switch (mode) {
        case 0:
            return vec4(0.0);

        case 1:
            return src;

        case 2:
            return dst;

        case 3:
            return src + (1.0 - src.w) * dst;

        case 4:
            return (1.0 - dst.w) * src + dst;

        case 5:
            return src * dst.w;

        case 6:
            return dst * src.w;

        case 7:
            return (1.0 - dst.w) * src;

        case 8:
            return (1.0 - src.w) * dst;

        case 9:
            return dst.w * src + (1.0 - src.w) * dst;

        case 10:
            return (1.0 - dst.w) * src + src.w * dst;

        case 11:
            return (1.0 - dst.w) * src + (1.0 - src.w) * dst;

        case 12:
            return min(src + dst, 1.0);

        case 13:
            return src * dst;

        case 14:
            return src + (1.0 - src) * dst;

        case 15:
            return blend_overlay(src, dst);
        case 16:
            vec4 _32_result;
            _32_result = src + (1.0 - src.w) * dst;

            _32_result.xyz = min(_32_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);

            return _32_result;

        case 17:
            vec4 _35_result;
            _35_result = src + (1.0 - src.w) * dst;

            _35_result.xyz = max(_35_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);

            return _35_result;

        case 18:
            return vec4(_color_dodge_component(src.xw, dst.xw), _color_dodge_component(src.yw, dst.yw), _color_dodge_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);

        case 19:
            return vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);

        case 20:
            return blend_overlay(dst, src);

        case 21:
            return dst.w == 0.0 ? src : vec4(_soft_light_component(src.xw, dst.xw), _soft_light_component(src.yw, dst.yw), _soft_light_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);

        case 22:
            return vec4((src.xyz + dst.xyz) - 2.0 * min(src.xyz * dst.w, dst.xyz * src.w), src.w + (1.0 - src.w) * dst.w);

        case 23:
            return vec4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);

        case 24:
            return vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);

        case 25:
            float _44_alpha;
            vec3 _45_sda;
            vec3 _46_dsa;
            _44_alpha = dst.w * src.w;
            _45_sda = src.xyz * dst.w;
            _46_dsa = dst.xyz * src.w;

            return vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_45_sda, _46_dsa), _44_alpha, _46_dsa) + dst.xyz) - _46_dsa) + src.xyz) - _45_sda, (src.w + dst.w) - _44_alpha);

        case 26:
            float _48_alpha;
            vec3 _49_sda;
            vec3 _50_dsa;
            _48_alpha = dst.w * src.w;
            _49_sda = src.xyz * dst.w;
            _50_dsa = dst.xyz * src.w;

            return vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_50_dsa, _49_sda), _48_alpha, _50_dsa) + dst.xyz) - _50_dsa) + src.xyz) - _49_sda, (src.w + dst.w) - _48_alpha);

        case 27:
            float _52_alpha;
            vec3 _53_sda;
            vec3 _54_dsa;
            _52_alpha = dst.w * src.w;
            _53_sda = src.xyz * dst.w;
            _54_dsa = dst.xyz * src.w;

            return vec4((((_blend_set_color_luminance(_53_sda, _52_alpha, _54_dsa) + dst.xyz) - _54_dsa) + src.xyz) - _53_sda, (src.w + dst.w) - _52_alpha);

        case 28:
            float _56_alpha;
            vec3 _57_sda;
            vec3 _58_dsa;
            _56_alpha = dst.w * src.w;
            _57_sda = src.xyz * dst.w;
            _58_dsa = dst.xyz * src.w;

            return vec4((((_blend_set_color_luminance(_58_dsa, _56_alpha, _57_sda) + dst.xyz) - _58_dsa) + src.xyz) - _57_sda, (src.w + dst.w) - _56_alpha);

    }
    return vec4(0.0);
}
in vec4 src;
in vec4 dst;
void main() {
    sk_FragColor = blend(13, src, dst);
}
