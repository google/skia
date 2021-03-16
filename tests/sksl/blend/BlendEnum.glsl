#version 400
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
void main() {
    vec4 _0_blend;
    {
        _0_blend = src * dst;
        _0_blend = src + (1.0 - src) * dst;
        _0_blend = blend_overlay(src, dst);
        vec4 _1_result = src + (1.0 - src.w) * dst;
        _1_result.xyz = min(_1_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
        _0_blend = _1_result;
        vec4 _2_result = src + (1.0 - src.w) * dst;
        _2_result.xyz = max(_2_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
        _0_blend = _2_result;
        _0_blend = vec4(_color_dodge_component(src.xw, dst.xw), _color_dodge_component(src.yw, dst.yw), _color_dodge_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
        _0_blend = vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
        _0_blend = blend_overlay(dst, src);
        _0_blend = dst.w == 0.0 ? src : vec4(_soft_light_component(src.xw, dst.xw), _soft_light_component(src.yw, dst.yw), _soft_light_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
        _0_blend = vec4((src.xyz + dst.xyz) - 2.0 * min(src.xyz * dst.w, dst.xyz * src.w), src.w + (1.0 - src.w) * dst.w);
        _0_blend = vec4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);
        _0_blend = vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
        float _3_alpha = dst.w * src.w;
        vec3 _4_sda = src.xyz * dst.w;
        vec3 _5_dsa = dst.xyz * src.w;
        _0_blend = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_4_sda, _5_dsa), _3_alpha, _5_dsa) + dst.xyz) - _5_dsa) + src.xyz) - _4_sda, (src.w + dst.w) - _3_alpha);
        float _6_alpha = dst.w * src.w;
        vec3 _7_sda = src.xyz * dst.w;
        vec3 _8_dsa = dst.xyz * src.w;
        _0_blend = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_8_dsa, _7_sda), _6_alpha, _8_dsa) + dst.xyz) - _8_dsa) + src.xyz) - _7_sda, (src.w + dst.w) - _6_alpha);
        float _9_alpha = dst.w * src.w;
        vec3 _10_sda = src.xyz * dst.w;
        vec3 _11_dsa = dst.xyz * src.w;
        _0_blend = vec4((((_blend_set_color_luminance(_10_sda, _9_alpha, _11_dsa) + dst.xyz) - _11_dsa) + src.xyz) - _10_sda, (src.w + dst.w) - _9_alpha);
        float _12_alpha = dst.w * src.w;
        vec3 _13_sda = src.xyz * dst.w;
        vec3 _14_dsa = dst.xyz * src.w;
        _0_blend = vec4((((_blend_set_color_luminance(_14_dsa, _12_alpha, _13_sda) + dst.xyz) - _14_dsa) + src.xyz) - _13_sda, (src.w + dst.w) - _12_alpha);
        _0_blend = vec4(0.0);
    }
    sk_FragColor = _0_blend;
}
