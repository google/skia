#version 400
out vec4 sk_FragColor;
in vec4 src, dst;
void main() {
    vec4 _0_blend_dst;
    {
        _0_blend_dst = dst;
    }

    sk_FragColor = _0_blend_dst;

}
