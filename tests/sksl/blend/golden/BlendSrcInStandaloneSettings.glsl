
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _0_blend_src_in;
    {
        _0_blend_src_in = src * dst.w;
    }

    sk_FragColor = _0_blend_src_in;

}
