#version 400
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _0_blend_exclusion;
    {
        _0_blend_exclusion = vec4((dst.xyz + src.xyz) - (2.0 * dst.xyz) * src.xyz, src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _0_blend_exclusion;

}
