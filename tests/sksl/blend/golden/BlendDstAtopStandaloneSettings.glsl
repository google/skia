
uniform vec4 src, dst;
vec4 blend_src_atop(vec4 src, vec4 dst) {
    return dst.w * src + (1.0 - src.w) * dst;
}
vec4 main() {
    vec4 _0_blend_src_atop;
    {
        _0_blend_src_atop = dst.w * src + (1.0 - src.w) * dst;
    }

    return _0_blend_src_atop;

}
