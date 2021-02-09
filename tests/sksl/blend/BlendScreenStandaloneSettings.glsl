
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_screen(vec4 src, vec4 dst) {
    return src + (1.0 - src) * dst;
}
void main() {
    sk_FragColor = blend_screen(src, dst);
}
