
uniform vec4 src, dst;
vec4 blend_src_over(vec4 src, vec4 dst) {
    return src + (1.0 - src.w) * dst;
}
vec4 main() {
    vec4 _0_blend_src_over;
    {
        _0_blend_src_over = src + (1.0 - src.w) * dst;
    }

    return _0_blend_src_over;

}
