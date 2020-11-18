
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float _color_dodge_component(vec2 s, vec2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            float _48_guarded_divide;
            float _49_n = d.x * s.y;
            {
                _48_guarded_divide = _49_n / delta;
            }
            delta = min(d.y, _48_guarded_divide);

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
        float _50_guarded_divide;
        float _51_n = (d.y - d.x) * s.y;
        {
            _50_guarded_divide = _51_n / s.x;
        }
        float delta = max(0.0, d.y - _50_guarded_divide);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
float _soft_light_component(vec2 s, vec2 d) {
    if (2.0 * s.x <= s.y) {
        float _54_guarded_divide;
        float _55_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        {
            _54_guarded_divide = _55_n / d.y;
        }
        return (_54_guarded_divide + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);

    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _56_guarded_divide;
        float _57_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        {
            _56_guarded_divide = _57_n / DaSqd;
        }
        return _56_guarded_divide;

    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
vec3 _blend_set_color_luminance(vec3 hueSatColor, float alpha, vec3 lumColor) {
    float _58_blend_color_luminance;
    {
        _58_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), lumColor);
    }
    float lum = _58_blend_color_luminance;

    float _59_blend_color_luminance;
    {
        _59_blend_color_luminance = dot(vec3(0.30000001192092896, 0.5899999737739563, 0.10999999940395355), hueSatColor);
    }
    vec3 result = (lum - _59_blend_color_luminance) + hueSatColor;

    float minComp = min(min(result.x, result.y), result.z);
    float maxComp = max(max(result.x, result.y), result.z);
    if (minComp < 0.0 && lum != minComp) {
        result = lum + ((result - lum) * lum) / (lum - minComp);
    }
    return maxComp > alpha && maxComp != lum ? lum + ((result - lum) * (alpha - lum)) / (maxComp - lum) : result;
}
vec3 _blend_set_color_saturation(vec3 hueLumColor, vec3 satColor) {
    float _60_blend_color_saturation;
    {
        _60_blend_color_saturation = max(max(satColor.x, satColor.y), satColor.z) - min(min(satColor.x, satColor.y), satColor.z);
    }
    float sat = _60_blend_color_saturation;

    if (hueLumColor.x <= hueLumColor.y) {
        if (hueLumColor.y <= hueLumColor.z) {
            vec3 _61_blend_set_color_saturation_helper;
            {
                _61_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.x)) / (hueLumColor.z - hueLumColor.x), sat) : vec3(0.0);
            }
            hueLumColor.xyz = _61_blend_set_color_saturation_helper;

        } else if (hueLumColor.x <= hueLumColor.z) {
            vec3 _62_blend_set_color_saturation_helper;
            {
                _62_blend_set_color_saturation_helper = hueLumColor.x < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.x)) / (hueLumColor.y - hueLumColor.x), sat) : vec3(0.0);
            }
            hueLumColor.xzy = _62_blend_set_color_saturation_helper;

        } else {
            vec3 _63_blend_set_color_saturation_helper;
            {
                _63_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.y ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.z)) / (hueLumColor.y - hueLumColor.z), sat) : vec3(0.0);
            }
            hueLumColor.zxy = _63_blend_set_color_saturation_helper;

        }
    } else if (hueLumColor.x <= hueLumColor.z) {
        vec3 _64_blend_set_color_saturation_helper;
        {
            _64_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.z ? vec3(0.0, (sat * (hueLumColor.x - hueLumColor.y)) / (hueLumColor.z - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yxz = _64_blend_set_color_saturation_helper;

    } else if (hueLumColor.y <= hueLumColor.z) {
        vec3 _65_blend_set_color_saturation_helper;
        {
            _65_blend_set_color_saturation_helper = hueLumColor.y < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.z - hueLumColor.y)) / (hueLumColor.x - hueLumColor.y), sat) : vec3(0.0);
        }
        hueLumColor.yzx = _65_blend_set_color_saturation_helper;

    } else {
        vec3 _66_blend_set_color_saturation_helper;
        {
            _66_blend_set_color_saturation_helper = hueLumColor.z < hueLumColor.x ? vec3(0.0, (sat * (hueLumColor.y - hueLumColor.z)) / (hueLumColor.x - hueLumColor.z), sat) : vec3(0.0);
        }
        hueLumColor.zyx = _66_blend_set_color_saturation_helper;

    }
    return hueLumColor;
}
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
        vec4 _67_blend_src_in;
        {
            _67_blend_src_in = dst * src.w;
        }
        _6_blend_dst_in = _67_blend_src_in;

    }

    sk_FragColor = _6_blend_dst_in;

    vec4 _7_blend_src_out;
    {
        _7_blend_src_out = (1.0 - dst.w) * src;
    }

    sk_FragColor = _7_blend_src_out;

    vec4 _8_blend_dst_out;
    {
        _8_blend_dst_out = (1.0 - src.w) * dst;
    }

    sk_FragColor = _8_blend_dst_out;

    vec4 _9_blend_src_atop;
    {
        _9_blend_src_atop = dst.w * src + (1.0 - src.w) * dst;
    }

    sk_FragColor = _9_blend_src_atop;

    vec4 _10_blend_dst_atop;
    {
        _10_blend_dst_atop = (1.0 - dst.w) * src + src.w * dst;
    }

    sk_FragColor = _10_blend_dst_atop;

    vec4 _11_blend_xor;
    {
        _11_blend_xor = (1.0 - dst.w) * src + (1.0 - src.w) * dst;
    }

    sk_FragColor = _11_blend_xor;

    vec4 _12_blend_plus;
    {
        _12_blend_plus = min(src + dst, 1.0);
    }

    sk_FragColor = _12_blend_plus;

    vec4 _13_blend_modulate;
    {
        _13_blend_modulate = src * dst;
    }

    sk_FragColor = _13_blend_modulate;

    vec4 _14_blend_screen;
    {
        _14_blend_screen = src + (1.0 - src) * dst;
    }

    sk_FragColor = _14_blend_screen;

    vec4 _15_blend_overlay;
    {
        float _68_blend_overlay_component;
        {
            _68_blend_overlay_component = 2.0 * dst.x <= dst.w ? (2.0 * src.x) * dst.x : src.w * dst.w - (2.0 * (dst.w - dst.x)) * (src.w - src.x);
        }
        float _73_blend_overlay_component;
        {
            _73_blend_overlay_component = 2.0 * dst.y <= dst.w ? (2.0 * src.y) * dst.y : src.w * dst.w - (2.0 * (dst.w - dst.y)) * (src.w - src.y);
        }
        float _75_blend_overlay_component;
        {
            _75_blend_overlay_component = 2.0 * dst.z <= dst.w ? (2.0 * src.z) * dst.z : src.w * dst.w - (2.0 * (dst.w - dst.z)) * (src.w - src.z);
        }
        vec4 _16_result = vec4(_68_blend_overlay_component, _73_blend_overlay_component, _75_blend_overlay_component, src.w + (1.0 - src.w) * dst.w);



        _16_result.xyz += dst.xyz * (1.0 - src.w) + src.xyz * (1.0 - dst.w);
        _15_blend_overlay = _16_result;
    }

    sk_FragColor = _15_blend_overlay;

    vec4 _17_blend_darken;
    {
        vec4 _69_blend_src_over;
        {
            _69_blend_src_over = src + (1.0 - src.w) * dst;
        }
        vec4 _18_result = _69_blend_src_over;

        _18_result.xyz = min(_18_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
        _17_blend_darken = _18_result;
    }

    sk_FragColor = _17_blend_darken;

    vec4 _19_blend_lighten;
    {
        vec4 _70_blend_src_over;
        {
            _70_blend_src_over = src + (1.0 - src.w) * dst;
        }
        vec4 _20_result = _70_blend_src_over;

        _20_result.xyz = max(_20_result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
        _19_blend_lighten = _20_result;
    }

    sk_FragColor = _19_blend_lighten;

    vec4 _21_blend_color_dodge;
    {
        _21_blend_color_dodge = vec4(_color_dodge_component(src.xw, dst.xw), _color_dodge_component(src.yw, dst.yw), _color_dodge_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _21_blend_color_dodge;

    vec4 _22_blend_color_burn;
    {
        _22_blend_color_burn = vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _22_blend_color_burn;

    vec4 _23_blend_hard_light;
    {
        vec4 _71_blend_overlay;
        {
            float _74_blend_overlay_component;
            {
                _74_blend_overlay_component = 2.0 * src.x <= src.w ? (2.0 * dst.x) * src.x : dst.w * src.w - (2.0 * (src.w - src.x)) * (dst.w - dst.x);
            }
            float _76_blend_overlay_component;
            {
                _76_blend_overlay_component = 2.0 * src.y <= src.w ? (2.0 * dst.y) * src.y : dst.w * src.w - (2.0 * (src.w - src.y)) * (dst.w - dst.y);
            }
            float _77_blend_overlay_component;
            {
                _77_blend_overlay_component = 2.0 * src.z <= src.w ? (2.0 * dst.z) * src.z : dst.w * src.w - (2.0 * (src.w - src.z)) * (dst.w - dst.z);
            }
            vec4 _72_result = vec4(_74_blend_overlay_component, _76_blend_overlay_component, _77_blend_overlay_component, dst.w + (1.0 - dst.w) * src.w);



            _72_result.xyz += src.xyz * (1.0 - dst.w) + dst.xyz * (1.0 - src.w);
            _71_blend_overlay = _72_result;
        }
        _23_blend_hard_light = _71_blend_overlay;

    }

    sk_FragColor = _23_blend_hard_light;

    vec4 _24_blend_soft_light;
    {
        _24_blend_soft_light = dst.w == 0.0 ? src : vec4(_soft_light_component(src.xw, dst.xw), _soft_light_component(src.yw, dst.yw), _soft_light_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _24_blend_soft_light;

    vec4 _25_blend_difference;
    {
        _25_blend_difference = vec4((src.xyz + dst.xyz) - 2.0 * min(src.xyz * dst.w, dst.xyz * src.w), src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _25_blend_difference;

    vec4 _26_blend_exclusion;
    {
        _26_blend_exclusion = vec4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _26_blend_exclusion;

    vec4 _27_blend_multiply;
    {
        _27_blend_multiply = vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _27_blend_multiply;

    vec4 _28_blend_hue;
    {
        float _29_alpha = dst.w * src.w;
        vec3 _30_sda = src.xyz * dst.w;
        vec3 _31_dsa = dst.xyz * src.w;
        _28_blend_hue = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_30_sda, _31_dsa), _29_alpha, _31_dsa) + dst.xyz) - _31_dsa) + src.xyz) - _30_sda, (src.w + dst.w) - _29_alpha);
    }

    sk_FragColor = _28_blend_hue;

    vec4 _32_blend_saturation;
    {
        float _33_alpha = dst.w * src.w;
        vec3 _34_sda = src.xyz * dst.w;
        vec3 _35_dsa = dst.xyz * src.w;
        _32_blend_saturation = vec4((((_blend_set_color_luminance(_blend_set_color_saturation(_35_dsa, _34_sda), _33_alpha, _35_dsa) + dst.xyz) - _35_dsa) + src.xyz) - _34_sda, (src.w + dst.w) - _33_alpha);
    }

    sk_FragColor = _32_blend_saturation;

    vec4 _36_blend_color;
    {
        float _37_alpha = dst.w * src.w;
        vec3 _38_sda = src.xyz * dst.w;
        vec3 _39_dsa = dst.xyz * src.w;
        _36_blend_color = vec4((((_blend_set_color_luminance(_38_sda, _37_alpha, _39_dsa) + dst.xyz) - _39_dsa) + src.xyz) - _38_sda, (src.w + dst.w) - _37_alpha);
    }

    sk_FragColor = _36_blend_color;

    vec4 _40_blend_luminosity;
    {
        float _41_alpha = dst.w * src.w;
        vec3 _42_sda = src.xyz * dst.w;
        vec3 _43_dsa = dst.xyz * src.w;
        _40_blend_luminosity = vec4((((_blend_set_color_luminance(_43_dsa, _41_alpha, _42_sda) + dst.xyz) - _43_dsa) + src.xyz) - _42_sda, (src.w + dst.w) - _41_alpha);
    }

    sk_FragColor = _40_blend_luminosity;

}
