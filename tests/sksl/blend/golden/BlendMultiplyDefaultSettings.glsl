
uniform vec4 src, dst;
vec4 blend_multiply(vec4 src, vec4 dst) {
    return vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
}
vec4 main() {
    vec4 _0_blend_multiply;
    {
        _0_blend_multiply = vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
    }

    return _0_blend_multiply;

}
