
uniform vec4 src, dst;
float _guarded_divide(float n, float d) {
    return n / d;
}
float _color_burn_component(float sc, float sa, float dc, float da) {
    if (da == dc) {
        return (sa * da + sc * (1.0 - da)) + dc * (1.0 - sa);
    } else if (sc == 0.0) {
        return dc * (1.0 - sa);
    } else {
        float _1_guarded_divide;
        float _2_n = (da - dc) * sa;
        {
            _1_guarded_divide = _2_n / sc;
        }
        float d = max(0.0, da - _1_guarded_divide);

        return (d * sa + sc * (1.0 - da)) + dc * (1.0 - sa);
    }
}
vec4 blend_color_burn(vec4 src, vec4 dst) {
    return vec4(_color_burn_component(src.x, src.w, dst.x, dst.w), _color_burn_component(src.y, src.w, dst.y, dst.w), _color_burn_component(src.z, src.w, dst.z, dst.w), src.w + (1.0 - src.w) * dst.w);
}
vec4 main() {
    vec4 _0_blend_color_burn;
    {
        _0_blend_color_burn = vec4(_color_burn_component(src.x, src.w, dst.x, dst.w), _color_burn_component(src.y, src.w, dst.y, dst.w), _color_burn_component(src.z, src.w, dst.z, dst.w), src.w + (1.0 - src.w) * dst.w);
    }

    return _0_blend_color_burn;

}
