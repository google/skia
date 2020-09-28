
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _0_blend_multiply;
    {
        _0_blend_multiply = vec4(((1.0 - src.w) * dst.xyz + (1.0 - dst.w) * src.xyz) + src.xyz * dst.xyz, src.w + (1.0 - src.w) * dst.w);
    }

    sk_FragColor = _0_blend_multiply;

}
