
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _0_blend_src_atop;
    {
        _0_blend_src_atop = dst.w * src + (1.0 - src.w) * dst;
    }

    sk_FragColor = _0_blend_src_atop;

}
