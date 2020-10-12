
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
float _guarded_divide(float n, float d) {
    return n / d;
}
float _color_burn_component(vec2 s, vec2 d) {
    if (d.y == d.x) {
        return (s.y * d.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    } else if (s.x == 0.0) {
        return d.x * (1.0 - s.y);
    } else {
        float _1_guarded_divide;
        float _2_n = (d.y - d.x) * s.y;
        {
            _1_guarded_divide = _2_n / s.x;
        }
        float delta = max(0.0, d.y - _1_guarded_divide);

        return (delta * s.y + s.x * (1.0 - d.y)) + d.x * (1.0 - s.y);
    }
}
vec4 blend_color_burn(vec4 src, vec4 dst) {
    return vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
}
void main() {
    vec4 _0_blend_color_burn;
    {
        _0_blend_color_burn = vec4(_color_burn_component(src.xw, dst.xw), _color_burn_component(src.yw, dst.yw), _color_burn_component(src.zw, dst.zw), src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _0_blend_color_burn;

}
