
uniform vec4 src, dst;
vec4 blend_xor(vec4 src, vec4 dst) {
    return (1.0 - dst.w) * src + (1.0 - src.w) * dst;
}
vec4 main() {
    vec4 _0_blend_xor;
    {
        _0_blend_xor = (1.0 - dst.w) * src + (1.0 - src.w) * dst;
    }

    return _0_blend_xor;

}
