
uniform vec4 src, dst;
vec4 blend_src_in(vec4 src, vec4 dst) {
    return src * dst.w;
}
vec4 main() {
    vec4 _0_blend_src_in;
    {
        _0_blend_src_in = src * dst.w;
    }

    return _0_blend_src_in;

}
