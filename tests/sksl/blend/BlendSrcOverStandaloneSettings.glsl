
out vec4 sk_FragColor;
uniform vec4 src;
uniform vec4 dst;
vec4 blend_src_over_h4h4h4(vec4 src, vec4 dst);
vec4 blend_src_over_h4h4h4(vec4 src, vec4 dst) {
    return src + (1.0 - src.w) * dst;
}
void main() {
    sk_FragColor = blend_src_over_h4h4h4(src, dst);
}
