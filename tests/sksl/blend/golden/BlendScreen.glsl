#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_screen;
    {
        _0_blend_screen = src + (1.0 - src) * dst;
    }

    sk_FragColor = _0_blend_screen;

}
