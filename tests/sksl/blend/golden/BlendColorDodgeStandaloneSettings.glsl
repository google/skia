
uniform vec4 src, dst;
float _guarded_divide(float n, float d) {
    {
        return n / d;
    }
}
float _color_dodge_component(float sc, float sa, float dc, float da) {
    if (dc == 0.0) {
        return sc * (1.0 - da);
    } else {
        float d = sa - sc;
        if (d == 0.0) {
            return (sa * da + sc * (1.0 - da)) + dc * (1.0 - sa);
        }
        float _1_guarded_divide;
        float _2_n = dc * sa;
        {
            {
                _1_guarded_divide = _2_n / d;
            }
        }
        d = min(da, _1_guarded_divide);

        return (d * sa + sc * (1.0 - da)) + dc * (1.0 - sa);
    }
}
vec4 blend_color_dodge(vec4 src, vec4 dst) {
    return vec4(_color_dodge_component(src.x, src.w, dst.x, dst.w), _color_dodge_component(src.y, src.w, dst.y, dst.w), _color_dodge_component(src.z, src.w, dst.z, dst.w), src.w + (1.0 - src.w) * dst.w);
}
vec4 main() {
    vec4 _0_blend_color_dodge;
    {
        _0_blend_color_dodge = vec4(_color_dodge_component(src.x, src.w, dst.x, dst.w), _color_dodge_component(src.y, src.w, dst.y, dst.w), _color_dodge_component(src.z, src.w, dst.z, dst.w), src.w + (1.0 - src.w) * dst.w);
    }

    return _0_blend_color_dodge;

}
