
uniform vec4 src, dst;
vec4 blend_dst_out(vec4 src, vec4 dst) {
    return (1.0 - src.w) * dst;
}
vec4 main() {
    vec4 _0_blend_dst_out;
    {
        _0_blend_dst_out = (1.0 - src.w) * dst;
    }

    return _0_blend_dst_out;

}
