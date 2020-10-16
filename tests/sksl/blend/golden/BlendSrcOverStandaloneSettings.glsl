
out vec4 sk_FragColor;
in vec4 src;
in vec4 dst;
vec4 blend_src_over(vec4 src, vec4 dst) {
    return src + (1.0 - src.w) * dst;
}
void main() {
    sk_FragColor = blend_src_over(src, dst);
}
