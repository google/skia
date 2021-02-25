
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
void main() {
    vec4 _0_blend_screen;
    sk_FragColor = src + (1.0 - src) * dst;

}
