
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float _guarded_divide(float n, float d) {
    return n / d;
}
float _color_dodge_component(vec2 s, vec2 d) {
    if (d.x == 0.0) {
        return s.x * (1.0 - d.y);
    } else {
        float delta = s.y - s.x;
        if (delta == 0.0) {
            return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
        } else {
            float _3_guarded_divide;
            float _4_n = d.x * s.y;
            {
                _3_guarded_divide = _4_n / delta;
            }
            delta = min(d.y, _3_guarded_divide);

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
        float _5_guarded_divide;
        float _6_n = (d.y - d.x) * s.y;
        {
            _5_guarded_divide = _6_n / s.x;
        }
        float delta = max(0.0, d.y - _5_guarded_divide);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
vec4 blend_color_burn(vec4 src, vec4 dst) {
    return vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
float _soft_light_component(vec2 s, vec2 d) {
    if (2.0 * s.x <= s.y) {
        float _7_guarded_divide;
        float _8_n = (d.x * d.x) * (s.y - 2.0 * s.x);
        {
            _7_guarded_divide = _8_n / d.y;
        }
        return (_7_guarded_divide + (1.0 - d.y) * s.x) + d.x * ((-s.y + 2.0 * s.x) + 1.0);

    } else if (4.0 * d.x <= d.y) {
        float DSqd = d.x * d.x;
        float DCub = DSqd * d.x;
        float DaSqd = d.y * d.y;
        float DaCub = DaSqd * d.y;
        float _9_guarded_divide;
        float _10_n = ((DaSqd * (s.x - d.x * ((3.0 * s.y - 6.0 * s.x) - 1.0)) + ((12.0 * d.y) * DSqd) * (s.y - 2.0 * s.x)) - (16.0 * DCub) * (s.y - 2.0 * s.x)) - DaCub * s.x;
        {
            _9_guarded_divide = _10_n / DaSqd;
        }
        return _9_guarded_divide;

    } else {
        return ((d.x * ((s.y - 2.0 * s.x) + 1.0) + s.x) - sqrt(d.y * d.x) * (s.y - 2.0 * s.x)) - d.y * s.x;
    }
}
vec4 blend_soft_light(vec4 src, vec4 dst) {
    return dst.w == 0.0 ? src : vec4(_soft_light_component(src.xw, dst.xw), _soft_light_component(src.yw, dst.yw), _soft_light_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
void main() {
    vec4 _0_blend_color_dodge;
    {
        _0_blend_color_dodge = vec4(_color_dodge_component(src.xw, dst.xw), _color_dodge_component(src.yw, dst.yw), _color_dodge_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _0_blend_color_dodge;

    vec4 _1_blend_color_burn;
    {
        _1_blend_color_burn = vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _1_blend_color_burn;

    vec4 _2_blend_soft_light;
    {
        _2_blend_soft_light = dst.w == 0.0 ? src : vec4(_soft_light_component(src.xw, dst.xw), _soft_light_component(src.yw, dst.yw), _soft_light_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _2_blend_soft_light;

}
