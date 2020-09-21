
uniform vec4 src, dst;
vec4 blend_dst_over(vec4 src, vec4 dst) {
    return (1.0 - dst.w) * src + dst;
}
vec4 main() {
    vec4 _0_blend_dst_over;
    {
        _0_blend_dst_over = (1.0 - dst.w) * src + dst;
    }

    return _0_blend_dst_over;

}
