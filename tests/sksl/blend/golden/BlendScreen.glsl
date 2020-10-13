#version 400
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_screen(vec4 src, vec4 dst) {
    return src + (1.0 - src) * dst;
}
void main() {
    vec4 _0_blend_screen;
    {
        _0_blend_screen = src + (1.0 - src) * dst;
    }

    sk_FragColor = _0_blend_screen;

}
