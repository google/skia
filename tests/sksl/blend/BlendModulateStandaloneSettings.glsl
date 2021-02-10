
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_modulate(vec4 src, vec4 dst) {
    return src * dst;
}
void main() {
    sk_FragColor = blend_modulate(src, dst);
}
