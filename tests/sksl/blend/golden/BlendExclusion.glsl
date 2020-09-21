#version 400
uniform vec4 src, dst;
vec4 blend_exclusion(vec4 src, vec4 dst) {
    return vec4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);
}
vec4 main() {
    vec4 _0_blend_exclusion;
    {
        _0_blend_exclusion = vec4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);
    }

    return _0_blend_exclusion;

}
