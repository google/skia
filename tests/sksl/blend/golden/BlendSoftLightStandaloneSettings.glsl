
uniform vec4 src, dst;
float _guarded_divide(float n, float d) {
    return n / d;
}
float _soft_light_component(float sc, float sa, float dc, float da) {
    if (2.0 * sc <= sa) {
        float _1_guarded_divide;
        float _2_n = (dc * dc) * (sa - 2.0 * sc);
        {
            _1_guarded_divide = _2_n / da;
        }
        return (_1_guarded_divide + (1.0 - da) * sc) + dc * ((-sa + 2.0 * sc) + 1.0);

    } else if (4.0 * dc <= da) {
        float DSqd = dc * dc;
        float DCub = DSqd * dc;
        float DaSqd = da * da;
        float DaCub = DaSqd * da;
        float _3_guarded_divide;
        float _4_n = ((DaSqd * (sc - dc * ((3.0 * sa - 6.0 * sc) - 1.0)) + ((12.0 * da) * DSqd) * (sa - 2.0 * sc)) - (16.0 * DCub) * (sa - 2.0 * sc)) - DaCub * sc;
        {
            _3_guarded_divide = _4_n / DaSqd;
        }
        return _3_guarded_divide;

    } else {
        return ((dc * ((sa - 2.0 * sc) + 1.0) + sc) - sqrt(da * dc) * (sa - 2.0 * sc)) - da * sc;
    }
}
vec4 blend_soft_light(vec4 src, vec4 dst) {
    return dst.w == 0.0 ? src : vec4(_soft_light_component(src.x, src.w, dst.x, dst.w), _soft_light_component(src.y, src.w, dst.y, dst.w), _soft_light_component(src.z, src.w, dst.z, dst.w), src.w + (1.0 - src.w) * dst.w);
}
vec4 main() {
    vec4 _0_blend_soft_light;
    {
        _0_blend_soft_light = dst.w == 0.0 ? src : vec4(_soft_light_component(src.x, src.w, dst.x, dst.w), _soft_light_component(src.y, src.w, dst.y, dst.w), _soft_light_component(src.z, src.w, dst.z, dst.w), src.w + (1.0 - src.w) * dst.w);
    }

    return _0_blend_soft_light;

}
