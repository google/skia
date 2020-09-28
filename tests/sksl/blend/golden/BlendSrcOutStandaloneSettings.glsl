
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _0_blend_src_out;
    {
        _0_blend_src_out = (1.0 - dst.w) * src;
    }

    sk_FragColor = _0_blend_src_out;

}
