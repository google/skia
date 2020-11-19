#version 400
out vec4 sk_FragColor;
float _color_dodge_component(vec2 s, vec2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            float _4_guarded_divide;
            float _5_n = d.x * s.y;
            {
                _4_guarded_divide = _5_n / delta;
            }
            delta = min(d.y, _4_guarded_divide);

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
        float _6_guarded_divide;
        float _7_n = (d.y - d.x) * s.y;
        {
            _6_guarded_divide = _7_n / s.x;
        }
        float delta = max(0.0, d.y - _6_guarded_divide);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component(vec2 s, vec2 d) {
    if (2.0 * s.x <= s.y) {
        float _11_guarded_divide;
        float _12_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        {
            _11_guarded_divide = _12_n / d.y;
        }
        return (_11_guarded_divide + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);

    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _13_guarded_divide;
        float _14_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        {
            _13_guarded_divide = _14_n / DaSqd;
        }
        return _13_guarded_divide;

    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
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
            hueLumColor.xyz = _18_blend_set_color_saturation_helper;

        } else if (hueLumColor.x <= hueLumColor.z) {
            vec3 _19_blend_set_color_saturation_helper;
            {
                _19_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.x)) / (hueLumColor.y - hueLumColor.x), sat) : vec3(0.0);
            }
            hueLumColor.xzy = _19_blend_set_color_saturation_helper;

        } else {
            vec3 _20_blend_set_color_saturation_helper;
            {
                _20_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.z)) / (hueLumColor.y - hueLumColor.z), sat) : vec3(0.0);
            }
            hueLumColor.zxy = _20_blend_set_color_saturation_helper;

        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        vec3 _21_blend_set_color_saturation_helper;
        {
            _21_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.y)) / (hueLumColor.z - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yxz = _21_blend_set_color_saturation_helper;

    } else if (hueLumColor.y <= hueLumColor.z) {
        vec3 _22_blend_set_color_saturation_helper;
        {
            _22_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.y)) / (hueLumColor.x - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yzx = _22_blend_set_color_saturation_helper;

    } else {
        vec3 _23_blend_set_color_saturation_helper;
        {
            _23_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.z)) / (hueLumColor.x - hueLumColor.z), sat) : vec3(0.0);
        }
        hueLumColor.zyx = _23_blend_set_color_saturation_helper;

    }
    return hueLumColor;
}
in vec4 src;
in vec4 dst;
void main() {
    sk_FragColor = vec4(0.0);

    vec4 _1_blend_src;
    {
        _1_blend_src = src;
    }
    sk_FragColor = _1_blend_src;

    vec4 _2_blend_dst;
    {
        _2_blend_dst = dst;
    }
    sk_FragColor = _2_blend_dst;

    vec4 _3_blend_src_over;
    {
        _3_blend_src_over = src + (1.0 - src.w) * dst;
    }
    sk_FragColor = _3_blend_src_over;

    vec4 _4_blend_dst_over;
    {
        _4_blend_dst_over = (1.0 - dst.w) * src + dst;
    }
    sk_FragColor = _4_blend_dst_over;

    vec4 _5_blend_src_in;
    {
        _5_blend_src_in = src * dst.w;
    }
    sk_FragColor = _5_blend_src_in;

    vec4 _6_blend_dst_in;
    {
        vec4 _7_0_blend_src_in;
        {
            _7_0_blend_src_in = dst * src.w;
        }
        _6_blend_dst_in = _7_0_blend_src_in;

    }
    sk_FragColor = _6_blend_dst_in;

    vec4 _8_blend_src_out;
    {
        _8_blend_src_out = (1.0 - dst.w) * src;
    }
    sk_FragColor = _8_blend_src_out;

    vec4 _9_blend_dst_out;
    {
        _9_blend_dst_out = (1.0 - src.w) * dst;
    }
    sk_FragColor = _9_blend_dst_out;

    vec4 _10_blend_src_atop;
    {
        _10_blend_src_atop = dst.w * src + (1.0 - src.w) * dst;
    }
    sk_FragColor = _10_blend_src_atop;

    vec4 _11_blend_dst_atop;
    {
        _11_blend_dst_atop = (1.0 - dst.w) * src + src.w * dst;
    }
    sk_FragColor = _11_blend_dst_atop;

    vec4 _12_blend_xor;
    {
        _12_blend_xor = (1.0 - dst.w) * src + (1.0 - src.w) * dst;
    }
    sk_FragColor = _12_blend_xor;

    vec4 _13_blend_plus;
    {
        _13_blend_plus = min(src + dst, 1.0);
    }
    sk_FragColor = _13_blend_plus;

    vec4 _14_blend_modulate;
    {
        _14_blend_modulate = src * dst;
    }
    sk_FragColor = _14_blend_modulate;

    vec4 _15_blend_screen;
    {
        _15_blend_screen = src + (1.0 - src) * dst;
    }
    sk_FragColor = _15_blend_screen;

    vec4 _16_blend_overlay;
    {
        float _17_1_blend_overlay_component;
        {
            _17_1_blend_overlay_component = 2.0 * dst.x <= dst.w ? (2.0 * src.x) * dst.x : src.w * dst.w - (2.0 * (dst.w - dst.x)) * (src.w - src.x);
        }
        float _18_75_blend_overlay_component;
        {
            _18_75_blend_overlay_component = 2.0 * dst.y <= dst.w ? (2.0 * src.y) * dst.y : src.w * dst.w - (2.0 * (dst.w - dst.y)) * (src.w - src.y);
        }
        float _19_79_blend_overlay_component;
        {
            _19_79_blend_overlay_component = 2.0 * dst.z <= dst.w ? (2.0 * src.z) * dst.z : src.w * dst.w - (2.0 * (dst.w - dst.z)) * (src.w - src.z);
        }
        vec4 _20_result = vec4(_17_1_blend_overlay_component, _18_75_blend_overlay_component, _19_79_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



        _20_result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
        _16_blend_overlay = _20_result;
    }
    sk_FragColor = _16_blend_overlay;

    vec4 _21_blend_darken;
    {
        vec4 _22_2_blend_src_over;
        {
            _22_2_blend_src_over = src + (1.0 - src.w) * dst;
        }
        vec4 _23_result = _22_2_blend_src_over;

        _23_result.xyz = min(_23_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
        _21_blend_darken = _23_result;
    }
    sk_FragColor = _21_blend_darken;

    vec4 _24_blend_lighten;
    {
        vec4 _25_3_blend_src_over;
        {
            _25_3_blend_src_over = src + (1.0 - src.w) * dst;
        }
        vec4 _26_result = _25_3_blend_src_over;

        _26_result.xyz = max(_26_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
        _24_blend_lighten = _26_result;
    }
    sk_FragColor = _24_blend_lighten;

    vec4 _27_blend_color_dodge;
    {
        _27_blend_color_dodge = vec4(_color_dodge_component(src.xw, dst.xw), _color_dodge_component(src.yw, dst.yw), _color_dodge_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }
    sk_FragColor = _27_blend_color_dodge;

    vec4 _28_blend_color_burn;
    {
        _28_blend_color_burn = vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }
    sk_FragColor = _28_blend_color_burn;

    vec4 _29_blend_hard_light;
    {
        vec4 _30_8_blend_overlay;
        {
            float _31_9_1_blend_overlay_component;
            {
                _31_9_1_blend_overlay_component = 2.0 * src.x <= src.w ? (2.0 * dst.x) * src.x : dst.w * src.w - (2.0 * (src.w - src.x)) * (dst.w - dst.x);
            }
            float _32_76_blend_overlay_component;
            {
                _32_76_blend_overlay_component = 2.0 * src.y <= src.w ? (2.0 * dst.y) * src.y : dst.w * src.w - (2.0 * (src.w - src.y)) * (dst.w - dst.y);
            }
            float _33_80_blend_overlay_component;
            {
                _33_80_blend_overlay_component = 2.0 * src.z <= src.w ? (2.0 * dst.z) * src.z : dst.w * src.w - (2.0 * (src.w - src.z)) * (dst.w - dst.z);
            }
            vec4 _34_10_result = vec4(_31_9_1_blend_overlay_component, _32_76_blend_overlay_component, _33_80_blend_overlay_component, dst.w + (1.0 - dst.w) * src.w);



            _34_10_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
            _30_8_blend_overlay = _34_10_result;
        }
        _29_blend_hard_light = _30_8_blend_overlay;

    }
    sk_FragColor = _29_blend_hard_light;

    vec4 _35_blend_soft_light;
    {
        _35_blend_soft_light = dst.w == 0.0 ? src : vec4(_soft_light_component(src.xw, dst.xw), _soft_light_component(src.yw, dst.yw), _soft_light_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }
    sk_FragColor = _35_blend_soft_light;

    vec4 _36_blend_difference;
    {
        _36_blend_difference = vec4((src.xyz + dst.xyz) - 2.0 * min(src.xyz * dst.w, dst.xyz * src.w), src.w + (1.0 - src.w) * dst.w);
    }
    sk_FragColor = _36_blend_difference;

    vec4 _37_blend_exclusion;
    {
        _37_blend_exclusion = vec4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);
    }
    sk_FragColor = _37_blend_exclusion;

    vec4 _38_blend_multiply;
    {
        _38_blend_multiply = vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
    }
    sk_FragColor = _38_blend_multiply;

    vec4 _39_blend_hue;
    {
        float _40_alpha = dst.w * src.w;
        vec3 _41_sda = src.xyz * dst.w;
        vec3 _42_dsa = dst.xyz * src.w;
        _39_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_41_sda, _42_dsa), _40_alpha, _42_dsa) + dst.xyz) - _42_dsa) + src.xyz) - _41_sda, (src.w + dst.w) - _40_alpha);
    }
    sk_FragColor = _39_blend_hue;

    vec4 _43_blend_saturation;
    {
        float _44_alpha = dst.w * src.w;
        vec3 _45_sda = src.xyz * dst.w;
        vec3 _46_dsa = dst.xyz * src.w;
        _43_blend_saturation = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_46_dsa, _45_sda), _44_alpha, _46_dsa) + dst.xyz) - _46_dsa) + src.xyz) - _45_sda, (src.w + dst.w) - _44_alpha);
    }
    sk_FragColor = _43_blend_saturation;

    vec4 _47_blend_color;
    {
        float _48_alpha = dst.w * src.w;
        vec3 _49_sda = src.xyz * dst.w;
        vec3 _50_dsa = dst.xyz * src.w;
        _47_blend_color = vec4((((_blend_set_color_luminance(_49_sda, _48_alpha, _50_dsa) + dst.xyz) - _50_dsa) + src.xyz) - _49_sda, (src.w + dst.w) - _48_alpha);
    }
    sk_FragColor = _47_blend_color;

    vec4 _51_blend_luminosity;
    {
        float _52_alpha = dst.w * src.w;
        vec3 _53_sda = src.xyz * dst.w;
        vec3 _54_dsa = dst.xyz * src.w;
        _51_blend_luminosity = vec4((((_blend_set_color_luminance(_54_dsa, _52_alpha, _53_sda) + dst.xyz) - _54_dsa) + src.xyz) - _53_sda, (src.w + dst.w) - _52_alpha);
    }
    sk_FragColor = _51_blend_luminosity;

}
