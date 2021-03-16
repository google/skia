
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
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
            float _0_n = d.x * s.y;
            delta = min(d.y, _0_n / delta);
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
        float _1_n = (d.y - d.x) * s.y;
        float delta = max(0.0, d.y - _1_n / s.x);
        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component(vec2 s, vec2 d) {
    if (2.0 * s.x <= s.y) {
        float _2_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        return (_2_n / d.y + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);
    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _3_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        return _3_n / DaSqd;
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
        float _4_d = lum - minComp;
        result = lum + (result - lum) * (lum / _4_d);
    }
    if (maxComp > alpha && maxComp != lum) {
        vec3 _5_n = (result - lum) * (alpha - lum);
        float _6_d = maxComp - lum;
        return lum + _5_n / _6_d;
    } else {
        return result;
    }
}
vec3 _blend_set_color_saturation_helper(vec3 minMidMax, float sat) {
    if (minMidMax.x < minMidMax.z) {
        float _7_n = sat * (minMidMax.y - minMidMax.x);
        float _8_d = minMidMax.z - minMidMax.x;
        return vec3(0.0, _7_n / _8_d, sat);
    } else {
        return vec3(0.0);
    }
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
            vec4 _9_result = src + (1.0 - src.w) * dst;
            _9_result.xyz = min(_9_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
            return _9_result;
        case 17:
            vec4 _10_result = src + (1.0 - src.w) * dst;
            _10_result.xyz = max(_10_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
            return _10_result;
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
            float _11_alpha = dst.w * src.w;
            vec3 _12_sda = src.xyz * dst.w;
            vec3 _13_dsa = dst.xyz * src.w;
            return vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_12_sda, _13_dsa), _11_alpha, _13_dsa) + dst.xyz) - _13_dsa) + src.xyz) - _12_sda, (src.w + dst.w) - _11_alpha);
        case 26:
            float _14_alpha = dst.w * src.w;
            vec3 _15_sda = src.xyz * dst.w;
            vec3 _16_dsa = dst.xyz * src.w;
            return vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_16_dsa, _15_sda), _14_alpha, _16_dsa) + dst.xyz) - _16_dsa) + src.xyz) - _15_sda, (src.w + dst.w) - _14_alpha);
        case 27:
            float _17_alpha = dst.w * src.w;
            vec3 _18_sda = src.xyz * dst.w;
            vec3 _19_dsa = dst.xyz * src.w;
            return vec4((((_blend_set_color_luminance(_18_sda, _17_alpha, _19_dsa) + dst.xyz) - _19_dsa) + src.xyz) - _18_sda, (src.w + dst.w) - _17_alpha);
        case 28:
            float _20_alpha = dst.w * src.w;
            vec3 _21_sda = src.xyz * dst.w;
            vec3 _22_dsa = dst.xyz * src.w;
            return vec4((((_blend_set_color_luminance(_22_dsa, _20_alpha, _21_sda) + dst.xyz) - _22_dsa) + src.xyz) - _21_sda, (src.w + dst.w) - _20_alpha);
        default:
            return vec4(0.0);
    }
}
void main() {
    sk_FragColor = blend(13, src, dst);
}
