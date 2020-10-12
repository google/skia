
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float _guarded_divide(float n, float d) {
    return n / (d + 9.9999999392252903e-09);
}
float _color_dodge_component(vec2 s, vec2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            float _0_guarded_divide;
            float _1_n = d.x * s.y;
            {
                _0_guarded_divide = _1_n / (delta + 9.9999999392252903e-09);
            }
            delta = min(d.y, _0_guarded_divide);

            return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        }
    }
}
vec4 blend_color_dodge(vec4 src, vec4 dst) {
    return vec4(_color_dodge_component(src.xw, dst.xw), _color_dodge_component(src.yw, dst.yw), _color_dodge_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
float _color_burn_component(vec2 s, vec2 d) {
    if (d.y == d.x) {
        return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    } else if (s.x == 0.0) {
        return d.x * (1.0 - s.y);
    } else {
        float _2_guarded_divide;
        float _3_n = (d.y - d.x) * s.y;
        {
            _2_guarded_divide = _3_n / (s.x + 9.9999999392252903e-09);
        }
        float delta = max(0.0, d.y - _2_guarded_divide);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
vec4 blend_color_burn(vec4 src, vec4 dst) {
    return vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
float _soft_light_component(vec2 s, vec2 d) {
    if (2.0 * s.x <= s.y) {
        float _4_guarded_divide;
        float _5_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        {
            _4_guarded_divide = _5_n / (d.y + 9.9999999392252903e-09);
        }
        return (_4_guarded_divide + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);

    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _6_guarded_divide;
        float _7_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        {
            _6_guarded_divide = _7_n / (DaSqd + 9.9999999392252903e-09);
        }
        return _6_guarded_divide;

    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
vec4 blend_soft_light(vec4 src, vec4 dst) {
    return dst.w == 0.0 ? src : vec4(_soft_light_component(src.xw, dst.xw), _soft_light_component(src.yw, dst.yw), _soft_light_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
void main() {
    vec4 _8_blend_color_dodge;
    {
        _8_blend_color_dodge = vec4(_color_dodge_component(src.xw, dst.xw), _color_dodge_component(src.yw, dst.yw), _color_dodge_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }
    sk_FragColor = _8_blend_color_dodge;

    vec4 _9_blend_color_burn;
    {
        _9_blend_color_burn = vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }
    sk_FragColor = _9_blend_color_burn;

    vec4 _10_blend_soft_light;
    {
        _10_blend_soft_light = dst.w == 0.0 ? src : vec4(_soft_light_component(src.xw, dst.xw), _soft_light_component(src.yw, dst.yw), _soft_light_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }
    sk_FragColor = _10_blend_soft_light;

}
