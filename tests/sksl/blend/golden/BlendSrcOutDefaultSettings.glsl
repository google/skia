
uniform vec4 src, dst;
vec4 blend_src_out(vec4 src, vec4 dst) {
    return (1.0 - dst.w) * src;
}
vec4 main() {
    vec4 _0_blend_src_out;
    {
        _0_blend_src_out = (1.0 - dst.w) * src;
    }

    return _0_blend_src_out;

}
