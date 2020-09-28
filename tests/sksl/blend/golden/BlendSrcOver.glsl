#version 400
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _0_blend_src_over;
    {
        _0_blend_src_over = src + (1.0 - src.w) * dst;
    }

    sk_FragColor = _0_blend_src_over;

}
