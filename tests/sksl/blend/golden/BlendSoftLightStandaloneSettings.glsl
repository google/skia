
uniform vec4 src, dst;
float _guarded_divide(float n, float d) {
    {
        return n / d;
    }
}
float _soft_light_component(float sc, float sa, float dc, float da) {
    if (2.0 * sc <= sa) {
        float _0_guarded_divide;
        float _1_n = (dc * dc) * (sa - 2.0 * sc);
        {
            {
                _0_guarded_divide = _1_n / da;
            }
        }
        return (_0_guarded_divide + (1.0 - da) * sc) + dc * ((-sa + 2.0 * sc) + 1.0);

    } else if (4.0 * dc <= da) {
        float DSqd = dc * dc;
        float DCub = DSqd * dc;
        float DaSqd = da * da;
        float DaCub = DaSqd * da;
        float _2_guarded_divide;
        float _3_n = ((DaSqd * (sc - dc * ((3.0 * sa - 6.0 * sc) - 1.0)) + ((12.0 * da) * DSqd) * (sa - 2.0 * sc)) - (16.0 * DCub) * (sa - 2.0 * sc)) - DaCub * sc;
        {
            {
                _2_guarded_divide = _3_n / DaSqd;
            }
        }
        return _2_guarded_divide;

    }
    return ((dc * ((sa - 2.0 * sc) + 1.0) + sc) - sqrt(da * dc) * (sa - 2.0 * sc)) - da * sc;
}
vec4 blend_soft_light(vec4 src, vec4 dst) {
    if (dst.w == 0.0) {
        return src;
    }
    return vec4(_soft_light_component(src.x, src.w, dst.x, dst.w), _soft_light_component(src.y, src.w, dst.y, dst.w), _soft_light_component(src.z, src.w, dst.z, dst.w), src.w + (1.0 - src.w) * dst.w);
}
vec4 main() {
    return blend_soft_light(src, dst);
}
